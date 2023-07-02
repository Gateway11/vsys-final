LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := i2c-tools
LOCAL_SRC_FILES := ../tools/i2cbusses.c ../tools/util.c ../lib/smbus.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include
#LOCAL_CFLAGS := -g -Wall -Werror -Wno-unused-parameter
#include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := test
LOCAL_SRC_FILES := main.cpp
LOCAL_LDLIBS := -llog
LOCAL_C_INCLUDES := 
LOCAL_STATIC_LIBRARIES := 
include $(BUILD_EXECUTABLE)
