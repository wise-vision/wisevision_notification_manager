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
#include "wisevision_notification_manager/notification_handler.hpp"

NotificationHandler::NotificationHandler(const rclcpp::NodeOptions &options)
    : Node("notification_handler", options) {
  this->declare_parameter<bool>("use_email_notifier", true);
  this->declare_parameter<bool>("use_firebase_notifier", false);
  this->declare_parameter<std::string>("service_account_path",
                                       "serviceAccount.json");
  this->declare_parameter<std::string>("smtp_server",
                                       "smtp://smtp.gmail.com:587");

  bool use_email_notifier;
  bool use_firebase_notifier;
  std::string service_account_path;
  std::string smtp_server;

  use_email_notifier = this->get_parameter("use_email_notifier").as_bool();
  use_firebase_notifier =
      this->get_parameter("use_firebase_notifier").as_bool();
  service_account_path =
      this->get_parameter("service_account_path").as_string();
  smtp_server = this->get_parameter("smtp_server").as_string();

  if (use_email_notifier) {
    try {
      m_email_notifier = EmailNotifier::createFromEnvAndArgs(smtp_server);
      if (!m_email_notifier) {
        throw std::runtime_error(
            "Failed to initialize EmailNotifier (nullptr returned)");
      }
    } catch (const std::exception &e) {
      RCLCPP_ERROR(this->get_logger(),
                   "EmailNotifier initialization failed: %s", e.what());
      throw;
    }
  }

  if (use_firebase_notifier) {
    const char *env_tokens = std::getenv(FirebaseNotifier::ENV_DEVICE_TOKENS);
    if (!env_tokens || std::string(env_tokens).empty()) {
      throw std::runtime_error(
          "Environment variable DEVICE_TOKENS is not set or empty! Firebase "
          "Notifier cannot be initialized.");
    }
    RCLCPP_INFO(this->get_logger(),
                "Initializing Firebase Notifier from serviceAccount.json");
    m_firebase_notifier =
        std::make_shared<FirebaseNotifier>(service_account_path);
  }

  if (!use_email_notifier && !use_firebase_notifier) {
    throw std::runtime_error("No notifier has been initialized. At least one "
                             "notifier should be enabled.");
  }

  m_subscription = this->create_subscription<Notification>(
      "notifications", 10,
      std::bind(&NotificationHandler::notificationCallback, this,
                std::placeholders::_1));
}

void NotificationHandler::notificationCallback(
    const Notification::SharedPtr msg) {
  RCLCPP_INFO(this->get_logger(), "Received notification:");
  RCLCPP_INFO(this->get_logger(), "Source: %s", msg->source.c_str());
  RCLCPP_INFO(this->get_logger(), "Severity: %d",
              static_cast<int>(msg->severity));
  RCLCPP_INFO(this->get_logger(), "Info: %s", msg->info.c_str());
  RCLCPP_INFO(this->get_logger(), "Publication Method: %d",
              static_cast<int>(msg->publication_method));

  switch (msg->publication_method) {
  case Notification::EMAIL:
    handleEmailNotification(msg);
    break;
  case Notification::PUSH_NOTIFICATION:
    handlePushNotification(msg);
    break;
  case Notification::WEB_PUSH_NOTIFICATION:
    RCLCPP_INFO(this->get_logger(), "Handling web push notification");
    break;
  case Notification::EMAIL_PUSH:
    RCLCPP_INFO(this->get_logger(), "Handling Email and push notification");
    handleEmailNotification(msg);
    handlePushNotification(msg);
    break;
  case Notification::EMAIL_WEB_PUSH:
    RCLCPP_INFO(this->get_logger(),
                "Handling Email, web and push notification");
    handleEmailNotification(msg);
    break;
  case Notification::PUSH_WEB_PUSH:
    RCLCPP_INFO(this->get_logger(), "Handling push and web push notification");
    handlePushNotification(msg);
    break;
  case Notification::EMAIL_PUSH_WEB_PUSH:
    RCLCPP_INFO(this->get_logger(), "Handling all notification methods");
    handleEmailNotification(msg);
    handlePushNotification(msg);
    break;
  default:
    RCLCPP_WARN(this->get_logger(), "Unknown publication method: %d",
                static_cast<int>(msg->publication_method));
    break;
  }
}

void NotificationHandler::handlePushNotification(
    const Notification::SharedPtr msg) {
  std::string severity_str;
  switch (msg->severity) {
  case Notification::NORMAL:
    severity_str = "Normal";
    break;
  case Notification::WARNING:
    severity_str = "Warning";
    break;
  case Notification::ERROR:
    severity_str = "Error";
    break;
  default:
    severity_str = "Unknown";
    break;
  }
  std::string title = msg->source;
  std::string body = "Severity: " + severity_str + "\nInfo: " + msg->info;
  ;
  std::string customKey = "navigationID";
  std::string customValue = "settings";

  if (!m_firebase_notifier) {
    RCLCPP_ERROR(this->get_logger(), "FirebaseNotifier is not initialized");
    return;
  }
  bool success = m_firebase_notifier->sendNotification(title, body, customKey,
                                                       customValue);
  if (!success) {
    RCLCPP_ERROR(this->get_logger(), "Failed to send push notification");
  } else {
    RCLCPP_INFO(this->get_logger(), "Push notification sent successfully");
  }
}

void NotificationHandler::handleEmailNotification(
    const Notification::SharedPtr msg) {
  std::string severity_str;
  switch (msg->severity) {
  case Notification::NORMAL:
    severity_str = "Normal";
    break;
  case Notification::WARNING:
    severity_str = "Warning";
    break;
  case Notification::ERROR:
    severity_str = "Error";
    break;
  default:
    severity_str = "Unknown";
    break;
  }

  std::string email_body = "Severity: " + severity_str + "\nInfo: " + msg->info;

  RCLCPP_INFO(this->get_logger(), "Handling email notification: %s",
              msg->info.c_str());
  if (!m_email_notifier) {
    RCLCPP_ERROR(this->get_logger(), "EmailNotifier is not initialized");
    return;
  }
  bool success = m_email_notifier->sendEmail(msg->source, email_body);
  if (!success) {
    RCLCPP_ERROR(this->get_logger(), "Failed to send email notification");
  }
}

#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(NotificationHandler)
