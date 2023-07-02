LOCAL_PATH := $(call my-dir)

#dnn ctc
ACOUSTIC_MODEL := dnn
THIRDLIB_PATH := thirdparty/libs/$(ACOUSTIC_MODEL)/$(TARGET_ARCH_ABI)

include $(CLEAR_VARS)
LOCAL_MODULE := librkvacti
LOCAL_SRC_FILES := \
	src/vt_word_manager.cpp \
	src/voice_activation.cpp \
	src/vsys_activation.cpp \
	src/audio_converter.cpp \
	src/phoneme.cpp \
	src/event_hub.cpp \
	src/legacy/r2mem_cod.cpp
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/src \
	$(LOCAL_PATH)/src/legacy \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/thirdparty/include
LOCAL_CPPFLAGS := -std=c++11
LOCAL_LDLIBS := -llog
LOCAL_SHARED_LIBRARIES := libr2ssp libr2vt libztvad
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libr2ssp
LOCAL_SRC_FILES := $(THIRDLIB_PATH)/libr2ssp.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libr2vt
LOCAL_SRC_FILES := $(THIRDLIB_PATH)/libr2vt.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libztvad
LOCAL_SRC_FILES := $(THIRDLIB_PATH)/libztvad.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libsourcelocation
LOCAL_SRC_FILES := $(THIRDLIB_PATH)/libsourcelocation.so
include $(PREBUILT_SHARED_LIBRARY)

ifeq ($(ACOUSTIC_MODEL), ctc)
include $(CLEAR_VARS)
LOCAL_MODULE := librasr
LOCAL_SRC_FILES := $(THIRDLIB_PATH)/librasr.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libtfam 
LOCAL_SRC_FILES := $(THIRDLIB_PATH)/libtfam.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := librfe 
LOCAL_SRC_FILES := $(THIRDLIB_PATH)/librfe.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libtensorflow_inference 
LOCAL_SRC_FILES := $(THIRDLIB_PATH)/libtensorflow_inference.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libtfrtl 
LOCAL_SRC_FILES := $(THIRDLIB_PATH)/libtfrtl.so
include $(PREBUILT_SHARED_LIBRARY)
endif

include $(CLEAR_VARS)
LOCAL_MODULE := librkvacti_jni
LOCAL_CPP_EXTENSION := .cc
LOCAL_SRC_FILES := $(LOCAL_PATH)/android/jni/com_rokid_openvoice_VoiceActivation.cc
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_CPPFLAGS := -std=c++11
LOCAL_LDLIBS := -llog
LOCAL_SHARED_LIBRARIES := librkvacti
include $(BUILD_SHARED_LIBRARY)
