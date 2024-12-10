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
};

TEST_F(FirebaseNotifierTest, ValidJsonWithDeviceTokensTest) {
  std::string valid_service_account_json = R"(
    {
      "type": "service_account",
      "project_id": "wisewisionpush",
      "private_key_id": "68e2c55fc76707c5e356b66397f8d2b7e5557b19",
      "private_key": "-----BEGIN PRIVATE KEY-----\nMIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQD3QzjJkL9gl6w0\n...",
      "client_email": "firebase-adminsdk-m9ueq@wisewisionpush.iam.gserviceaccount.com",
      "client_id": "111143268269893351780",
      "auth_uri": "https://accounts.google.com/o/oauth2/auth",
      "token_uri": "https://oauth2.googleapis.com/token",
      "auth_provider_x509_cert_url": "https://www.googleapis.com/oauth2/v1/certs",
      "client_x509_cert_url": "https://www.googleapis.com/robot/v1/metadata/x509/firebase-adminsdk-m9ueq%40wisewisionpush.iam.gserviceaccount.com"
    })";

  std::string valid_device_tokens_json = R"(
    {
      "devices": [
        {"token": "clgx1-RdRsuRzHQ8t176FO:APA91bHKoVjEyvl1QuvQquceAXj3vzdGx--vGN5Uu8-tOUCiZF3Z2XcLCJUIsQmPfbNxIXfoosyJbDcB_DLSmN3G99aNygw8EssHuz55aAZUehuPJ6FJkuhGwkOguOqOTJXXUjvFmHqa"}
      ]
    })";

  std::string service_account_file = "valid_service_account.json";
  std::string device_tokens_file = "valid_device_tokens.json";

  createJsonFile(valid_service_account_json, service_account_file);
  createJsonFile(valid_device_tokens_json, device_tokens_file);

  EXPECT_NO_THROW(
      { FirebaseNotifier notifier(service_account_file, device_tokens_file); });

  removeJsonFile(service_account_file);
  removeJsonFile(device_tokens_file);
}

TEST_F(FirebaseNotifierTest, MissingDeviceTokensTest) {
  std::string valid_service_account_json = R"(
    {
      "type": "service_account",
      "project_id": "wisewisionpush",
      "private_key_id": "68e2c55fc76707c5e356b66397f8d2b7e5557b19",
      "private_key": "-----BEGIN PRIVATE KEY-----\nMIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQD3QzjJkL9gl6w0\n...",
      "client_email": "firebase-adminsdk-m9ueq@wisewisionpush.iam.gserviceaccount.com",
      "client_id": "111143268269893351780",
      "auth_uri": "https://accounts.google.com/o/oauth2/auth",
      "token_uri": "https://oauth2.googleapis.com/token",
      "auth_provider_x509_cert_url": "https://www.googleapis.com/oauth2/v1/certs",
      "client_x509_cert_url": "https://www.googleapis.com/robot/v1/metadata/x509/firebase-adminsdk-m9ueq%40wisewisionpush.iam.gserviceaccount.com"
    })";

  std::string invalid_device_tokens_json = R"(
    {
      "devices": []
    })";

  std::string service_account_file = "valid_service_account.json";
  std::string device_tokens_file = "empty_device_tokens.json";

  createJsonFile(valid_service_account_json, service_account_file);
  createJsonFile(invalid_device_tokens_json, device_tokens_file);

  EXPECT_THROW(
      { FirebaseNotifier notifier(service_account_file, device_tokens_file); },
      std::runtime_error);

  removeJsonFile(service_account_file);
  removeJsonFile(device_tokens_file);
}

TEST_F(FirebaseNotifierTest, MissingDevicesFieldTest) {
  std::string valid_service_account_json = R"(
    {
      "type": "service_account",
      "project_id": "wisewisionpush",
      "private_key_id": "68e2c55fc76707c5e356b66397f8d2b7e5557b19",
      "private_key": "-----BEGIN PRIVATE KEY-----\nMIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQD3QzjJkL9gl6w0\n...",
      "client_email": "firebase-adminsdk-m9ueq@wisewisionpush.iam.gserviceaccount.com",
      "client_id": "111143268269893351780",
      "auth_uri": "https://accounts.google.com/o/oauth2/auth",
      "token_uri": "https://oauth2.googleapis.com/token",
      "auth_provider_x509_cert_url": "https://www.googleapis.com/oauth2/v1/certs",
      "client_x509_cert_url": "https://www.googleapis.com/robot/v1/metadata/x509/firebase-adminsdk-m9ueq%40wisewisionpush.iam.gserviceaccount.com"
    })";

  std::string invalid_device_tokens_json = R"(
    {
      "not_devices": [
        {"token": "clgx1-RdRsuRzHQ8t176FO:APA91bHKoVjEyvl1QuvQquceAXj3vzdGx--vGN5Uu8-tOUCiZF3Z2XcLCJUIsQmPfbNxIXfoosyJbDcB_DLSmN3G99aNygw8EssHuz55aAZUehuPJ6FJkuhGwkOguOqOTJXXUjvFmHqa"}
      ]
    })";

  std::string service_account_file = "valid_service_account.json";
  std::string device_tokens_file = "invalid_device_tokens.json";

  createJsonFile(valid_service_account_json, service_account_file);
  createJsonFile(invalid_device_tokens_json, device_tokens_file);

  EXPECT_THROW(
      { FirebaseNotifier notifier(service_account_file, device_tokens_file); },
      std::runtime_error);

  removeJsonFile(service_account_file);
  removeJsonFile(device_tokens_file);
}