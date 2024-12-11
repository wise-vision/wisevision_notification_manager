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
```
Run:
```bash
source install/setup.bash
ros2 run wisevision_notification_manager notifications_handler --ros-args -p use_email_notifier:=${USE_EMAIL_NOTIFIER} -p use_firebase_notifier:=${USE_FIREBASE_NOTIFIER}
```
## Examples

To get notification from wisevision_notification_manager, it's needed to have running in another terminal `wisevision_action_executor`:
```bash
cd ~/wisevision_action_executor_ws
source install/setup.bash
ros2 run automatic_action_execution automatic_action_service
```
Create action for `topic_1`:
```bash
ros2 service call /create_automatic_action lora_msgs/srv/AutomaticAction "{listen_topic: '/topic_1', listen_message_type: 'std_msgs/msg/Int32', value: 'data', trigger_val: '50.0', trigger_type: 'LessThan', action_and_publisher_name: '/example_topic_1', pub_message_type: 'std_msgs/msg/String', trigger_text: 'test', data_validity_ms: 5000}"
```
Create `topic_1` publisher to published example data:
```bash
ros2 topic pub /topic_1 std_msgs/msg/Int32 "{data: 20}" --rate 0.05
```

Now notifications will be sent to the selected target.


