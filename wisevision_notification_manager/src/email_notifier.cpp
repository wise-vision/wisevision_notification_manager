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
EmailNotifier::createFromConfig(const std::string &config_file) {
  try {
    YAML::Node config = YAML::LoadFile(config_file);

    if (!config["smtp_server"] || !config["username"] || !config["password"] ||
        !config["recipients"]) {
      std::cerr << "Missing required configuration fields" << std::endl;
      return nullptr;
    }

    if (!config["smtp_server"] || config["smtp_server"].IsNull() ||
        config["smtp_server"].as<std::string>().empty()) {
      std::cerr << "smtp_server is empty or null" << std::endl;
      return nullptr;
    }

    if (!config["username"] || config["username"].IsNull() ||
        config["username"].as<std::string>().empty()) {
      std::cerr << "username is empty or null" << std::endl;
      return nullptr;
    }

    if (!config["password"] || config["password"].IsNull() ||
        config["password"].as<std::string>().empty()) {
      std::cerr << "password is empty or null" << std::endl;
      return nullptr;
    }

    auto recipients_node = config["recipients"];
    if (!recipients_node || recipients_node.size() == 0) {
      std::cerr << "Recipients list cannot be empty" << std::endl;
      return nullptr;
    }

    std::vector<std::string> recipients;
    for (const auto &recipient : recipients_node) {
      std::string recipient_str = recipient.as<std::string>();
      if (recipient_str.empty()) {
        std::cerr << "Recipient cannot be an empty string" << std::endl;
        return nullptr;
      }
      recipients.push_back(recipient_str);
    }

    std::string smtp_server = config["smtp_server"].as<std::string>();
    std::string username = config["username"].as<std::string>();
    std::string password = config["password"].as<std::string>();

    return std::make_shared<EmailNotifier>(smtp_server, username, password,
                                           recipients);
  } catch (const YAML::Exception &e) {
    std::cerr << "Failed to load config file: " << e.what() << std::endl;
    return nullptr;
  }
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