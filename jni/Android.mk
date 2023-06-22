LOCAL_PATH := $(call my-dir)

define all-named-files-under
$(patsubst ./%,%, \
  $(shell cd $(LOCAL_PATH) ; \
          find -L $(2) -name "$(1)" -and -not -name ".*") \
 )
endef

include $(CLEAR_VARS)
LOCAL_MODULE    := fftw
LOCAL_SRC_FILES := ../3rd-party/$(TARGET_ARCH_ABI)/libs/libfftw3f.a
include $(PREBUILT_STATIC_LIBRARY)
#include Android-fftw3.mk

include $(CLEAR_VARS)
LOCAL_MODULE    := blis
LOCAL_SRC_FILES := ../3rd-party/$(TARGET_ARCH_ABI)/libs/libblis.a
include $(PREBUILT_STATIC_LIBRARY)
#include Android-blis.mk

include $(CLEAR_VARS)
LOCAL_MODULE    := r2ssp
LOCAL_SRC_FILES := ../3rd-party/$(TARGET_ARCH_ABI)/libs/libr2ssp.so
#include $(PREBUILT_SHARED_LIBRARY)
include Android-r2ssp.mk

include Android-ztvad.mk

#include Android-r2vt4.mk

#include $(call all-subdir-makefiles)
