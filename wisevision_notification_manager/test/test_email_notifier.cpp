#include "wisevision_notification_manager/email_notifier.hpp"
#include <curl/curl.h>
#include <fstream>
#include <gtest/gtest.h>
#include <memory>

class EmailNotifierTest : public ::testing::Test {
protected:
  void SetUp() override {
    notifier = new EmailNotifier("smtp.example.com", "user@example.com",
                                 "password", {"recipient@example.com"});
  }

  void TearDown() override { delete notifier; }

  EmailNotifier *notifier;

  void createConfigFile(const std::string &file_content,
                        const std::string &file_name) {
    std::ofstream file(file_name);
    file << file_content;
    file.close();
  }
  void removeConfigFile(const std::string &file_name) {
    std::remove(file_name.c_str());
  }
};

CURLcode mockCurlPerform(CURL *curl) {
  std::cout << "Mock: curl_easy_perform called!" << std::endl;
  return CURLE_OK;
}

TEST_F(EmailNotifierTest, TestSendEmailSuccess) {
  notifier->curlPerform = mockCurlPerform;

  bool result = notifier->sendEmail("Test Subject", "Test Body Test");

  EXPECT_TRUE(result);
}

CURLcode mockCurlPerformFail(CURL *curl) {
  std::cout << "Mock: curl_easy_perform failed!" << std::endl;
  return CURLE_FAILED_INIT;
}

TEST_F(EmailNotifierTest, TestSendEmailFailure) {
  notifier->curlPerform = mockCurlPerformFail;

  bool result = notifier->sendEmail("Test Subject", "Test Body");

  EXPECT_FALSE(result);
}

TEST_F(EmailNotifierTest, ValidConfigTest) {
  std::string config_content = R"(
smtp_server: smtp.example.com
username: user@example.com
password: securepassword
recipients:
  - recipient1@example.com
  - recipient2@example.com
)";
  std::string config_file = "valid_config.yaml";
  createConfigFile(config_content, config_file);

  auto notifier_from_config = EmailNotifier::createFromConfig(config_file);

  ASSERT_NE(notifier_from_config, nullptr);

  removeConfigFile(config_file);
}

TEST_F(EmailNotifierTest, MissingFieldTest) {
  std::string config_content = R"(
smtp_server: smtp.example.com
username: user@example.com
recipients:
  - recipient1@example.com
)";
  std::string config_file = "missing_password.yaml";
  createConfigFile(config_content, config_file);

  auto notifier_from_config = EmailNotifier::createFromConfig(config_file);

  EXPECT_EQ(notifier_from_config, nullptr);

  removeConfigFile(config_file);
}

TEST_F(EmailNotifierTest, EmptyFieldTest) {
  std::string config_content = R"(
smtp_server: 
username: user@example.com
password: securepassword
recipients:
  - recipient1@example.com
)";
  std::string config_file = "empty_smtp_server.yaml";
  createConfigFile(config_content, config_file);

  auto notifier_from_config = EmailNotifier::createFromConfig(config_file);

  EXPECT_EQ(notifier_from_config, nullptr);

  removeConfigFile(config_file);
}

TEST_F(EmailNotifierTest, EmptyRecipientsListTest) {
  std::string config_content = R"(
smtp_server: smtp.example.com
username: user@example.com
password: securepassword
recipients: []
)";
  std::string config_file = "empty_recipients.yaml";
  createConfigFile(config_content, config_file);

  auto notifier_from_config = EmailNotifier::createFromConfig(config_file);

  EXPECT_EQ(notifier_from_config, nullptr);

  removeConfigFile(config_file);
}

TEST_F(EmailNotifierTest, InvalidYamlTest) {
  std::string config_content = R"(
smtp_server smtp.example.com
username: user@example.com
password: securepassword
recipients:
  - recipient1@example.com
)";
  std::string config_file = "invalid_yaml.yaml";
  createConfigFile(config_content, config_file);

  auto notifier_from_config = EmailNotifier::createFromConfig(config_file);

  EXPECT_EQ(notifier_from_config, nullptr);

  removeConfigFile(config_file);
}