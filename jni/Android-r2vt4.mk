LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := libr2vt
LOCAL_SRC_FILES := $(shell find -L ../r2slv4/r2vt4/src -name "*.c*")
#LOCAL_SRC_FILES += ../r2slv4/r2ad3/src

ifneq (,$(filter armeabi-v7a, $(TARGET_ARCH_ABI)))
LOCAL_ARM_NEON  := true
EXTRA_CFLAGS    := -march=armv7-a -mfloat-abi=softfp -mfpu=neon-fp16 #-mfpu=neon
EXTRA_LDFLAGS   := -Wl,--fix-cortex-a8
endif

ifeq (,$(filter-out arm64-v8a, $(TARGET_ARCH_ABI)))
EXTRA_LDFLAGS   := "-Wl"
endif

LOCAL_LDFLAGS   := "-Wl,--start-group" $(LIBS) "-Wl,--end-group" -ldl  -fopenmp
LOCAL_C_INCLUDES := ../3rd-party/armeabi-v7a/include #../r2slv4/r2ad3/src

LOCAL_SHARED_LIBRARIES := r2ssp ztvad blis
LOCAL_STATIC_LIBRARIES := fftw #blis

include $(BUILD_SHARED_LIBRARY)
