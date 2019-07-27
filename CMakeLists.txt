cmake_minimum_required(VERSION 2.8.3)
project(xiaoqiang_qrcode_wifi)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  std_msgs
  sensor_msgs
  message_generation
  cv_bridge
)

find_package(OpenCV 3.2.0 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(ZBAR zbar)

include_directories(
  ${catkin_INCLUDE_DIRS}
  ${OpenCV_INCLIDE_DIRS}
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/src
)

catkin_package(
  CATKIN_DEPENDS roscpp rospy std_msgs sensor_msgs message_generation cv_bridge message_runtime
)

add_executable(qrcode_wifi_node
  ${PROJECT_SOURCE_DIR}/src/qrcode.cpp
  ${PROJECT_SOURCE_DIR}/src/qrcode_wifi_node.cpp
)

target_link_libraries(qrcode_wifi_node
  ${catkin_LIBRARIES}
  ${OpenCV_LIBS}
  ${ZBAR_LIBRARIES}
)