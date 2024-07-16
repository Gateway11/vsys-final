#ifndef __AUDIO_HAL_BRIDGE_H__
#define __AUDIO_HAL_BRIDGE_H__

#include "android/hardware/automotive/audiocontrol/2.0/IAudioHalBridge.h"
//#include <hidl/HidlSupport.h>

using android::hardware::hidl_array;
using android::hardware::Return;

using android::hardware::automotive::audiocontrol::V2_0::IAudioHalBridge;
using android::hardware::automotive::audiocontrol::V2_0::DuckingInfo;
using android::hardware::automotive::audiocontrol::V2_0::MutingInfo;
using android::hardware::automotive::audiocontrol::V2_0::AudioParamConfig;

class AudioHalBridge : public IAudioHalBridge {

public:
    struct DeathNotifier : public android::hardware::hidl_death_recipient {
        void serviceDied(uint64_t cookie, const android::wp<::android::hidl::base::V1_0::IBase>& who);
    };

    Return<void> onDevicesToDuckChange(const hidl_array<DuckingInfo, 1>& duckingInfos);

    Return<void> onDevicesToMuteChange(const hidl_array<MutingInfo, 1>& mutingInfos);

    Return<int32_t> setAudioParamConfig(const AudioParamConfig& config);
};

#endif
