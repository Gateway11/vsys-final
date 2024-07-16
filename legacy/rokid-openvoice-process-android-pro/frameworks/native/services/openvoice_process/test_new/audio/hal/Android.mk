# Copyright (C) 2016 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)

# The default audio HAL module, which is a stub, that is loaded if no other
# device specific modules are present. The exact load order can be seen in
# libhardware/hardware.c
#
# The format of the name is audio.<type>.<hardware/etc>.so where the only
# required type is 'primary'. Other possibilites are 'a2dp', 'usb', etc.
include $(CLEAR_VARS)

LOCAL_MODULE := audio.primary.$(TARGET_BOARD)
#LOCAL_PROPRIETARY_MODULE := true
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_STATIC_LIBRARIES := libperfapi
ifneq ($(filter x9 d9,$(TARGET_BOARD_PLATFORM)),)
   AUDIO_PLATFORM = x9serial
endif

ifneq ($(filter x9hp_phoenix x9sp_a2_phoenix,$(TARGET_BOARD)),)
    AUDIO_FILE_PATH = phoenix
else
    AUDIO_FILE_PATH = normal
endif

LOCAL_SRC_FILES := \
        $(AUDIO_FILE_PATH)/audio_extra/audio_extra.c \
        $(AUDIO_FILE_PATH)/audio_extra/hfp.c \
        $(AUDIO_FILE_PATH)/audio_extra/hal_streamer.c \
        $(AUDIO_FILE_PATH)/audio_extra/ext_circlebuffer.c \
        $(AUDIO_FILE_PATH)/plugin/pcm_plugin.c \
        $(AUDIO_FILE_PATH)/plugin/pcm_wrapper.c \
        $(AUDIO_FILE_PATH)/remote.c  \
        $(AUDIO_FILE_PATH)/ecnr_fake.c \
        $(AUDIO_FILE_PATH)/audio_bt_call.c \
        $(AUDIO_FILE_PATH)/platform_info.c \
        $(AUDIO_FILE_PATH)/dsp.c \
        $(AUDIO_FILE_PATH)/$(AUDIO_PLATFORM)/platform.c

ifeq ($(PRODUCT_BRAND), D9)
LOCAL_CFLAGS += -DPLATFORM_D9
LOCAL_SRC_FILES += \
        $(AUDIO_FILE_PATH)/audio_hw_normal.c
else
LOCAL_SRC_FILES += \
        $(AUDIO_FILE_PATH)/audio_hw.c
endif

LOCAL_SHARED_LIBRARIES := liblog libcutils libaudioutils libtinyalsa libaudioroute  libutils libexpat
LOCAL_CFLAGS := -Wno-unused-parameter -DAUDIO_HAL

ifeq ($(SUPPORT_AUDIO_CHANN_MAP), true)
LOCAL_CFLAGS += -DUSE_CHANNEL_MAP
endif

ifeq ($(SUPPORT_AUDIO_INPUT_SRC), true)
LOCAL_CFLAGS += -DENABLE_INPUT_RESAMPLER
endif

ifeq ($(IPCC_RPC_RPROC_MP), true)
LOCAL_CFLAGS += -DIPCC_RPC_RPROC_MP
endif

ifeq ($(SUPPORT_FIX_CAPTURE_SAMPLE_RATE), true)
LOCAL_CFLAGS += -DENABLE_FIX_CAPTURE_SAMPLE_RATE
endif

ifeq ($(SUPPORT_AUDIO_BTCALL), true)
LOCAL_CFLAGS += -DENABLE_BTCALL
endif

ifeq ($(TARGET_BOARD), x9u_ref_a)
LOCAL_CFLAGS += -DX9U_REF_A -DENABLE_AUDIO_SHAREING
LOCAL_SRC_FILES +=  \
        $(AUDIO_FILE_PATH)/audio_sharing.c
endif

ifeq ($(TARGET_BOARD), x9u_ref_b)
LOCAL_CFLAGS += -DX9U_REF_B -DENABLE_AUDIO_SHAREING
LOCAL_SRC_FILES +=  \
        $(AUDIO_FILE_PATH)/audio_sharing.c
endif

ifeq ($(AUDIO_FILE_PATH), phoenix)
LOCAL_SHARED_LIBRARIES += android.hardware.automotive.audiocontrol@2.0 libhidlbase
LOCAL_SRC_FILES += \
                   $(AUDIO_FILE_PATH)/audio_hal_bridge.cpp \
                   $(AUDIO_FILE_PATH)/audio_control_wrap.cpp \
                   $(AUDIO_FILE_PATH)/hal_hidl_utils.cpp

LOCAL_CFLAGS += -Wno-unused-function #-Wno-implicit-function-declaration
endif

LOCAL_C_INCLUDES += \
        external/tinyalsa/include \
        external/expat/lib \
        system/media/audio_utils/include \
        system/media/audio_effects/include \
        system/core/include \
        system/media/audio/include \
        hardware/libhardware/include \
        system/media/audio_route/include \
        $(LOCAL_PATH)/$(AUDIO_FILE_PATH)/audio_extra \
        $(LOCAL_PATH)/$(AUDIO_FILE_PATH)/plugin \
        $(LOCAL_PATH)/$(AUDIO_FILE_PATH)/$(AUDIO_PLATFORM) \
        $(LOCAL_PATH)/$(AUDIO_FILE_PATH) \
        $(LOCAL_PATH)/../common

include $(BUILD_SHARED_LIBRARY)

