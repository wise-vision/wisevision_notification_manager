# Minimal example

This minimal example demonstrates how to use the wisevision_notification_manager.

## Prerequisites
- [ROS2 Humble](https://docs.ros.org/en/humble/Installation.html) or later
- [Build and run](BUILD_AND_RUN.md)
- [Build and run wisevision_action_executor](https://github.com/wise-vision/wisevision_action_executor/blob/main/docs/BUILD_AND_RUN.md)

## Local run
Assuming that after building the package, you can source the workspace, and run the node.

Before run set environment variables:
```bash
export USE_EMAIL_NOTIFIER=<true or false>
export USE_FIREBASE_NOTIFIER=<true or false>
export EMAIL_USERNAME_NOTIFICATION="user@example.com"
export EMAIL_PASSWORD_NOTIFICATION="supersecurepassword"
export EMAIL_RECIPIENTS_NOTIFICATION="recipient1@example.com,recipient2@example.com"
export DEVICE_TOKENS_FIREBASE="<your-device-token-from-app-1>, <your-device-token-from-app-2>"
```
Run:
```bash
source install/setup.bash
ros2 run wisevision_notification_manager notifications_handler --ros-args -p use_email_notifier:=${USE_EMAIL_NOTIFIER} -p use_firebase_notifier:=${USE_FIREBASE_NOTIFIER}
```
Run with config file:
```bash
ros2 run wisevision_notification_manager notifications_handler --ros-args --params-file ~/wisevision_notification_manager_ws/src/wisevision_notification_manager/config/params.yaml
```
## Examples

To get notification from wisevision_notification_manager, it's needed to have running in another terminal `wisevision_action_executor`:
```bash
cd ~/wisevision_action_executor_ws
source install/setup.bash
ros2 run wisevision_action_executor automatic_action_service
```
Create action for `topic_1` that later will be triggered:
```bash
ros2 service call /create_automatic_action lora_msgs/srv/AutomaticAction "{listen_topic: '/topic_1', listen_message_type: 'std_msgs/msg/Int32', value: 'data', trigger_val: '50.0', trigger_type: 'LessThan', action_and_publisher_name: '/example_topic_1', pub_message_type: 'std_msgs/msg/String', trigger_text: 'test', data_validity_ms: 5000}"
```
Create `topic_1` publisher to published example data:
```bash
ros2 topic pub /topic_1 std_msgs/msg/Int32 "{data: 20}" --rate 0.05
```
Seted value `20`, will trigger created action, and that will publish data on topic `notifications`, which is subscribed, by `wisevision_notification manager`.

Now notifications will be sent to the selected target.


