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
include $(PREBUILT_SHARED_LIBRARY)

include Android-ztvad.mk Android-r2vt4.mk
else
#include $(call all-subdir-makefiles)
include $(call all-named-files-under,Android-*, .)
endif
