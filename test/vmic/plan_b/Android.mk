LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := server
LOCAL_SRC_FILES := VirtualMicServer.cpp
#LOCAL_CPPFLAGS    := -Wno-writable-strings -Wno-reorder-init-list
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := client
LOCAL_SRC_FILES := VirtualMicClient.cpp
include $(BUILD_EXECUTABLE)
