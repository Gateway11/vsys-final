#include <string.h>
#include <cutils/log.h>

#include "audio_hw.h"
#include "audio_hal_bridge.h"
#include "audio_control_wrap.h"
#include "hal_hidl_utils.h"
#include "audio_dsp_common.h"

#define LOG_TAG "AudioHalBridge"
#define LOG_NDEBUG 0

using ::android::hardware::automotive::audiocontrol::V2_0::AudioCommandType;
using ::android::hardware::automotive::audiocontrol::V2_0::AudioGeqType;

static inline bool audio_device_address_is_valid(uint32_t addr) {
    switch(addr) {
    case BUS0 ... BUS3:
    case BUS6:
          return true;
    }
    return false;
}

void AudioHalBridge::DeathNotifier::serviceDied(
        uint64_t cookie, const android::wp<::android::hidl::base::V1_0::IBase>& who) {
    ALOGE("%s", __func__);
    hidl_died();
}

Return<void> AudioHalBridge::onDevicesToDuckChange(const hidl_array<DuckingInfo, 1>& duckingInfos) {
    ALOGD("%s", toString(duckingInfos).c_str());
    for (int32_t i = 0; i < duckingInfos.size(); i++) {
        const DuckingInfo& duckingInfo = duckingInfos[i];
        if (duckingInfo.zoneId != 0) {
            return android::hardware::Void();
        }
        *(uint32_t *)current_audio_bus_status.focus_bus = 0xFFFFFFFF;
        bool hasDuckBus = false;
        for (int32_t j = 0; j < duckingInfo.usagesHoldingFocus.size(); j++) {
            const char* addr = getBusFromUsage(duckingInfo.usagesHoldingFocus[j]);
            if (addr && !strncmp(addr, "bus", 3)) {

            }
        }
        for (int32_t j = 0; j < 4; j++) {
            const char* addr = duckingInfo.deviceAddressesToUnduck[j].c_str();
            if (!strncmp(addr, "bus", 3) && audio_device_address_is_valid(atoi(addr + 3))) {
                current_audio_bus_status.focus_bus[j] = atoi(addr + 3);
            }
        }
        for (int32_t j = 0; j < duckingInfo.deviceAddressesToDuck.size(); j++) {
            const char* addr = duckingInfo.deviceAddressesToDuck[j].c_str();
            if (!strncmp(addr, "bus", 3) && audio_device_address_is_valid(atoi(addr + 3))) {
                set_device_address_is_ducked(atoi(addr + 3), true);
                hasDuckBus = true;
            }
        }
        if (!hasDuckBus) {
            for (int32_t j = 0; j < 4; j++) {
                if (current_audio_bus_status.focus_bus[j] != 0xFF) {
                    set_device_address_is_ducked(current_audio_bus_status.focus_bus[j], false);
                }
            }
        }
    }
    return android::hardware::Void();
}

Return<void> AudioHalBridge::onDevicesToMuteChange(const hidl_array<MutingInfo, 1>& mutingInfos) {
    ALOGD("%s", toString(mutingInfos).c_str());
    for (int32_t i = 0; i < mutingInfos.size(); i++) {
        const MutingInfo& mutingInfo = mutingInfos[i];
        if (mutingInfo.zoneId != 0) {
            return android::hardware::Void();
        }
        for (int32_t j = 0; j < mutingInfo.deviceAddressesToMute.size(); j++) {
            const char* addr = mutingInfo.deviceAddressesToMute[j].c_str();
            if (!strncmp(addr, "bus", 3) && audio_device_address_is_valid(atoi(addr + 3))) {
                set_device_address_is_muted(atoi(addr + 3), true);
            }
        }
        for (int32_t j = 0; j < mutingInfo.deviceAddressesToUnmute.size(); j++) {
            const char* addr = mutingInfo.deviceAddressesToUnmute[j].c_str();
            if (!strncmp(addr, "bus", 3) && audio_device_address_is_valid(atoi(addr + 3))) {
                set_device_address_is_muted(atoi(addr + 3), false);
            }
        }
    }
    return android::hardware::Void();
}

