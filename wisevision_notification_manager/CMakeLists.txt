cmake_minimum_required(VERSION 3.8)
project(wisevision_notification_manager)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)

include_directories(${JSONCPP_INCLUDE_DIRS})
link_directories(${JSONCPP_LIBRARY_DIRS})

include(FetchContent)
fetchcontent_declare(
    jwt-cpp 
    GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
    GIT_TAG 08bcf77a687fb06e34138e9e9fa12a4ecbe12332 # v0.7.0 release
)
set(JWT_BUILD_EXAMPLES OFF CACHE BOOL "disable building examples" FORCE)
fetchcontent_makeavailable(jwt-cpp)

find_package(OpenSSL REQUIRED)
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(notification_msgs REQUIRED)
find_package(CURL REQUIRED)
find_package(yaml-cpp REQUIRED)

include_directories(include)
include_directories(${CMAKE_BINARY_DIR})

add_executable(notifications_handler
    src/notification_handler.cpp
    src/email_notifier.cpp
    src/firebase_notifier.cpp
    src/main.cpp
)

add_executable(notifications_publisher 
    src/notifications_publisher.cpp
)

ament_target_dependencies(notifications_handler 
    rclcpp 
    notification_msgs
    CURL
    yaml-cpp
    PkgConfig
)

ament_target_dependencies(notifications_publisher 
    rclcpp 
    notification_msgs
)

target_link_libraries(notifications_handler 
    ${YAML_CPP_LIBRARIES} 
    ${JSONCPP_LIBRARIES} 
    ${CONAN_LIBS} 
    jwt-cpp::jwt-cpp 
    OpenSSL::SSL 
    OpenSSL::Crypto
)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  find_package(ament_cmake_gmock REQUIRED)

  ament_add_gtest(tests test/test_email_notifier.cpp test/test_firebase_notifier.cpp)
  target_sources(tests PRIVATE src/email_notifier.cpp src/firebase_notifier.cpp)

  target_include_directories(tests PRIVATE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  )

  target_link_libraries(tests
    ${YAML_CPP_LIBRARIES}
    ${JSONCPP_LIBRARIES}
    jwt-cpp::jwt-cpp
    OpenSSL::SSL
    OpenSSL::Crypto
  )

  ament_target_dependencies(tests 
    rclcpp 
    notification_msgs
    CURL
    yaml-cpp
  )
endif()

install(TARGETS
    notifications_handler
    notifications_publisher
    DESTINATION lib/${PROJECT_NAME}
)

ament_package()