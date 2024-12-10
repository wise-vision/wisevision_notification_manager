// Copyright (c) 2024, WiseVision. All rights reserved.
#ifndef FIREBASE_NOTIFIER_HPP
#define FIREBASE_NOTIFIER_HPP

#include <chrono>
#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <json/json.h>
#include <jwt-cpp/jwt.h>
#include <memory>
#include <string>
#include <vector>

#include "wisevision_notification_manager/structs.hpp"

class FirebaseNotifier {
public:
  FirebaseNotifier(const std::string &service_account_file,
                   const std::string &device_token_file);
  bool sendNotification(const std::string &title, const std::string &body,
                        const std::string &custom_key,
                        const std::string &custom_value);

private:
  std::string m_service_account_file;
  std::string m_device_tokens_file;
  Json::Value m_service_account;
  std::vector<std::string> m_device_tokens;
  void loadServiceAccount();
  void checkDeviceTokens();
  std::string getAccessToken();

  static size_t writeCallback(void *contents, size_t size, size_t nmemb,
                              void *userp);
  std::vector<std::string> getDeviceTokensFromJson(const std::string &filename);
  std::string processPrivateKey(const std::string &rawKey);
  CURLcode
  performCurlRequest(const std::string &url, const std::string &post_data_str,
                     std::unique_ptr<curl_slist, CurlSlistDeleter> &headers,
                     std::string &response_string);

  CURLcode performCurlNotificationRequest(
      const std::string &url,
      std::unique_ptr<curl_slist, CurlSlistDeleter> &headers,
      std::string json_data);
};

#endif // FIREBASE_NOTIFIER_HPP