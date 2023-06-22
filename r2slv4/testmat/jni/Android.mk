LOCAL_PATH := $(call my-dir)
#MY_SLV_DIR := /Users/hadoop/Documents/XCode/r2slv3
MY_SLV_DIR := $(shell pwd)/..
MY_THIRDLIB_DIR := $(MY_SLV_DIR)/../thirdlib/$(TARGET_ARCH_ABI)
MY_PRJ_DIR := $(MY_SLV_DIR)/testmat

include $(CLEAR_VARS)
LOCAL_MODULE    := r2vt
LOCAL_SRC_FILES := $(MY_SLV_DIR)/r2vt4/libs/armeabi-v7a/libr2vt.so
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(MY_THIRDLIB_DIR)/include 
LOCAL_MODULE := testmat

MY_FILES_PATH  :=  $(MY_PRJ_DIR)/src
MY_FILES_SUFFIX := %.cpp %.c %.cc
My_All_Files := $(foreach src_path,$(MY_FILES_PATH), $(shell find "$(src_path)" -type f) ) 
My_All_Files := $(My_All_Files:$(MY_CPP_PATH)/./%=$(MY_CPP_PATH)%)
MY_SRC_LIST  := $(filter $(MY_FILES_SUFFIX),$(My_All_Files)) 
MY_SRC_LIST  := $(MY_SRC_LIST:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES := $(MY_SRC_LIST)

LOCAL_SHARED_LIBRARIES := r2vt

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog

LOCAL_ARM_NEON := true

EXTRA_CFLAGS="-march=armv7-a -mfloat-abi=softfp -mfpu=neon"                                           
EXTRA_LDFLAGS="-Wl,--fix-cortex-a8 "
LOCAL_LDFLAGS+= "-Wl,--start-group" $(LIBS) "-Wl,--end-group" -ldl  -fopenmp 

include $(BUILD_EXECUTABLE)

