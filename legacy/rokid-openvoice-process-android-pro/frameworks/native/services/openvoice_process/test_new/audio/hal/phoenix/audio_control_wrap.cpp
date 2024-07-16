#include <thread>

#include "audio_control_wrap.h"
#include "audio_hal_bridge.h"
#include "hal_hidl_utils.h"

#include "android/hardware/automotive/audiocontrol/2.0/IAudioControl.h"

using ::android::hardware::automotive::audiocontrol::V2_0::IAudioControl;
using ::android::hardware::automotive::audiocontrol::V2_0::AudioFocusChange;

using android::hardware::audio::common::V6_0::AudioUsage;
using android::hardware::hidl_bitfield;

android::sp<IAudioControl> audioControl = nullptr;
android::sp<AudioHalBridge> halBridge = new AudioHalBridge;

void set_audio_hal_bridge() {
    audioControl = IAudioControl::getService();
    if (audioControl) {
        audioControl->setAudioHalBridge(halBridge);
        audioControl->linkToDeath(new AudioHalBridge::DeathNotifier, 0);
    }
}

void hidl_died() {
    audioControl.clear();
    std::thread thread([] {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            set_audio_hal_bridge();
        });
    thread.detach();
}

void request_audio_focus(hal_audio_usage_t usage, int32_t zone_id, hal_audio_focus_t focus) {
    if (audioControl) {
        AudioUsage usageEnum = getUsageEnumFromHal(usage);
        AudioFocusChange focusEnum = getFocusEnumFromHal(focus);
        audioControl->requestAudioFocus(
                static_cast<hidl_bitfield<AudioUsage>>(usageEnum),
                zone_id,
                static_cast<hidl_bitfield<AudioFocusChange>>(focusEnum));
    }
}

void abandon_audio_focus(hal_audio_usage_t usage, int32_t zone_id) {
    if (audioControl) {
        AudioUsage usageEnum = getUsageEnumFromHal(usage);
        audioControl->abandonAudioFocus(
                static_cast<hidl_bitfield<AudioUsage>>(usageEnum), zone_id);
    }
}
