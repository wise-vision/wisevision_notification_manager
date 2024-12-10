# wisevision_notification_manager

## Prerequisites:


Install yaml cpp, json library and cURL
```bash
sudo apt install curl
sudo apt-get install libyaml-cpp-dev
sudo apt install libjsoncpp-dev
sudo apt-get install -y libcurl4-openssl-dev

```
* ROS2 installed [ROS2 installation](https://docs.ros.org/en/humble/Installation/Ubuntu-Install-Debians.html)

## Build
```bash
source /opt/ros/$ROS_DISTRO/setup.bash
mkdir -p ~/notifications_ws/src && cd ~/notifications_ws/src
git clone git@github.com:wise-vision/wisevision_notification_manager.git
vcs import --recursive < wisevision_notification_manager.repos
cd ..
rosdep install --from-paths src --ignore-src -r -y
colcon build --symlink-install --cmake-args -DCMAKE_BUILD_TYPE=Release --packages-up-to notifications
```

## Push notifications
* In `notifications_ws`  or for docker in `~/notifications_ws/src/wisevision_notification_manager` create `deviceTokens.json`
```bash
cd ~/notifications_ws
mkdir deviceTokens.json
```
In this file add [device token from app](https://github.com/wise-vision/notificator_app/blob/c_k/dev_android_app/README.md#L20) in this way:
```json
{
 "devices": [
    {"token": "your-device-token-from-app"}
  ]
}
```
* Download file from firebase console with service account password `serviceAccount.json` and copy it to `notifications_ws` or for docker in `~/notifications_ws/src/wisevision_notification_manager`
    * Go to firebase console
    * Go to project setting by click on gear icon
    * In settings go to Service accounts
    * In service accounts choose `Java` in admin SDK configuration and click on `Generate new private key`
    * Copy this file to `notifications_ws`


## Email

Before start create in for local `~/notifications_ws` or for docker in `~/notifications_ws/src/wisevision_notification_manager` file `config_email.yaml` with:
```yaml
smtp_server: "smtp://stmp_server"
username: "sender_email"
password: "app_passowrd"
recipients:
  - "recipient_email_1"
  - "recipient_email_2"
```
### Hints
- To use mail as smtp server go to security settings in mail and create app password
## Minimal exampel
In one terminal run:
```bash
cd ~/notifications_ws
ros2 run notifications notifications_handler --ros-args -p use_email_notifier:=true-p use_firebase_notifier:=true
```
In second terminal for testing purpose run publisher with wisevision_msgs/msg/Notifiaction.msgs which indicate for sending notfication via `email` or `push_notification`, in `publicaton_method` pass name one of the methods:
```bash
cd ~/notifications_ws
ros2 run wisevision_notification_manager notifications_publisher <publicaton_method>
```

# Docker
Beofore create conatiner in `docker-compose.yml`, change arguments for your needs of sending notifications via email and via firebase.
```docker-compose
args:
  - USE_EMAIL_NOTIFIER: "<true or false>"
  - USE_FIREBASE_NOTIFIER: "<true or false>"
```
To start `wisevision_notification_manager` in docker conatiner:
```bash
mkdir -p ~/notifications_ws/src && cd ~/notifications_ws/src
git clone git@github.com:wise-vision/wisevision_notification_manager .git
cd wisevision_notification_manager 
docker-compose up
```
