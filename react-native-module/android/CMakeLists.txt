cmake_minimum_required(VERSION 3.4.1)
project(Sss)

set (CMAKE_VERBOSE_MAKEFILE ON)
set (CMAKE_CXX_STANDARD 20)

file(TO_CMAKE_PATH "${NODE_MODULES_DIR}" NODE_MODULES_DIR)

find_package(ReactAndroid REQUIRED CONFIG)
find_package(fbjni REQUIRED CONFIG)
find_package(OpenCV REQUIRED COMPONENTS OpenCV::opencv_java4)

add_library(react-native-sss
            SHARED
            ../cpp/react-native-sss.cpp
            ../cpp/react-native-sss.h
            cpp-adapter.cpp
            "${NODE_MODULES_DIR}/react-native/ReactCommon/jsi/jsi/jsi.cpp"
            ../cpp/jsi-utils/TypedArray.cpp
            ../cpp/jsi-utils/Promise.cpp
            ../cpp/fast-opencv/ConvertImage.cpp
            ../cpp/main/mainfunctions.cpp
            ../cpp/main/objectdetect.cpp
            ../cpp/main/imageprocessing.cpp
            ../cpp/main/helperfunctions.cpp
            ../cpp/main/cascade.xml
)

# Specifies a path to native header files.
include_directories(
            ../cpp
            ../cpp/jsi-utils
            ../cpp/fast-opencv
            ../cpp/main
            "${NODE_MODULES_DIR}/react-native/React"
            "${NODE_MODULES_DIR}/react-native/React/Base"
            "${NODE_MODULES_DIR}/react-native/ReactCommon"
            "${NODE_MODULES_DIR}/react-native/ReactCommon/jsi"
            "${NODE_MODULES_DIR}/react-native/ReactCommon/callinvoker"
            "${NODE_MODULES_DIR}/react-native/ReactAndroid/src/main/jni/react/turbomodule"
)

if(ReactAndroid_VERSION_MINOR GREATER_EQUAL 76)
  target_link_libraries(
          react-native-sss
          log
          android
          ReactAndroid::reactnative
          fbjni::fbjni
          OpenCV::opencv_java4
  )
else()
  target_link_libraries(
          react-native-sss
          log
          android
          ReactAndroid::folly_runtime
          ReactAndroid::glog
          ReactAndroid::jsi
          ReactAndroid::reactnativejni
          fbjni::fbjni
          OpenCV::opencv_java4
  )
endif()
