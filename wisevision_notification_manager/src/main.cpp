// Copyright (c) 2024, WiseVision. All rights reserved.
#include <wisevision_notification_manager/notification_handler.hpp>

int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<NotificationHandler>());
  rclcpp::shutdown();
  return 0;
}