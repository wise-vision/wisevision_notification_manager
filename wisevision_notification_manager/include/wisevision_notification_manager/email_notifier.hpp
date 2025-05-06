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
#ifndef EMAIL_NOTIFIER_HPP
#define EMAIL_NOTIFIER_HPP

#include <cstring>
#include <curl/curl.h>
#include <iostream>
#include <string>
#include <yaml-cpp/yaml.h>

#include "wisevision_notification_manager/structs.hpp"

struct ReadCallbackData {
  std::string *data;
  size_t *position;
};

using CurlPerformFunc = CURLcode (*)(CURL *);

class EmailNotifier {
public:
  EmailNotifier(const std::string &smtp_server, const std::string &username,
                const std::string &password,
                const std::vector<std::string> &recipients);

  ~EmailNotifier();

  bool sendEmail(const std::string &subject, const std::string &body);

  static std::shared_ptr<EmailNotifier>
  createFromEnvAndArgs(const std::string &smtp_server);
  CurlPerformFunc curlPerform;

  static constexpr const char *EMAIL_USERNAME = "EMAIL_USERNAME_NOTIFICATION";
  static constexpr const char *EMAIL_PASSWORD = "EMAIL_PASSWORD_NOTIFICATION";
  static constexpr const char *EMAIL_RECIPIENTS =
      "EMAIL_RECIPIENTS_NOTIFICATION";

private:
  std::string m_smtp_server;
  std::string m_username;
  std::string m_password;
  std::vector<std::string> m_recipients;
  std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> m_curl;
  std::string createEmailData(const std::string &subject,
                              const std::string &body);
  bool configureCurl(ReadCallbackData &callback_data,
                     curl_slist *recipients_list);
  std::unique_ptr<curl_slist, struct CurlSlistDeleter> prepareRecipientsList();

  friend class EmailNotifierTest;
};

#endif // EMAIL_NOTIFIER_HPP
