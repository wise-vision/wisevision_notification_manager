/*
 * Copyright (C) 2025 wisevision
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "wisevision_notification_manager/email_notifier.hpp"
#include <cstdlib>
#include <curl/curl.h>
#include <fstream>
#include <gtest/gtest.h>
#include <memory>

class EmailNotifierTest : public ::testing::Test {
protected:
  std::unique_ptr<EmailNotifier> notifier;

  void SetUp() override {
    notifier = std::make_unique<EmailNotifier>(
        "smtp.example.com", "user@example.com", "password",
        std::vector<std::string>{"recipient@example.com"});
  }

  void createEnvVar(const std::string &key, const std::string &value) {
    setenv(key.c_str(), value.c_str(), 1);
  }

  void unsetEnvVar(const std::string &key) { unsetenv(key.c_str()); }

  void TearDown() override {
    notifier.reset();
    unsetEnvVar("EMAIL_USERNAME_NOTIFICATION");
    unsetEnvVar("EMAIL_PASSWORD_NOTIFICATION");
    unsetEnvVar("EMAIL_RECIPIENTS_NOTIFICATION");
  }
};

CURLcode mockCurlPerform(CURL *curl) { return CURLE_OK; }

CURLcode mockCurlPerformFail(CURL *curl) { return CURLE_FAILED_INIT; }

TEST_F(EmailNotifierTest, TestSendEmailSuccess) {
  notifier->curlPerform = mockCurlPerform;
  bool result = notifier->sendEmail("Test Subject", "Test Body");
  EXPECT_TRUE(result);
}

TEST_F(EmailNotifierTest, TestSendEmailFailure) {
  notifier->curlPerform = mockCurlPerformFail;
  bool result = notifier->sendEmail("Test Subject", "Test Body");
  EXPECT_FALSE(result);
}

TEST_F(EmailNotifierTest, CreateFromEnvAndArgsSuccess) {
  createEnvVar("EMAIL_USERNAME_NOTIFICATION", "user@example.com");
  createEnvVar("EMAIL_PASSWORD_NOTIFICATION", "securepassword");
  createEnvVar("EMAIL_RECIPIENTS_NOTIFICATION",
               "recipient1@example.com,recipient2@example.com");

  std::string smtp_server = "smtp.example.com";
  auto notifier_env = EmailNotifier::createFromEnvAndArgs(smtp_server);

  ASSERT_NE(notifier_env, nullptr);
}

TEST_F(EmailNotifierTest, CreateFromEnvAndArgs_MissingUsername) {
  unsetEnvVar("EMAIL_USERNAME_NOTIFICATION");
  createEnvVar("EMAIL_PASSWORD_NOTIFICATION", "securepassword");
  createEnvVar("EMAIL_RECIPIENTS_NOTIFICATION",
               "recipient1@example.com,recipient2@example.com");

  std::string smtp_server = "smtp.example.com";
  EXPECT_THROW(EmailNotifier::createFromEnvAndArgs(smtp_server),
               std::runtime_error);
}

TEST_F(EmailNotifierTest, CreateFromEnvAndArgs_MissingPassword) {
  createEnvVar("EMAIL_USERNAME_NOTIFICATION", "user@example.com");
  unsetEnvVar("EMAIL_PASSWORD_NOTIFICATION");
  createEnvVar("EMAIL_RECIPIENTS_NOTIFICATION",
               "recipient1@example.com,recipient2@example.com");

  std::string smtp_server = "smtp.example.com";
  EXPECT_THROW(EmailNotifier::createFromEnvAndArgs(smtp_server),
               std::runtime_error);
}
TEST_F(EmailNotifierTest, CreateFromEnvAndArgs_MissingRecipients) {
  createEnvVar("EMAIL_USERNAME_NOTIFICATION", "user@example.com");
  createEnvVar("EMAIL_PASSWORD_NOTIFICATION", "securepassword");
  unsetEnvVar("EMAIL_RECIPIENTS_NOTIFICATION");

  std::string smtp_server = "smtp.example.com";
  EXPECT_THROW(EmailNotifier::createFromEnvAndArgs(smtp_server),
               std::runtime_error);
}

TEST_F(EmailNotifierTest, CreateFromEnvAndArgs_MissingSmtpServer) {
  createEnvVar("EMAIL_USERNAME_NOTIFICATION", "user@example.com");
  createEnvVar("EMAIL_PASSWORD_NOTIFICATION", "securepassword");
  createEnvVar("EMAIL_RECIPIENTS_NOTIFICATION",
               "recipient1@example.com,recipient2@example.com");

  std::string smtp_server = "";
  EXPECT_THROW(EmailNotifier::createFromEnvAndArgs(smtp_server),
               std::runtime_error);
}