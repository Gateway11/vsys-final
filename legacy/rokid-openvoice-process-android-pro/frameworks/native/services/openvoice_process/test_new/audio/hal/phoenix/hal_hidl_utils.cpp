#include <map>
#include "hal_hidl_utils.h"

std::map<hidl_string, const char*> busAddrString {
    {"AUDIO_USAGE_MEDIA",                           "bus0"},
    {"AUDIO_USAGE_VOICE_COMMUNICATION",             "bus2"},
    {"AUDIO_USAGE_VOICE_COMMUNICATION_SIGNALLING",  "bus2"},
    {"AUDIO_USAGE_ALARM",                           "bus6"},
    {"AUDIO_USAGE_NOTIFICATION",                    "bus6"},
    {"AUDIO_USAGE_NOTIFICATION_TELEPHONY_RINGTONE", "bus2"},
    {"AUDIO_USAGE_ASSISTANCE_ACCESSIBILITY",        "bus1"},
    {"AUDIO_USAGE_ASSISTANCE_NAVIGATION_GUIDANCE",  "bus3"},
    {"AUDIO_USAGE_ASSISTANCE_SONIFICATION",         "bus6"},
    {"AUDIO_USAGE_GAME",                            "bus0"},
    {"AUDIO_USAGE_VIRTUAL_SOURCE",                  "bus6"},
    {"AUDIO_USAGE_ASSISTANT",                       "bus1"},
    {"AUDIO_USAGE_CALL_ASSISTANT",                  "bus2"},
    {"AUDIO_USAGE_EMERGENCY",                       "bus6"},
    {"AUDIO_USAGE_SAFETY",                          "bus6"},
    {"AUDIO_USAGE_VEHICLE_STATUS",                  "bus6"},
    {"AUDIO_USAGE_ANNOUNCEMENT",                    "bus6"},
    {"AUDIO_USAGE_UNKNOWN",                         "bus0"},
    {"bus0",                                        "bus0"},
    {"bus1",                                        "bus1"},
    {"bus2",                                        "bus2"},
    {"bus3",                                        "bus3"},
    {"bus6",                                        "bus6"}};

std::map<hal_audio_usage_t, AudioUsage> halUsageEnum {
    {HAL_AUDIO_USAGE_UNKNOWN, AudioUsage::UNKNOWN},
    {HAL_AUDIO_USAGE_EMERGENCY, AudioUsage::EMERGENCY},
    {HAL_AUDIO_USAGE_SAFETY, AudioUsage::SAFETY},
};

std::map<hal_audio_focus_t, AudioFocusChange> halFocusEnum {
    {HAL_AUDIO_FOCUS_NONE, AudioFocusChange::NONE},
    {HAL_AUDIO_FOCUS_GAIN, AudioFocusChange::GAIN},
    {HAL_AUDIO_FOCUS_GAIN_TRANSIENT, AudioFocusChange::GAIN_TRANSIENT},
    {HAL_AUDIO_FOCUS_GAIN_TRANSIENT_MAY_DUCK, AudioFocusChange::GAIN_TRANSIENT_MAY_DUCK},
    {HAL_AUDIO_FOCUS_GAIN_TRANSIENT_EXCLUSIVE, AudioFocusChange::GAIN_TRANSIENT_EXCLUSIVE},
    {HAL_AUDIO_FOCUS_LOSS, AudioFocusChange::LOSS},
    {HAL_AUDIO_FOCUS_LOSS_TRANSIENT, AudioFocusChange::LOSS_TRANSIENT},
    {HAL_AUDIO_FOCUS_LOSS_TRANSIENT_CAN_DUCK, AudioFocusChange::LOSS_TRANSIENT_CAN_DUCK},
};

const char* getBusFromUsage(const hidl_string& usage) {
    return busAddrString[usage];
}

AudioUsage getUsageEnumFromHal(hal_audio_usage_t usage) {
    return halUsageEnum[usage];
}

AudioFocusChange getFocusEnumFromHal(hal_audio_focus_t focus) {
    return halFocusEnum[focus];
}
