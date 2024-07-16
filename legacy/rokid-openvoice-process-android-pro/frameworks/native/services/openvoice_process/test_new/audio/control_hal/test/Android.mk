LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := audiocontrol_test

LOCAL_SRC_FILES := test.cpp

LOCAL_SHARED_LIBRARIES := \
        libutils \
        libhidlbase \
        android.hardware.automotive.audiocontrol@2.0 \
        android.hardware.audio@4.0

LOCAL_PROPRIETARY_MODULE := true
LOCAL_CPPFLAGS :=-Wno-date-time -Wno-implicit-fallthrough

include $(BUILD_EXECUTABLE)
