LOCAL_PATH := $(call my-dir)

define all-named-files-under
$(patsubst ./%,%, \
  $(shell cd $(LOCAL_PATH) ; \
          find -L $(2) -name "$(1)" -and -not -name ".*") \
 )
endef

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
include $(CLEAR_VARS)
LOCAL_MODULE    := fftw
LOCAL_SRC_FILES := ../3rd-party/armeabi-v7a/libs/libfftw3f.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := blis
LOCAL_SRC_FILES := ../3rd-party/armeabi-v7a/libs/libblis.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := r2ssp
LOCAL_SRC_FILES := ../3rd-party/armeabi-v7a/libs/libr2ssp.so
#include $(PREBUILT_SHARED_LIBRARY)

include Android-ztvad.mk Android-r2vt4.mk Android-r2ssp.mk
else
include $(call all-named-files-under,Android-*.mk, .)
endif

include $(CLEAR_VARS)
LOCAL_MODULE    := activation
LOCAL_C_INCLUDES := $(shell find -L ../activation -name include) ../activation/activation ../activation/activation/legacy
LOCAL_SRC_FILES := $(shell find -L ../activation/activation -name "*.cpp")
LOCAL_LDLIBS    := -llog
LOCAL_SHARED_LIBRARIES := r2ssp ztvad blis r2vt fftw
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := test_activation
LOCAL_C_INCLUDES := ../activation/activation ../activation/activation/include
LOCAL_SRC_FILES := ../activation/test_activation/main.cpp
LOCAL_LDLIBS    := -llog
LOCAL_SHARED_LIBRARIES := r2ssp ztvad blis r2vt fftw activation
include $(BUILD_EXECUTABLE)
