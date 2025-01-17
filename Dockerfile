FROM wisevision/ros_with_wisevision_msgs_and_wisevision_core:humble

WORKDIR /root/wisevision_notification_manager_ws

COPY  . /root/wisevision_notification_manager_ws/src/wisevision_notification_manager

ARG USE_EMAIL_NOTIFIER=true
ARG USE_FIREBASE_NOTIFIER=false

ENV USE_EMAIL_NOTIFIER=${USE_EMAIL_NOTIFIER}
ENV USE_FIREBASE_NOTIFIER=${USE_FIREBASE_NOTIFIER}

RUN if [ "${USE_EMAIL_NOTIFIER}" = "true" ]; then \
      echo "Copying email configuration..." && \
      cp /root/wisevision_notification_manager_ws/src/wisevision_notification_manager/config_email.yaml /root/wisevision_notification_manager_ws/config_email.yaml; \
    else \
      echo "Email notifier is disabled"; \
    fi

RUN if [ "${USE_FIREBASE_NOTIFIER}" = "true" ]; then \
      echo "Copying Firebase configuration..." && \
      cp /root/wisevision_notification_manager_ws/src/wisevision_notification_manager/deviceTokens.json /root/wisevision_notification_manager_ws/deviceTokens.json && \
      cp /root/wisevision_notification_manager_ws/src/wisevision_notification_manager/serviceAccount.json /root/wisevision_notification_manager_ws/serviceAccount.json; \
    else \
      echo "Firebase notifier is disabled"; \
    fi


RUN apt-get update && \
    apt-get install -y curl libyaml-cpp-dev libjsoncpp-dev libcurl4-openssl-dev && \
    rosdep update --include-eol-distros && \
    rosdep install --from-paths src --ignore-src -r -y --rosdistro humble

SHELL ["/bin/bash", "-c"]

RUN /bin/bash -c "source /opt/ros/humble/setup.bash && \
source /root/wisevision_ws/install/setup.bash && \
colcon build --symlink-install"

ENTRYPOINT ["/bin/bash", "-c", "source install/setup.bash && ros2 run wisevision_notification_manager notifications_handler --ros-args -p use_email_notifier:=${USE_EMAIL_NOTIFIER} -p use_firebase_notifier:=${USE_FIREBASE_NOTIFIER}"]
RUN echo 'source install/setup.bash && ros2 run wisevision_notification_manager notifications_handler' >> ~/.bashrc