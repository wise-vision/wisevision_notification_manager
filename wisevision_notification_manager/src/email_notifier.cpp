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
#include "wisevision_notification_manager/email_notifier.hpp"

EmailNotifier::EmailNotifier(const std::string &smtp_server,
                             const std::string &username,
                             const std::string &password,
                             const std::vector<std::string> &recipients)
    : curlPerform(curl_easy_perform), m_smtp_server(smtp_server),
      m_username(username), m_password(password), m_recipients(recipients),
      m_curl(curl_easy_init(), curl_easy_cleanup) {
  if (!m_curl) {
    throw std::runtime_error("Failed to initialize CURL");
  }
}

EmailNotifier::~EmailNotifier() {}

std::shared_ptr<EmailNotifier>
EmailNotifier::createFromEnvAndArgs(const std::string &smtp_server) {
  if (smtp_server.empty()) {
    throw std::runtime_error("SMTP server must be provided via ROS arguments");
  }

  const char *env_username = std::getenv(EMAIL_USERNAME);
  const char *env_password = std::getenv(EMAIL_PASSWORD);
  const char *env_recipients = std::getenv(EMAIL_RECIPIENTS);

  if (!env_username || !env_password || !env_recipients ||
      std::string(env_username).empty() || std::string(env_password).empty() ||
      std::string(env_recipients).empty()) {
    throw std::runtime_error(
        "One or more email environment variables are missing or empty");
  }

  std::vector<std::string> recipients;
  std::stringstream ss(env_recipients);
  std::string recipient;

  while (std::getline(ss, recipient, ',')) {
    if (!recipient.empty()) {
      recipients.push_back(recipient);
    }
  }

  if (recipients.empty()) {
    throw std::runtime_error("EMAIL_RECIPIENTS environment variable is empty");
  }

  return std::make_shared<EmailNotifier>(smtp_server, env_username,
                                         env_password, recipients);
}

size_t readCallback(char *buffer, size_t size, size_t nmemb, void *userdata) {
  ReadCallbackData *callbackData =
      reinterpret_cast<ReadCallbackData *>(userdata);

  std::string *data = callbackData->data;
  size_t *data_pos = callbackData->position;

  size_t buffer_size = size * nmemb;
  size_t data_left = data->size() - *data_pos;
  size_t copy_size = std::min(buffer_size, data_left);

  if (copy_size > 0) {
    memcpy(buffer, data->c_str() + *data_pos, copy_size);
    *data_pos += copy_size;
  }

  return copy_size;
}

bool EmailNotifier::configureCurl(ReadCallbackData &callback_data,
                                  curl_slist *recipients_list) {
  if (!m_curl) {
    std::cerr << "Failed to initialize CURL" << std::endl;
    return false;
  }

  curl_easy_setopt(m_curl.get(), CURLOPT_USERNAME, m_username.c_str());
  curl_easy_setopt(m_curl.get(), CURLOPT_PASSWORD, m_password.c_str());
  curl_easy_setopt(m_curl.get(), CURLOPT_URL, m_smtp_server.c_str());
  curl_easy_setopt(m_curl.get(), CURLOPT_MAIL_FROM, m_username.c_str());
  curl_easy_setopt(m_curl.get(), CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
  curl_easy_setopt(m_curl.get(), CURLOPT_MAIL_RCPT, recipients_list);
  curl_easy_setopt(m_curl.get(), CURLOPT_READDATA, &callback_data);
  curl_easy_setopt(m_curl.get(), CURLOPT_READFUNCTION, readCallback);
  curl_easy_setopt(m_curl.get(), CURLOPT_UPLOAD, 1L);

  return true;
}

std::unique_ptr<curl_slist, CurlSlistDeleter>
EmailNotifier::prepareRecipientsList() {
  std::unique_ptr<curl_slist, CurlSlistDeleter> recipients(nullptr);
  for (const auto &recipient : m_recipients) {
    recipients.reset(
        curl_slist_append(recipients.release(), recipient.c_str()));
  }
  return recipients;
}

std::string EmailNotifier::createEmailData(const std::string &subject,
                                           const std::string &body) {
  std::string from = "From: " + m_username;
  std::string subject_field = "Subject: " + subject;

  std::string data = from + "\r\n" + "To: " + m_recipients[0] + "\r\n" +
                     subject_field + "\r\n\r\n" + body;
  return data;
}

bool EmailNotifier::sendEmail(const std::string &subject,
                              const std::string &body) {
  std::string data = createEmailData(subject, body);
  size_t data_pos = 0;
  ReadCallbackData callback_data = {&data, &data_pos};

  auto recipients = prepareRecipientsList();
  if (!recipients) {
    std::cerr << "Failed to set recipients" << std::endl;
    return false;
  }

  if (!configureCurl(callback_data, recipients.get())) {
    return false;
  }

  std::cout << "Sending email to recipients" << std::endl;
  CURLcode res = curlPerform(m_curl.get());
  std::cout << "Finished sending email to recipients" << std::endl;

  if (res != CURLE_OK) {
    std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res)
              << std::endl;
    return false;
  }

  return true;
}