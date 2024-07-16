#ifndef __HAL_HIDL_UTILS_H__
#define __HAL_HIDL_UTILS_H__

#include "audio_control_wrap.h"
#include "android/hardware/automotive/audiocontrol/2.0/types.h"
#include "android/hardware/audio/common/6.0/types.h"

using ::android::hardware::automotive::audiocontrol::V2_0::AudioFocusChange;
using ::android::hardware::audio::common::V6_0::AudioUsage;
using ::android::hardware::hidl_string;

const char* getBusFromUsage(const hidl_string& usage);

AudioUsage getUsageEnumFromHal(hal_audio_usage_t usage);

AudioFocusChange getFocusEnumFromHal(hal_audio_focus_t focus);

#endif
