LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := ipc_test
LOCAL_SRC_FILES := ../main.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := spidev_client
LOCAL_SRC_FILES := ../spi/spidev_client.c ../spi/main.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := virtual_mic
LOCAL_SRC_FILES := ../../VirtualMicServer.cpp
#LOCAL_CPPFLAGS    := -Wno-writable-strings -Wno-reorder-init-list
include $(BUILD_EXECUTABLE)
