#ifndef SEMIDRIVE_PLATFORM_H
#define SEMIDRIVE_PLATFORM_H

#include <system/audio.h>
#include <hardware/audio.h>
#include "audio_hw.h"
#include "audio_sharing.h"
#include "platform_info.h"

#define MIXER_PATH_MAX_LENGTH 128
#define SND_DEVICE_MAX 2  // bus0 bus100
#define SSB_JACK_REAR2_GPIO 61
#define SSB_JACK_REAR3_GPIO 127
#define CARD_NAME_LENGTH 32
#define MAX_AUDIO_DEVICES 16
#define AUDIO_IN 1
#define AUDIO_OUT 2

#define AUDIO_NAME_PRIMARY "AUDIO_PRIMARY"
#define AUDIO_NAME_CAPTURE "AUDIO_CAPTURE"
#define AUDIO_NAME_REAR1 "AUDIO_REAR1"
#define AUDIO_NAME_REAR2 AUDIO_NAME_REAR1  //"AUDIO_REAR2"
#define AUDIO_NAME_REAR3 "AUDIO_REAR3"

#define AUDIO_NAME_USB "AUDIO_USB"
#define AUDIO_NAME_BT "AUDIO_BT"
#define AUDIO_NAME_NONE "AUDIO_NONE"

typedef struct audio_device_manager {
    char android_name[CARD_NAME_LENGTH];
    char linux_name[CARD_NAME_LENGTH];
    int card;
    int device;
    int flag_in;         //
    int flag_in_active;  // 0: not used, 1: used to caputre
    int flag_out;
    int flag_out_active;  // 0: not used, 1: used to playback
    bool flag_exist;      // for hot-plugging
    struct pcm_config out_pcm_cfg;
    struct pcm_config in_pcm_cfg;

} audio_device_manager;

enum bt_chip_connect_e {
    BT_CONNECT_WITH_DSP,
    BT_CONNECT_WITH_SOC,
};

typedef enum sd_effect_direction { STREAM_OUT_EFFECT, STREAM_IN_EFFECT } sd_effect_direction_t;

struct platform_data {
    struct alsa_audio_device *adev;
    int snd_card_num;
    uint32_t declared_mic_count;
    struct audio_device_manager dev_manager[MAX_AUDIO_DEVICES];
    struct audio_microphone_characteristic_t microphones[AUDIO_MICROPHONE_MAX_COUNT];
    int remote;
    int ipcc_fd;
    enum bt_chip_connect_e bt_type;
    au_sharing_t *sharing;
    au_platform_info_t info;
};

int sd_platform_update_audio_devices(struct platform_data *platform);
int sd_platform_get_card_info(struct platform_data *platform, char *addr, int32_t *card,
                              int32_t *device, struct pcm_config *pcm_cfg);
int sd_platform_get_card_id(struct platform_data *platform, const char *android_name);
int sd_platform_get_card_name(struct platform_data *platform, const char *android_name,
                              char *linux_name);

int32_t sd_platform_init(struct alsa_audio_device *adev);
int32_t sd_platform_deinit(struct alsa_audio_device *adev);
int sd_effect_init(struct audio_stream *stream, sd_effect_direction_t direction,
                   const char *lib_path);
int get_hs_connect_status();
int get_active_hs_device(struct alsa_audio_device *adev);
int32_t sd_platform_get_microphones(void *platform,
                                    struct audio_microphone_characteristic_t *mic_array,
                                    size_t *mic_count);
int32_t sd_platform_get_active_microphones(void *platform, unsigned int channels,
                                           struct audio_microphone_characteristic_t *mic_array,
                                           size_t *mic_count);
bool sd_platform_check_input(struct audio_config *config, void *platform /*__unused*/);
bool sd_platform_check_output(struct audio_config *config, void *platform /*__unused*/);
int32_t sd_platform_get_card_pcm_config(struct platform_data *platform, const char *android_name,
                                        struct pcm_config *pcm_cfg, int32_t *period_ms,
                                        int32_t direct);
#endif
