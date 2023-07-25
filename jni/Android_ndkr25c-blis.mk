LOCAL_PATH:= $(call my-dir)

LOCAL_CFLAGS_COMMON := -O2 -Wall -Wno-unused-function -Wfatal-errors -Wno-tautological-compare -Wno-pass-failed -fPIC -std=c99 -D_POSIX_C_SOURCE=200112L -I../3rd-party/blis/include/arm64 -I../3rd-party/blis/frame/include -DBLIS_IS_BUILDING_LIBRARY -fvisibility=hidden
LOCAL_CFLAGS_REF_KERNELS := $(LOCAL_CFLAGS_COMMON) -O3 -funsafe-math-optimizations -ffp-contract=fast -DBLIS_IN_REF_KERNEL=1
LOCAL_SRC_FILES_REF_KERNELS := $(shell find -L ../3rd-party/blis/ref_kernels ! -path "*old*" ! -path *other* -name "*.c")

include $(CLEAR_VARS)
LOCAL_MODULE    := ref_kernels_armsve
LOCAL_SRC_FILES := $(LOCAL_SRC_FILES_REF_KERNELS) ../3rd-party/blis/config/armsve/bli_cntx_init_armsve.c
LOCAL_SRC_FILES += $(shell find -L ../3rd-party/blis/kernels/armsve ! -path "*old*" -name "*.c")
LOCAL_CFLAGS    := $(LOCAL_CFLAGS_REF_KERNELS) -march=armv8-a+sve -ftree-vectorize -D_GNU_SOURCE -DBLIS_CNAME=armsve
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := ref_kernels_firestorm
LOCAL_SRC_FILES := $(LOCAL_SRC_FILES_REF_KERNELS) ../3rd-party/blis/config/firestorm/bli_cntx_init_firestorm.c
LOCAL_SRC_FILES += $(shell find -L ../3rd-party/blis/kernels/armv8a ! -path "*old*" -name "*.c")
LOCAL_CFLAGS    := $(LOCAL_CFLAGS_REF_KERNELS) -march=armv8-a -ftree-vectorize -D_GNU_SOURCE -DBLIS_CNAME=firestorm
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := ref_kernels_thunderx2
LOCAL_SRC_FILES := $(LOCAL_SRC_FILES_REF_KERNELS) ../3rd-party/blis/config/thunderx2/bli_cntx_init_thunderx2.c
LOCAL_CFLAGS    := $(LOCAL_CFLAGS_REF_KERNELS) -mcpu=thunderx2t99 -ftree-vectorize -D_GNU_SOURCE -DBLIS_CNAME=thunderx2
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := ref_kernels_cortexa57
LOCAL_SRC_FILES := $(LOCAL_SRC_FILES_REF_KERNELS) ../3rd-party/blis/config/cortexa57/bli_cntx_init_cortexa57.c
LOCAL_CFLAGS    := $(LOCAL_CFLAGS_REF_KERNELS) -mcpu=cortex-a57 -ftree-vectorize -D_GNU_SOURCE -DBLIS_CNAME=cortexa57
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := ref_kernels_cortexa53
LOCAL_SRC_FILES := $(LOCAL_SRC_FILES_REF_KERNELS) ../3rd-party/blis/config/cortexa53/bli_cntx_init_cortexa53.c
LOCAL_CFLAGS    := $(LOCAL_CFLAGS_REF_KERNELS) -mcpu=cortex-a53 -ftree-vectorize -D_GNU_SOURCE -DBLIS_CNAME=cortexa53
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := ref_kernels_generic
LOCAL_SRC_FILES := $(LOCAL_SRC_FILES_REF_KERNELS) ../3rd-party/blis/config/generic/bli_cntx_init_generic.c
LOCAL_CFLAGS    := $(LOCAL_CFLAGS_REF_KERNELS) -DBLIS_CNAME=generic
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := blis
LOCAL_SRC_FILES := $(shell find -L ../3rd-party/blis/frame ! -path "*old*" ! -path "*other*" ! -path "*attic*" ! -path "*amd*" -name "*.c")
LOCAL_CFLAGS    := $(LOCAL_CFLAGS_COMMON) -D_GNU_SOURCE -I../3rd-party/blis/frame/compat/cblas/src
LOCAL_LDLIBS    := -lm -ldl

LOCAL_STATIC_LIBRARIES := ref_kernels_armsve ref_kernels_firestorm ref_kernels_thunderx2 ref_kernels_cortexa57 ref_kernels_cortexa53 ref_kernels_generic
include $(BUILD_SHARED_LIBRARY)
