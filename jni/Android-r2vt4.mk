LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := libr2vt
LOCAL_SRC_FILES := $(shell find -L ../r2slv4/r2vt4/src -name "*.c*")

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a) 
LOCAL_ARM_NEON  := true
EXTRA_CFLAGS    := -march=armv7-a -mfloat-abi=softfp -mfpu=neon-fp16 #-mfpu=neon
EXTRA_LDFLAGS   := -Wl,--fix-cortex-a8
endif

ifeq ($(TARGET_ARCH_ABI),arm64-v8a) 
EXTRA_LDFLAGS   := "-Wl"
endif

LOCAL_LDLIBS    := -llog
LOCAL_LDFLAGS   := "-Wl,--start-group" $(LIBS) "-Wl,--end-group" -ldl  -fopenmp #-DUSE_ARM_NEON

LOCAL_C_INCLUDES := ../3rd-party/armeabi-v7a/include 

LOCAL_SHARED_LIBRARIES := r2ssp ztvad blis
LOCAL_STATIC_LIBRARIES := fftw #blis

include $(BUILD_SHARED_LIBRARY)
