LOCAL_PATH:= $(call my-dir)

#https://github.com/flame/blis.git
#configure auto
CBLAS_RELATIVE_PATH := ../3rd-party/blis/frame/compat/cblas

include $(CLEAR_VARS)
LOCAL_MODULE    := blas
LOCAL_SRC_FILES := $(shell find -L ../3rd-party/BLAS2/SRC -name "*.c")

#the inclusion of BLASWRAP caused unresolved symbols for TooN
LOCAL_CFLAGS    := -O3 -fPIC -DNO_BLAS_WRAP -Wno-logical-op-parentheses
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := blis
LOCAL_SRC_FILES := \
    $(CBLAS_RELATIVE_PATH)/src/cblas_sger.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_dger.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_sdot.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_dgemm.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_ddot.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_saxpy.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_sscal.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_daxpy.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_dscal.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_scopy.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_ssyrk.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_ssymm.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_dcopy.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_dsyrk.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_dsymm.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_sgemv.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_dgemv.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_sspmv.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_stpmv.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_sgbmv.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_stpsv.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_dspmv.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_dtpmv.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_dgbmv.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_dtpsv.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_sspr2.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_dspr2.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_srot.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_drot.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_sspr.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_dspr.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_globals.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_xerbla.c \
    $(CBLAS_RELATIVE_PATH)/../../thread/bli_pthread.h

LOCAL_C_INCLUDES := \
    ../3rd-party/$(TARGET_ARCH_ABI)/include/blis \
    ../3rd-party/blis \
    ../3rd-party/blis/frame/thread \
    ../3rd-party/blis/frame/include

LOCAL_CFLAGS := -O2 -Wall -Wno-unused-function -Wfatal-errors -Wno-tautological-compare -Wno-pass-failed -fPIC -std=c99 -D_GNU_SOURCE -D_POSIX_C_SOURCE=200112L -Iinclude/arm64 -I./frame/include -DBLIS_IS_BUILDING_LIBRARY -DBLIS_ENABLE_CBLAS #-fvisibility=hidden 

LOCAL_STATIC_LIBRARIES := blas
LOCAL_LDLIBS := -lm -ldl

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)
