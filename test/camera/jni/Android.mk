LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := mycmaera
LOCAL_SRC_FILES := native-lib.cpp
LOCAL_LDLIBS := -lcamera2ndk -lmediandk -landroid -llog
include $(BUILD_SHARED_LIBRARY)
