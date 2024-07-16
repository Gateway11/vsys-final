#ifndef __AUDIO_CONTROL_WRAP_H__
#define __AUDIO_CONTROL_WRAP_H__

__BEGIN_DECLS

typedef enum {
    HAL_AUDIO_USAGE_UNKNOWN = 0,
    HAL_AUDIO_USAGE_EMERGENCY,
    HAL_AUDIO_USAGE_SAFETY,
} hal_audio_usage_t;

typedef enum {
    HAL_AUDIO_FOCUS_NONE = 0,
    HAL_AUDIO_FOCUS_GAIN = 1,
    HAL_AUDIO_FOCUS_GAIN_TRANSIENT = 2,
    HAL_AUDIO_FOCUS_GAIN_TRANSIENT_MAY_DUCK = 3,
    HAL_AUDIO_FOCUS_GAIN_TRANSIENT_EXCLUSIVE = 4,
    HAL_AUDIO_FOCUS_LOSS = -1 * HAL_AUDIO_FOCUS_GAIN,
    HAL_AUDIO_FOCUS_LOSS_TRANSIENT = -1 * HAL_AUDIO_FOCUS_GAIN_TRANSIENT,
    HAL_AUDIO_FOCUS_LOSS_TRANSIENT_CAN_DUCK = -1 * HAL_AUDIO_FOCUS_GAIN_TRANSIENT_MAY_DUCK,
} hal_audio_focus_t;

void set_audio_hal_bridge();

void hidl_died();

void request_audio_focus(hal_audio_usage_t usage, int32_t zone_id, hal_audio_focus_t focus);

void abandon_audio_focus(hal_audio_usage_t usage, int32_t zone_id);

__END_DECLS

#endif
