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
include $(BUILD_EXECUTABLE)

#out/soong/host/linux-x86/bin/hidl-gen -L hash -r android.hardware:hardware/interfaces android.hardware.audio@2.0
#out/soong/host/linux-x86/bin/hidl-gen -L hash -r android.hardware:hardware/interfaces android.hardware.automotive.audiocontrol@2.0
#m android.hardware.automotive.audiocontrol-update-api
#m android.hardware.automotive.audiocontrol-freeze-api

#development/vndk/tools/header-checker/utils/create_reference_dumps.py -l android.hardware.audio@2.0 -product xxxx
