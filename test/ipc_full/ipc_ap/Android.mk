LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_CFLAGS := -Wno-date-time
LOCAL_MODULE := ipc_server
LOCAL_SRC_FILES := server.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_CFLAGS := -Wno-date-time
LOCAL_MODULE := ipc_client
LOCAL_SRC_FILES:= client.cpp
include $(BUILD_EXECUTABLE)
