FROM wisevision/ros_with_wisevision_msgs_and_wisevision_core:humble

WORKDIR /root/wisevision_notification_manager_ws

COPY . /root/wisevision_notification_manager_ws/src/wisevision_notification_manager

ARG CUSTOM_CONFIG=false
ARG USER_CONFIG_PATH=""

ENV CUSTOM_CONFIG=${CUSTOM_CONFIG}
ENV USER_CONFIG_PATH=${USER_CONFIG_PATH}

RUN if [ "$CUSTOM_CONFIG" = "true" ] && [ -n "$USER_CONFIG_PATH" ]; then \
      echo "Using user-provided config file: $USER_CONFIG_PATH"; \
      cp "$USER_CONFIG_PATH" /root/wisevision_notification_manager_ws/params.yaml; \
    else \
      echo "Using default params.yaml"; \
      cp /root/wisevision_notification_manager_ws/src/wisevision_notification_manager/config/params.yaml /root/wisevision_notification_manager_ws/params.yaml; \
    fi

RUN if [ ! -f "/root/wisevision_notification_manager_ws/serviceAccount.json" ]; then \
      echo "Using default serviceAccount.json"; \
      cp /root/wisevision_notification_manager_ws/src/wisevision_notification_manager/serviceAccount.json /root/wisevision_notification_manager_ws/serviceAccount.json; \
    else \
      echo "Using user-provided serviceAccount.json"; \
    fi

RUN apt-get update && \
    apt-get install -y curl libyaml-cpp-dev libjsoncpp-dev libcurl4-openssl-dev && \
    rosdep update --include-eol-distros && \
    rosdep install --from-paths src --ignore-src -r -y --rosdistro humble

SHELL ["/bin/bash", "-c"]

RUN /bin/bash -c "source /opt/ros/humble/setup.bash && \
source /root/wisevision_ws/install/setup.bash && \
colcon build --symlink-install"

ENTRYPOINT ["/bin/bash", "-c", "source install/setup.bash && ros2 run wisevision_notification_manager notifications_handler --ros-args --params-file /root/wisevision_notification_manager_ws/params.yaml"]