Return<int32_t> AudioHalBridge::setAudioParamConfig(const AudioParamConfig& config) {
    int32_t ret = 0;

    ALOGD("%s", toString(config).c_str());
    switch (config.command) {
    case AudioCommandType::GEQ:
        switch (config.equalizer.type) {
        case AudioGeqType::EFFECT_TYPE:
            if (config.equalizer.eqEffectType == "Default") {
                ret = dspdev_set_geq(GEQ_BASS, -2);
                ret = dspdev_set_geq(GEQ_MIDDLE_BASS, -1);
                ret = dspdev_set_geq(GEQ_MIDDLE, 1);
                ret = dspdev_set_geq(GEQ_MIDDLE_HIGH, 2);
                ret = dspdev_set_geq(GEQ_HIGH, 3);
            } else if (config.equalizer.eqEffectType == "Classic") {
                ret = dspdev_set_geq(GEQ_BASS, 2);
                ret = dspdev_set_geq(GEQ_MIDDLE_BASS, 2);
                ret = dspdev_set_geq(GEQ_MIDDLE, 0);
                ret = dspdev_set_geq(GEQ_MIDDLE_HIGH, 3);
                ret = dspdev_set_geq(GEQ_HIGH, 4);
            } else if (config.equalizer.eqEffectType == "Pop") {
                ret = dspdev_set_geq(GEQ_BASS, 4);
                ret = dspdev_set_geq(GEQ_MIDDLE_BASS, -2);
                ret = dspdev_set_geq(GEQ_MIDDLE, 4);
                ret = dspdev_set_geq(GEQ_MIDDLE_HIGH, 2);
                ret = dspdev_set_geq(GEQ_HIGH, 2);
            } else if (config.equalizer.eqEffectType == "Jazz") {
                ret = dspdev_set_geq(GEQ_BASS, 2);
                ret = dspdev_set_geq(GEQ_MIDDLE_BASS, 0);
                ret = dspdev_set_geq(GEQ_MIDDLE, 4);
                ret = dspdev_set_geq(GEQ_MIDDLE_HIGH, 0);
                ret = dspdev_set_geq(GEQ_HIGH, 2);
            } else if (config.equalizer.eqEffectType == "Percussion") {
                ret = dspdev_set_geq(GEQ_BASS, -4);
                ret = dspdev_set_geq(GEQ_MIDDLE_BASS, 2);
                ret = dspdev_set_geq(GEQ_MIDDLE, 2);
                ret = dspdev_set_geq(GEQ_MIDDLE_HIGH, -2);
                ret = dspdev_set_geq(GEQ_HIGH, -4);
            } else if (config.equalizer.eqEffectType == "Rock") {
                ret = dspdev_set_geq(GEQ_BASS, 6);
                ret = dspdev_set_geq(GEQ_MIDDLE_BASS, 0);
                ret = dspdev_set_geq(GEQ_MIDDLE, 2);
                ret = dspdev_set_geq(GEQ_MIDDLE_HIGH, 0);
                ret = dspdev_set_geq(GEQ_HIGH, 6);
            } else if (config.equalizer.eqEffectType == "Customize") {

            } else {
                ret = dspdev_set_geq(GEQ_BASS, -2);
                ret = dspdev_set_geq(GEQ_MIDDLE_BASS, -1);
                ret = dspdev_set_geq(GEQ_MIDDLE, 1);
                ret = dspdev_set_geq(GEQ_MIDDLE_HIGH, 2);
                ret = dspdev_set_geq(GEQ_HIGH, 3);
            }
            break;
        case AudioGeqType::FREQUENCY_INDEX:
            switch (config.equalizer.frequencyIdx) {
            case 0:
                ret = dspdev_set_geq(GEQ_BASS, config.values);
                break;
            case 1:
                ret = dspdev_set_geq(GEQ_MIDDLE_BASS, config.values);
                break;
            case 2:
                ret = dspdev_set_geq(GEQ_MIDDLE, config.values);
                break;
            case 3:
                ret = dspdev_set_geq(GEQ_MIDDLE_HIGH, config.values);
                break;
            case 4:
                ret = dspdev_set_geq(GEQ_HIGH, config.values);
                break;
            default:
                break;
            }
            break;
        case AudioGeqType::UNKNOWN:
        case AudioGeqType::CLOSE:
        case AudioGeqType::MAX:
        default:
            ret = dspdev_set_geq(GEQ_BASS, 0);
            ret = dspdev_set_geq(GEQ_MIDDLE_BASS, 0);
            ret = dspdev_set_geq(GEQ_MIDDLE, 0);
            ret = dspdev_set_geq(GEQ_MIDDLE_HIGH, 0);
            ret = dspdev_set_geq(GEQ_HIGH, 0);
            break;
        }
        break;
    case AudioCommandType::FIELD:
        ret = dspdev_set_sound_field(config.extraData[0], config.extraData[1]);
        break;
    case AudioCommandType::SURROUND:
    case AudioCommandType::RESTORE:
    case AudioCommandType::DTS:
    case AudioCommandType::UNKNOWN:
    default:
        break;
    }
    return ret;
}
