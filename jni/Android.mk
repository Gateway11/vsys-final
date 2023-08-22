LOCAL_PATH := $(call my-dir)

define all-named-files-under
$(patsubst ./%,%, \
  $(shell cd $(LOCAL_PATH) ; \
          find -L $(2) -name "$(1)" -and -not -name ".*") \
 )
endef

include $(call all-named-files-under,Android-*.mk, .)

include $(CLEAR_VARS)
LOCAL_MODULE    := activation
LOCAL_C_INCLUDES := $(shell find -L ../activation -name include) ../activation/activation ../activation/activation/legacy
LOCAL_SRC_FILES := $(shell find -L ../activation/activation -name "*.cpp")
LOCAL_SHARED_LIBRARIES := r2ssp ztvad blis r2vt fftw
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := test_activation
LOCAL_C_INCLUDES := ../activation/activation ../activation/activation/include
LOCAL_SRC_FILES := ../activation/test_activation/main.cpp
LOCAL_SHARED_LIBRARIES := r2ssp ztvad blis r2vt fftw activation
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test_aec
LOCAL_C_INCLUDES := ../activation/activation ../activation/speex ../activation/speex/include
LOCAL_SRC_FILES := ../activation/test_audio_processing/main.cpp
LOCAL_SHARED_LIBRARIES := r2ssp ztvad blis
#include $(BUILD_EXECUTABLE)
