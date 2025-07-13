LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)
LOCAL_MODULE := myservice
LOCAL_SRC_FILES := EncryptServer.cpp native-lib.cpp
LOCAL_LDLIBS := -lbinder_ndk -llog
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := myclient
LOCAL_SRC_FILES := EncryptClenit.cpp
LOCAL_LDLIBS := -lbinder_ndk -llog
include $(BUILD_SHARED_LIBRARY)
