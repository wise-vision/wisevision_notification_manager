/*
 * Copyright (C) 2025 wisevision
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "wisevision_notification_manager/firebase_notifier.hpp"
#include <fstream>
#include <gtest/gtest.h>

class FirebaseNotifierTest : public ::testing::Test {
protected:
  void createJsonFile(const std::string &file_content,
                      const std::string &file_name) {
    std::ofstream file(file_name);
    file << file_content;
    file.close();
  }

  void removeJsonFile(const std::string &file_name) {
    std::remove(file_name.c_str());
  }

  void setEnv(const std::string &key, const std::string &value) {
    setenv(key.c_str(), value.c_str(), 1);
  }

  void unsetEnv(const std::string &key) { unsetenv(key.c_str()); }
};

TEST_F(FirebaseNotifierTest, ValidJsonWithDeviceTokensTest) {
  std::string valid_service_account_json = R"(
    {
      "type": "service_account",
      "project_id": "example-project",
      "private_key_id": "1234567890abcdef1234567890abcdef",
      "private_key": "-----BEGIN PRIVATE KEY-----\nMIIEvQIBADANBgkqhkiG9w0BAQEFAASC\n..."
                      "-----END PRIVATE KEY-----",
      "client_email": "example-adminsdk@example-project.iam.gserviceaccount.com",
      "client_id": "123456789012345678901",
      "auth_uri": "https://accounts.google.com/o/oauth2/auth",
      "token_uri": "https://oauth2.googleapis.com/token",
      "auth_provider_x509_cert_url": "https://www.googleapis.com/oauth2/v1/certs",
      "client_x509_cert_url": "https://www.googleapis.com/robot/v1/metadata/x509/example-adminsdk%40example-project.iam.gserviceaccount.com"
    })";

  std::string service_account_file = "valid_service_account.json";

  createJsonFile(valid_service_account_json, service_account_file);

  // Ustawienie zmiennej środowiskowej z tokenami
  setEnv("DEVICE_TOKENS_FIREBASE", "token1,token2,token3");

  EXPECT_NO_THROW({ FirebaseNotifier notifier(service_account_file); });

  removeJsonFile(service_account_file);
  unsetEnv("DEVICE_TOKENS_FIREBASE");
}

TEST_F(FirebaseNotifierTest, MissingDeviceTokensTest) {
  std::string valid_service_account_json = R"(
    {
      "type": "service_account",
      "project_id": "example-project",
      "private_key_id": "1234567890abcdef1234567890abcdef",
      "private_key": "-----BEGIN PRIVATE KEY-----\nMIIEvQIBADANBgkqhkiG9w0BAQEFAASC\n..."
                      "-----END PRIVATE KEY-----",
      "client_email": "example-adminsdk@example-project.iam.gserviceaccount.com",
      "client_id": "123456789012345678901",
      "auth_uri": "https://accounts.google.com/o/oauth2/auth",
      "token_uri": "https://oauth2.googleapis.com/token",
      "auth_provider_x509_cert_url": "https://www.googleapis.com/oauth2/v1/certs",
      "client_x509_cert_url": "https://www.googleapis.com/robot/v1/metadata/x509/example-adminsdk%40example-project.iam.gserviceaccount.com"
    })";

  std::string service_account_file = "valid_service_account.json";

  createJsonFile(valid_service_account_json, service_account_file);

  unsetEnv("DEVICE_TOKENS_FIREBASE");

  EXPECT_THROW({ FirebaseNotifier notifier(service_account_file); },
               std::runtime_error);

  removeJsonFile(service_account_file);
}

TEST_F(FirebaseNotifierTest, EmptyDeviceTokensTest) {
  std::string valid_service_account_json = R"(
    {
      "type": "service_account",
      "project_id": "example-project",
      "private_key_id": "1234567890abcdef1234567890abcdef",
      "private_key": "-----BEGIN PRIVATE KEY-----\nMIIEvQIBADANBgkqhkiG9w0BAQEFAASC\n..."
                      "-----END PRIVATE KEY-----",
      "client_email": "example-adminsdk@example-project.iam.gserviceaccount.com",
      "client_id": "123456789012345678901",
      "auth_uri": "https://accounts.google.com/o/oauth2/auth",
      "token_uri": "https://oauth2.googleapis.com/token",
      "auth_provider_x509_cert_url": "https://www.googleapis.com/oauth2/v1/certs",
      "client_x509_cert_url": "https://www.googleapis.com/robot/v1/metadata/x509/example-adminsdk%40example-project.iam.gserviceaccount.com"
    })";

  std::string service_account_file = "valid_service_account.json";

  createJsonFile(valid_service_account_json, service_account_file);

  setEnv("DEVICE_TOKENS_FIREBASE", "");

  EXPECT_THROW({ FirebaseNotifier notifier(service_account_file); },
               std::runtime_error);

  removeJsonFile(service_account_file);
  unsetEnv("DEVICE_TOKENS_FIREBASE");
}