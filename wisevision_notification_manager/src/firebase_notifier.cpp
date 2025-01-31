/*
 * Copyright (C) 2025 wisevision
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

// Copyright (c) 2024, WiseVision. All rights reserved.
#include "wisevision_notification_manager/firebase_notifier.hpp"

FirebaseNotifier::FirebaseNotifier(const std::string &service_account_file,
                                   const std::string &device_token_file)
    : m_service_account_file(service_account_file),
      m_device_tokens_file(device_token_file) {
  loadServiceAccount();
  checkDeviceTokens();
}

void FirebaseNotifier::loadServiceAccount() {
  try {
    std::ifstream service_account_stream(m_service_account_file,
                                         std::ifstream::binary);
    if (!service_account_stream.is_open()) {
      throw std::runtime_error("Failed to open service account file: " +
                               m_service_account_file);
    }

    Json::Value service_account;
    service_account_stream >> service_account;
    std::vector<std::string> required_fields = {"type",
                                                "project_id",
                                                "private_key_id",
                                                "private_key",
                                                "client_email",
                                                "client_id",
                                                "auth_uri",
                                                "token_uri",
                                                "auth_provider_x509_cert_url",
                                                "client_x509_cert_url"};

    for (const auto &field : required_fields) {
      if (!service_account.isMember(field)) {
        throw std::runtime_error(
            "Missing required field in service account file: " + field);
      }
    }
    m_service_account = service_account;
  } catch (const Json::Exception &e) {
    throw std::runtime_error("Failed to parse JSON: " + std::string(e.what()));
  }
}

void FirebaseNotifier::checkDeviceTokens() {
    try {
        std::ifstream device_tokens_stream(m_device_tokens_file, std::ifstream::binary);
        if (!device_tokens_stream.is_open()) {
            throw std::runtime_error("Failed to open device tokens file: " + m_device_tokens_file);
        }

        Json::Value device_tokens_json;
        device_tokens_stream >> device_tokens_json;

        if (!device_tokens_json.isMember("devices") ||
            !device_tokens_json["devices"].isArray() ||
            device_tokens_json["devices"].empty()) {
            throw std::runtime_error("Device tokens file is invalid or empty: " + m_device_tokens_file);
        }

        m_device_tokens.clear();

        for (const auto& device : device_tokens_json["devices"]) {
            if (!device.isMember("token") || !device["token"].isString()) {
                throw std::runtime_error(
                    "A device entry is missing a valid token in the device tokens file: " + m_device_tokens_file);
            }
            m_device_tokens.push_back(device["token"].asString());
        }
    } catch (const Json::Exception& e) {
        throw std::runtime_error("Failed to parse JSON in device tokens file: " + std::string(e.what()));
    }
}

std::string FirebaseNotifier::processPrivateKey(const std::string &raw_key) {
  std::string private_key = raw_key;

  size_t pos = private_key.find("-----BEGIN PRIVATE KEY-----");
  if (pos != std::string::npos) {
    private_key = private_key.substr(pos + 27);
  }

  pos = private_key.find("-----END PRIVATE KEY-----");
  if (pos != std::string::npos) {
    private_key = private_key.substr(0, pos);
  }

  private_key.erase(std::remove(private_key.begin(), private_key.end(), '\n'),
                    private_key.end());
  private_key.insert(0, "-----BEGIN PRIVATE KEY-----\n");
  private_key.append("\n-----END PRIVATE KEY-----\n");

  return private_key;
}

CURLcode FirebaseNotifier::performCurlRequest(
    const std::string &url, const std::string &post_data_str,
    std::unique_ptr<curl_slist, CurlSlistDeleter> &headers,
    std::string &response_string) {
  std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(curl_easy_init(),
                                                           curl_easy_cleanup);
  CURLcode res;
  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers.get());
  curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, post_data_str.c_str());
  curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, writeCallback);
  curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response_string);
  res = curl_easy_perform(curl.get());
  return res;
}

std::string FirebaseNotifier::getAccessToken() {
  std::string raw_private_key = m_service_account["private_key"].asString();
  std::string private_key = processPrivateKey(raw_private_key);
  std::cout << "Processed private key (truncated): "
            << private_key.substr(0, 30) << "..." << std::endl;
  std::string client_email = m_service_account["client_email"].asString();
  auto token =
      jwt::create<jwt::traits::kazuho_picojson>()
          .set_issuer(client_email)
          .set_audience("https://oauth2.googleapis.com/token")
          .set_issued_at(std::chrono::system_clock::now())
          .set_expires_at(std::chrono::system_clock::now() +
                          std::chrono::minutes(60))
          .set_subject(client_email)
          .set_payload_claim(
              "scope",
              jwt::claim(std::string(
                  "https://www.googleapis.com/auth/firebase.messaging")))
          .sign(jwt::algorithm::rs256("", private_key, "", ""));
  std::string access_token;
  std::string response_string;
  Json::Value post_data;
  post_data["grant_type"] = "urn:ietf:params:oauth:grant-type:jwt-bearer";
  post_data["assertion"] = token;
  Json::StreamWriterBuilder writer;
  std::string post_data_str = Json::writeString(writer, post_data);
  std::unique_ptr<curl_slist, CurlSlistDeleter> headers(nullptr);
  headers.reset(
      curl_slist_append(headers.release(), "Content-Type: application/json"));
  CURLcode res = performCurlRequest("https://oauth2.googleapis.com/token",
                                    post_data_str, headers, response_string);
  if (res != CURLE_OK) {
    std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res)
              << std::endl;
  } else {
    Json::CharReaderBuilder reader_builder;
    std::string errs;
    Json::Value json_response;
    std::istringstream responseStream(response_string);
    if (Json::parseFromStream(reader_builder, responseStream, &json_response,
                              &errs)) {
      access_token = json_response["access_token"].asString();
    }
  }

  return access_token;
}

size_t FirebaseNotifier::writeCallback(void *contents, size_t size,
                                       size_t nmemb, void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

CURLcode FirebaseNotifier::performCurlNotificationRequest(
    const std::string &url,
    std::unique_ptr<curl_slist, CurlSlistDeleter> &headers,
    std::string json_data) {
  std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(curl_easy_init(),
                                                           curl_easy_cleanup);
  CURLcode res;
  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers.get());
  curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, json_data.c_str());
  res = curl_easy_perform(curl.get());
  return res;
}

bool FirebaseNotifier::sendNotification(const std::string& title,
                                        const std::string& body,
                                        const std::string& custom_key,
                                        const std::string& custom_value) {
    std::string access_token = getAccessToken();
    if (access_token.empty()) {
        std::cerr << "Failed to get access token" << std::endl;
        return false;
    }

    for (const auto& device_token : m_device_tokens) {
        Json::Value root;
        root["message"]["token"] = device_token;
        root["message"]["notification"]["title"] = title;
        root["message"]["notification"]["body"] = body;
        root["message"]["data"][custom_key] = custom_value;

        Json::StreamWriterBuilder writer;
        std::string jsonData = Json::writeString(writer, root);


        std::unique_ptr<curl_slist, CurlSlistDeleter> headers(nullptr);
        headers.reset(curl_slist_append(headers.release(), "Content-Type: application/json"));
        headers.reset(curl_slist_append(headers.release(), ("Authorization: Bearer " + access_token).c_str()));

        std::string url = "https://fcm.googleapis.com/v1/projects/wisewisionpush/messages:send";

        std::string response_string;
        CURLcode res = performCurlRequest(url, jsonData, headers, response_string);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            return false;
        } else {
            std::cout << "Response from server: " << response_string << std::endl;
        }
    }
    return true;
}
