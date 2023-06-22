LOCAL_PATH:= $(call my-dir)

#https://www.netlib.org/lapack
#https://github.com/Reference-LAPACK/lapack
#https://blog.csdn.net/u012815193/article/details/108777014
#cmake ..  \
    -DCMAKE_SYSTEM_NAME=Android \
    -DCMAKE_TOOLCHAIN_FILE=../../toolbox/ndk-r21/build/cmake/android.toolchain.cmake \
    -DCMAKE_ANDROID_ARCH_ABI=arm64-v8a \
    -DCMAKE_SYSTEM_VERSION=21 \
    -DCMAKE_ANDROID_STL_TYPE=c++_static \
    -DCMAKE_ANDROID_NDK_TOOLCHAIN_VERSION=4.9 \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_Fortran_COMPILER=aarch64-linux-android-gfortran

#CBLAS_RELATIVE_PATH := frame/compat/cblas
CBLAS_RELATIVE_PATH := ../CBLAS

include $(CLEAR_VARS)
LOCAL_MODULE    := blas
LOCAL_SRC_FILES := $(shell find -L ../BLAS2/SRC -name "*.c")

#the inclusion of BLASWRAP caused unresolved symbols for TooN
LOCAL_CFLAGS    := -O3 -fPIC -DNO_BLAS_WRAP -Wno-logical-op-parentheses
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := blis #$(shell basename `dirname $(LOCAL_PATH)`)
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

LOCAL_C_INCLUDES := $(CBLAS_RELATIVE_PATH)/include

LOCAL_CFLAGS := -O2 -O3 -fomit-frame-pointer -Wall -Wno-unused-function -Wfatal-errors -Wno-tautological-compare -Wno-pass-failed -fPIC -std=c99 -D_POSIX_C_SOURCE=200112L -D_DARWIN_C_SOURCE -DBLIS_IS_BUILDING_LIBRARY #-fvisibility=hidden

LOCAL_STATIC_LIBRARIES := blas
LOCAL_LDLIBS := -lm -ldl

include $(BUILD_SHARED_LIBRARY)
#include $(BUILD_STATIC_LIBRARY)
