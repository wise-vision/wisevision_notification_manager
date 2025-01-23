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
#ifndef NOTIFICATION_HANDLER_HPP
#define NOTIFICATION_HANDLER_HPP

#include "notification_msgs/msg/notification.hpp"
#include "rclcpp/rclcpp.hpp"

#include <wisevision_notification_manager/email_notifier.hpp>
#include <wisevision_notification_manager/firebase_notifier.hpp>

using Notification = notification_msgs::msg::Notification;

class NotificationHandler : public rclcpp::Node {
public:
  NotificationHandler(
      const rclcpp::NodeOptions &options = rclcpp::NodeOptions());

private:
  void notificationCallback(const Notification::SharedPtr msg);
  void handleEmailNotification(const Notification::SharedPtr msg);
  void handlePushNotification(const Notification::SharedPtr msg);

  rclcpp::Subscription<Notification>::SharedPtr m_subscription;
  std::shared_ptr<EmailNotifier> m_email_notifier;
  std::shared_ptr<FirebaseNotifier> m_firebase_notifier;
};

#endif // NOTIFICATION_HANDLER_HPP