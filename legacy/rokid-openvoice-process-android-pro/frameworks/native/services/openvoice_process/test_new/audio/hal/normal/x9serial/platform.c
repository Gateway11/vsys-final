#define LOG_TAG "audio_hw_platform"
// #define LOG_NDEBUG 0

#include "remote.h"
#include "platform.h"
#include "hal_streamer.h"
#include "platform_interface.h"
#include "audio_extra.h"
#include "audio_sharing.h"
#include <dlfcn.h>
#include <hardware/audio_effect.h>
#include <log/log.h>
#include <stdlib.h>
#include <unistd.h>

extern int card_0;
extern int card_1;

#define UNUSED(x) (void)(x)

#define MIXER_XML_NAME "mixer_paths"
#define MIXER_FILE_PATH "/vendor/etc/"
extern struct pcm_config *pcm_main_config;

typedef struct name_map_t {
    char name_linux[CARD_NAME_LENGTH];
    char name_android[CARD_NAME_LENGTH];
} name_map;

static name_map audio_name_map[MAX_AUDIO_DEVICES] = {
    {"x9refak7738", AUDIO_NAME_PRIMARY},     // REF A03 primary card
    {"x9ref04ak7738", AUDIO_NAME_PRIMARY},   // REF A04 primary card
    {"x9refmach", AUDIO_NAME_PRIMARY},       // REF A04 remote card
    {"x9msmach", AUDIO_NAME_PRIMARY},        // MS A03 remote card
    {"d9refes7144", AUDIO_NAME_PRIMARY},     // D9 playback card
    {"d9refes7243e", AUDIO_NAME_CAPTURE},    // D9 capture card
    {"x9icl02ak7738", AUDIO_NAME_PRIMARY},   // icl 02 primary card
    {"x9core01ak7738", AUDIO_NAME_PRIMARY},  // MS 01 primary card
    {"x9refak4556", AUDIO_NAME_REAR1},       // REF A02 rear seat card
    {"x9reftlv320aic2", AUDIO_NAME_REAR1},   // rear seat card 1
    {"x9reftlv320aic3", AUDIO_NAME_REAR1},   // rear seat card 1
    {"x9umsb1tlv320ai", AUDIO_NAME_REAR3},   // rear seat card 3
    {"x9msmach", AUDIO_NAME_BT},             // bt call card
};

static int find_name_map(char *linux_name, char *android_name) {
    int index = 0;

    if (linux_name == 0 || android_name == 0) {
        ALOGE("error params");
        return -1;
    }

    for (; index < MAX_AUDIO_DEVICES; index++) {
        if (strlen(audio_name_map[index].name_linux) == 0) {
            sprintf(android_name, "AUDIO_USB_%d", index);
            strcpy(audio_name_map[index].name_linux, linux_name);
            strcpy(audio_name_map[index].name_android, android_name);
            // adev->is_usb_exist = true;
            ALOGD("linux name = %s, android name = %s", audio_name_map[index].name_linux,
                  audio_name_map[index].name_android);
            return 0;
        }

        if (!strcmp(linux_name, audio_name_map[index].name_linux)) {
            strcpy(android_name, audio_name_map[index].name_android);
            ALOGD("linux name = %s, android name = %s", audio_name_map[index].name_linux,
                  audio_name_map[index].name_android);
            return 0;
        }
    }

    return 0;
}

static int do_init_audio_card(struct platform_data *platform, int card) {
    int ret = -1;
    int fd = 0;
    char *location = "/sys/class/sound";
    char snd_path[128], snd_node[128];

    if (!platform) {
        ALOGE("%s() invalid argument!", __func__);
        return -1;
    }

    memset(snd_path, 0, sizeof(snd_path));
    memset(snd_node, 0, sizeof(snd_node));

    sprintf(snd_path, "%s/card%d", location, card);
    ret = access(snd_path, F_OK);
    if (ret == 0) {
        // id / name
        sprintf(snd_node, "%s/card%d/id", location, card);
        AALOGI("read card %s/card%d/id", location, card);

        fd = open(snd_node, O_RDONLY);
        if (fd > 0) {
            ret = read(fd, platform->dev_manager[card].linux_name,
                       sizeof(platform->dev_manager[card].linux_name));
            if (ret > 0) {
                platform->dev_manager[card].linux_name[ret - 1] = 0;
                AALOGI("%s, %s, len: %d", snd_node, platform->dev_manager[card].linux_name, ret);
            }
            close(fd);
        } else {
            return -1;
        }

        find_name_map(platform->dev_manager[card].linux_name,
                      platform->dev_manager[card].android_name);
        AALOGI("find name map, linux_name = %s, android_name = %s ",
              platform->dev_manager[card].linux_name, platform->dev_manager[card].android_name);

        platform->dev_manager[card].card = card;
        platform->dev_manager[card].device = 0;
        platform->dev_manager[card].flag_exist = true;

        // playback device
        sprintf(snd_node, "%s/card%d/pcmC%dD0p", location, card, card);
        ret = access(snd_node, F_OK);
        if (ret == 0) {
            // there is a playback device
            AALOGI("snd node: %s, card : %d", snd_node, card);
            platform->dev_manager[card].flag_out = AUDIO_OUT;
            platform->dev_manager[card].flag_out_active = 0;
            if (!(strcmp(platform->dev_manager[card].android_name, AUDIO_NAME_PRIMARY))) {
                memcpy(&platform->dev_manager[card].out_pcm_cfg, pcm_main_config,
                       sizeof(struct pcm_config));
            }
        }

        // capture device
        sprintf(snd_node, "%s/card%d/pcmC%dD0c", location, card, card);
        ret = access(snd_node, F_OK);
        if (ret == 0) {
            // there is a capture device
            platform->dev_manager[card].flag_in = AUDIO_IN;
            platform->dev_manager[card].flag_in_active = 0;
        }
    } else {
        return -1;
    }

    return 0;
}

static void sd_platform_init_audio_devices(struct platform_data *platform) {
    int card = 0;

    if (!platform) {
        ALOGE("%s() invalid argument!", __func__);
        return;
    }

    memset(platform->dev_manager, 0, sizeof(platform->dev_manager));

    for (card = 0; card < MAX_AUDIO_DEVICES; card++) {
        if (do_init_audio_card(platform, card) == 0) {
            ALOGV("card: %d, name: %s, capture: %d, playback: %d", card,
                  platform->dev_manager[card].android_name,
                  platform->dev_manager[card].flag_in == AUDIO_IN,
                  platform->dev_manager[card].flag_out == AUDIO_OUT);
        }
    }
}

int sd_platform_update_audio_devices(struct platform_data *platform) {
    int card = 0;
    int ret = -1;
    char *location = "/sys/class/sound";
    char snd_path[128];

    if (!platform) {
        ALOGE("platform is not inited!");
        return -1;
    }

    memset(snd_path, 0, sizeof(snd_path));

    for (card = 0; card < MAX_AUDIO_DEVICES; card++) {
        sprintf(snd_path, "%s/card%d", location, card);
        ret = access(snd_path, F_OK);
        if (ret == 0) {
            if (platform->dev_manager[card].flag_exist == true) {
                continue;  // no changes
            } else {
                // plug-in
                ALOGD("do init audio card");
                do_init_audio_card(platform, card);
            }
        } else {
            if (platform->dev_manager[card].flag_exist == false) {
                continue;  // no changes
            } else {
                // plug-out
                platform->dev_manager[card].flag_exist = false;
                platform->dev_manager[card].flag_in = 0;
                platform->dev_manager[card].flag_out = 0;
            }
        }
    }

    return 0;
}

int sd_platform_get_card_id(struct platform_data *platform, const char *android_name) {
    int i = 0;

    if (!platform) {
        ALOGE("platform is not inited!");
        return -1;
    }
    for (i = 0; i < MAX_AUDIO_DEVICES; i++) {
        if (!(strcmp(android_name, AUDIO_NAME_USB))) {
            if (!strncmp(platform->dev_manager[i].android_name, AUDIO_NAME_USB, 9)) {
                return platform->dev_manager[i].card;
            }

        } else {
            if (!(strcmp(platform->dev_manager[i].android_name, android_name))) {
                return platform->dev_manager[i].card;
            }
        }
    }
    ALOGE("can not find device %s!", android_name);
    return -1;
}

int sd_platform_get_card_info(struct platform_data *platform, char *addr, int32_t *card,
                              int32_t *device, struct pcm_config *pcm_cfg) {
    if (!platform) {
        ALOGE("platform is not inited!");
        return -1;
    }

    for (int i = 0; i < platform->info.card_num; i++) {
        // if (bus_id == platform->info.cards[i].bus_id) {
        AALOGV("%d addr: %s", i, platform->info.cards[i].address);
        if (!strcmp(addr, platform->info.cards[i].address)) {
            *card = platform->info.cards[i].card;
            *device = platform->info.cards[i].device;
            pcm_cfg->rate = platform->info.cards[i].pcm_cfg.rate;
            pcm_cfg->format = platform->info.cards[i].pcm_cfg.format;
            pcm_cfg->channels = platform->info.cards[i].pcm_cfg.channels;
            AALOGI("addr: %s, card: %d, device: %d, rate: %d, channels: %d, format: %d", addr,
                   *card, *device, pcm_cfg->rate, pcm_cfg->channels, pcm_cfg->format);
            return 0;
        }
    }
    AALOGE("can not card info for bus %s!", addr);
    return -1;
}

int sd_platform_get_card_name(struct platform_data *platform, const char *android_name,
                              char *linux_name) {
    int i = 0;

    if (!platform) {
        ALOGE("audio platform is not inited!");
        return -1;
    }

    for (i = 0; i < MAX_AUDIO_DEVICES; i++) {
        if (!(strcmp(platform->dev_manager[i].android_name, android_name))) {
            strcpy(linux_name, platform->dev_manager[i].linux_name);
            return platform->dev_manager[i].card;
        }
    }
    ALOGE("can not find device %s!", android_name);
    return -1;
}

int32_t sd_platform_get_card_pcm_config(struct platform_data *platform, const char *android_name,
                                        struct pcm_config *pcm_cfg, int32_t *period_ms,
                                        int32_t direct) {
    int i = 0;

    if (!platform) {
        ALOGE("audio platform is not inited!");
        return -EINVAL;
    }

    for (i = 0; i < MAX_AUDIO_DEVICES; i++) {
        if (!(strcmp(platform->dev_manager[i].android_name, android_name))) {
            if (direct == AUDIO_HAL_PLAYBACK) {
                pcm_cfg->rate = platform->dev_manager[i].out_pcm_cfg.rate;
                pcm_cfg->channels = platform->dev_manager[i].out_pcm_cfg.channels;
                pcm_cfg->format = platform->dev_manager[i].out_pcm_cfg.format;
                pcm_cfg->period_size = platform->dev_manager[i].out_pcm_cfg.period_size;
                pcm_cfg->period_count = platform->dev_manager[i].out_pcm_cfg.period_count;
            }
            return 0;
        }
    }
    ALOGE("can not find device %s!", android_name);
    return -EPIPE;
}

static int get_headset_status(char *android_name) {
    int fd = 0;
    int status = -1;
    char value[32];
    char *path_gpio = "/sys/class/gpio";
    char path_value[128];

    memset(path_value, 0, sizeof(path_value));

    if (strcmp(android_name, AUDIO_NAME_REAR2)) {
        sprintf(path_value, "%s/gpio%d/value", path_gpio, SSB_JACK_REAR2_GPIO);
    } else if (strcmp(android_name, AUDIO_NAME_REAR3)) {
        sprintf(path_value, "%s/gpio%d/value", path_gpio, SSB_JACK_REAR3_GPIO);
    } else {
        ALOGE("unsupport %s jack detect.", android_name);
        return -1;
    }

    if (access(path_value, F_OK) == 0) {
        if ((fd = open(path_value, O_RDONLY)) > 0) {
            if (read(fd, value, sizeof(value)) > 0) {
                status = atoi(value);
            } else {
                ALOGE("read %s error !", path_value);
                return -1;
            }
        } else {
            ALOGE("open %s error !", path_value);
            return -1;
        }
    } else {
        ALOGE("access %s error !", path_value);
        return -1;
    }

    ALOGD("%s jack status : %d", android_name, status);
    return status;
}

int get_hs_connect_status() {
    int hs_status = 0;

    if (get_headset_status(AUDIO_NAME_REAR2) > 0) {
        hs_status |= REAR_SEAT2_HS_STATUS;
        ALOGD("rear set 2 headset is connect");
    } else {
        hs_status &= ~REAR_SEAT2_HS_STATUS;
        ALOGD("rear set 2 headset is disconnect");
    }
    if (get_headset_status(AUDIO_NAME_REAR3) > 0) {
        hs_status |= REAR_SEAT3_HS_STATUS;
        ALOGD("rear set 3 headset is connect");
    } else {
        hs_status &= ~REAR_SEAT3_HS_STATUS;
        ALOGD("rear set 3 headset is disconnect");
    }

    return hs_status;
}

int get_active_hs_device(struct alsa_audio_device *adev) {
    int status = get_hs_connect_status();

    if (adev->hs_device == REAR_SEAT3_HS_STATUS) {
        if (status & REAR_SEAT3_HS_STATUS) {
            AALOGI("active hs device : %d", REAR_SEAT3_HS_STATUS);
            return REAR_SEAT3_HS_STATUS;
        }
    }
    if (status & REAR_SEAT2_HS_STATUS) {
        AALOGI("active hs device : %d", REAR_SEAT2_HS_STATUS);
        return REAR_SEAT2_HS_STATUS;
    } else if (status & REAR_SEAT3_HS_STATUS) {
        AALOGI("active hs device : %d", REAR_SEAT3_HS_STATUS);
        return REAR_SEAT3_HS_STATUS;
    }
    AALOGI(" no active hs device");

    return 0;
}

int32_t sd_platform_get_active_microphones(void *platform, unsigned int channels,
                                           struct audio_microphone_characteristic_t *mic_array,
                                           size_t *mic_count) {
    ALOGE("%s() in", __func__);
    return -ENOSYS;
}

bool sd_platform_set_microphone_characteristic(void *platform,
                                               struct audio_microphone_characteristic_t mic) {
    struct platform_data *my_data = (struct platform_data *)platform;
    if (my_data->declared_mic_count >= AUDIO_MICROPHONE_MAX_COUNT) {
        ALOGE("mic number is more than maximum number");
        return false;
    }
    // for (size_t ch = 0; ch < AUDIO_CHANNEL_COUNT_MAX; ch++) {
    //     mic.channel_mapping[ch] = AUDIO_MICROPHONE_CHANNEL_MAPPING_UNUSED;
    // }
    my_data->microphones[my_data->declared_mic_count++] = mic;
    ALOGI("%s declared_mic_count: %d", __func__, my_data->declared_mic_count);
    return true;
}
static void process_microphone_characteristic(void *platform) {
    struct platform_data *my_data = (struct platform_data *)platform;
    struct audio_microphone_characteristic_t *microphone = NULL;
    if (!my_data) {
        ALOGE("%s() invalid argument!", __func__);
        return;
    }
    microphone = (struct audio_microphone_characteristic_t *)malloc(
        sizeof(struct audio_microphone_characteristic_t));
    if (!microphone) {
        ALOGE("microphone malloc failed");
        return;
    }
    microphone->sensitivity = AUDIO_MICROPHONE_SENSITIVITY_UNKNOWN;
    microphone->max_spl = AUDIO_MICROPHONE_SPL_UNKNOWN;
    microphone->min_spl = AUDIO_MICROPHONE_SPL_UNKNOWN;
    microphone->orientation.x = 0.0f;
    microphone->orientation.y = 0.0f;
    microphone->orientation.z = 0.0f;
    microphone->geometric_location.x = AUDIO_MICROPHONE_COORDINATE_UNKNOWN;
    microphone->geometric_location.y = AUDIO_MICROPHONE_COORDINATE_UNKNOWN;
    microphone->geometric_location.z = AUDIO_MICROPHONE_COORDINATE_UNKNOWN;

    sd_platform_set_microphone_characteristic(my_data, *microphone);
}

int32_t sd_platform_init(struct alsa_audio_device *adev) {
    int32_t ret = 0;
    char mixer_xml_file[MIXER_PATH_MAX_LENGTH] = {0};
    int snd_card_num = 0;
    const char *snd_card_name = NULL;
    struct mixer *mixer = NULL;
    struct audio_route_l *ar_l = NULL;
    struct audio_route *ar = NULL;
    struct snd_card_split *snd_split_handle = NULL;
    struct listnode *p = NULL;
    struct listnode *q = NULL;
    struct platform_data *platform = NULL;
    if (NULL == adev)
        return -EINVAL;
    platform = (struct platform_data *)calloc(1, sizeof(struct platform_data));
    if (!platform)
        goto init_failed;
    memset(platform, 0, sizeof(struct platform_data));
    platform->remote = -1;
    platform->adev = adev;
    platform->declared_mic_count = 0;

    process_microphone_characteristic(platform);

    adev->platform = platform;
    sd_platform_init_audio_devices(platform);
    platform_info_init((void *)platform);

    snd_card_num = audio_extra_utils_get_snd_card_num(adev);
    if (-1 == snd_card_num) {
        ALOGE("%s: invalid sound card number (-1), bailing out ", __func__);
        ret = -ENOSYS;
        goto init_failed;
    }

    for (int i = 0; i < snd_card_num; i++) {
        if (!strncmp(platform->dev_manager[i].linux_name, "d9", 2)) {
            ALOGV("skip d9 audio device mixer file");
            continue;
        }
        if (!strncmp(platform->dev_manager[i].android_name, AUDIO_NAME_USB, 9)) {
            ALOGV("skip open usb audio device mixer file");
            continue;
        }
        if (strstr(platform->dev_manager[i].linux_name, "mach")) {
            if (!platform->info.misc.ahub_en) {
                audio_remote_init(adev);
            }
            continue;
        }
        if (platform->info.misc.audio_route)
            mixer = mixer_open(i);

        if (mixer) {
            snd_card_name = mixer_get_name(mixer);
            ALOGV("snd_card_name: %s", snd_card_name);
            audio_extra_set_snd_card_split(snd_card_name);
            snd_split_handle = audio_extra_get_snd_card_split();
            if (snd_split_handle) {
                memset(mixer_xml_file, 0, sizeof(mixer_xml_file));
                snprintf(mixer_xml_file, sizeof(mixer_xml_file), "%s_%s_%s_%s.xml", MIXER_XML_NAME,
                         snd_split_handle->chip, snd_split_handle->board, snd_split_handle->codec);
                audio_extra_utils_resolve_config_file(mixer_xml_file);
                ALOGI("%s: Loading mixer file: %s", __func__, mixer_xml_file);
                ar = audio_route_init(i, mixer_xml_file);
                if (!ar) {
                    ALOGE(
                        "%s: Failed to init audio route "
                        "controls, aborting.",
                        __func__);
                    ret = -ENOEXEC;
                    goto init_failed;
                }
                ar_l = (struct audio_route_l *)calloc(1, sizeof(struct audio_route_l));
                ar_l->audio_route = ar;
                ar_l->card_id = i;
                list_add_tail(&adev->audio_route_list, &(ar_l->list));
                ALOGV(
                    "platform put audio to audio_route_list "
                    "snd:%d->route%p",
                    i, ar);
            } else {
                ALOGE("%s: Failed to got split, snd_card_name:%s", __func__, snd_card_name);
                ret = -ENOEXEC;
                goto init_failed;
            }
            if (i < sizeof(adev->mixers) / sizeof(struct mixer *))
                adev->mixers[i] = mixer;
        }
    }
#ifdef ENABLE_AUDIO_SHAREING
    au_shr_cfg_t rep_cfg = {
        .dev = adev,
    };
    platform->sharing = au_sharing_init(&rep_cfg);
#endif
    return ret;

init_failed:
    ALOGE("Platform init failed");
    if (mixer) {
        mixer_close(mixer);
        mixer = NULL;
    }
    list_for_each_safe(p, q, &adev->audio_route_list) {
        ar_l = node_to_item(p, struct audio_route_l, list);
        if (ar_l) {
            list_remove(&ar_l->list);
            audio_route_free(ar_l->audio_route);
            ar_l->audio_route = NULL;
            free(ar_l);
            ar_l = NULL;
        }
    }
    if (platform)
        free(platform);

    return ret;
}

int32_t sd_platform_deinit(struct alsa_audio_device *adev) {
    int32_t i = 0;
    if (!adev) {
        ALOGE("%s, invalid arg\n", __func__);
        return -EINVAL;
    }
    if (!adev->platform) {
        for (; i < adev->platform->declared_mic_count; i++) {
            if (&(adev->platform->microphones[i])) {
                free(&adev->platform->microphones[i]);
            }
        }
    }
    if (adev->platform->remote) {
        close(adev->platform->remote);
        adev->platform->remote = -1;
    }
    return 0;
}

int sd_effect_init(struct audio_stream *stream, sd_effect_direction_t direction,
                   const char *lib_path) {
    int ret = 0;
    void *hdl = NULL;
    audio_effect_library_t *desc;
    struct alsa_stream_out *out = NULL;
    switch (direction) {
        case STREAM_OUT_EFFECT:
            out = (struct alsa_stream_out *)stream;
            break;
        case STREAM_IN_EFFECT:
        default:
            ALOGE("Unsupport direction");
            ret = -EINVAL;
            break;
    }
    if (out) {
        out->effect_hdl = NULL;
        out->effect_desc = NULL;
    }
    hdl = dlopen(lib_path, RTLD_NOW);
    if (!hdl) {
        ALOGW("dlopen %s failed, error:%s\n", lib_path, dlerror());

        ret = -ENOSYS;
        goto error;
    }
    if (out)
        out->effect_hdl = hdl;
    desc = (audio_effect_library_t *)dlsym(hdl, AUDIO_EFFECT_LIBRARY_INFO_SYM_AS_STR);
    if (desc == NULL) {
        ALOGW("loadLibrary() could not find symbol %s", AUDIO_EFFECT_LIBRARY_INFO_SYM_AS_STR);
        ret = -ENOSYS;
        goto error;
    }
    if (out)
        out->effect_desc = desc;

    return ret;
error:
    if (hdl != NULL) {
        dlclose(hdl);
    }
    return ret;
}

int sd_platform_get_microphones(void *platform, struct audio_microphone_characteristic_t *mic_array,
                                size_t *mic_count) {
    struct platform_data *my_data = (struct platform_data *)platform;
    if (mic_count == NULL || !my_data) {
        return -EINVAL;
    }
    if (mic_array == NULL) {
        return -EINVAL;
    }

    if (*mic_count == 0) {
        *mic_count = 1;  // TODO: Read form platform config
        return 0;
    }

    size_t max_mic_count = *mic_count;
    size_t actual_mic_count = 0;
    for (size_t i = 0; i < max_mic_count && i < 2; i++) {
        mic_array[i] = my_data->microphones[i];
        actual_mic_count++;
    }
    *mic_count = actual_mic_count;
    return 0;
}

bool sd_platform_check_input(struct audio_config *config, void *platform /*__unused*/) {
    /*platform_data will save current product support params,
      that will be implement.
    */
    // struct platform_data *platform_data = (struct platform_data
    // *)platform;

    switch (config->sample_rate) {
        case 8000:
        case 11025:
        case 16000:
        case 22050:
        case 24000:
        case 32000:
        case 44100:
        case 48000:
            break;
        default:
            ALOGE("%s, unsupport sample rate : %d", __func__, config->sample_rate);
            return false;
    }

    if (config->format != AUDIO_FORMAT_PCM_16_BIT) {
        ALOGE("%s, unsupport format : %#x", __func__, config->format);
        return false;
    }

    switch (config->channel_mask) {
        case AUDIO_CHANNEL_IN_STEREO:
        case AUDIO_CHANNEL_IN_2POINT0POINT2:
        case AUDIO_CHANNEL_IN_MONO:
        case AUDIO_CHANNEL_IN_FRONT_BACK:
            break;
        default:
            ALOGE("%s, unsupport channelmask : %#x", __func__, config->channel_mask);
            return false;
    }

    return true;
}

bool sd_platform_check_output(struct audio_config *config, void *platform /*__unused*/) {
    bool ret = false;
    /*platform_data will save current product support params,
      that will be implement.
    */
    // struct platform_data *platform_data = (struct platform_data *)platform;

    if (config->format == AUDIO_FORMAT_PCM_16_BIT &&
        (config->channel_mask == AUDIO_CHANNEL_OUT_MONO ||
         config->channel_mask == AUDIO_CHANNEL_OUT_STEREO ||
         config->channel_mask == AUDIO_CHANNEL_OUT_7POINT1 ||
         config->channel_mask == AUDIO_CHANNEL_OUT_2POINT0POINT2)) {
        ret = true;
    } else {
        ALOGE("%s, unsupport output params", __func__);
    }
    return ret;
}

void platform_set_bind_bus(void *platform, const char *card_name, int32_t bind_bus) {
    hal_streamer_bind_bus(card_name, strlen(card_name), bind_bus);
}

void platform_set_card_info(void *platform, const card_info_t *card_info, int32_t direct) {
    AALOGI("card name: %s[ %d:%d ], rate: %d, fmt: %d, ch: %d", card_info->name, card_info->card,
           card_info->device, card_info->pcm_cfg.rate, card_info->pcm_cfg.format,
           card_info->pcm_cfg.channels);
    struct platform_data *pf = (struct platform_data *)platform;
    for (int32_t i = 0; i < MAX_AUDIO_DEVICES; i++) {
        if (!(strcmp(pf->dev_manager[i].linux_name, card_info->name))) {
            if (direct == AUDIO_HAL_PLAYBACK) {
                memcpy(&pf->dev_manager[i].out_pcm_cfg, &card_info->pcm_cfg,
                       sizeof(struct pcm_config));
            } else if (direct == AUDIO_HAL_CAPTURE) {
                memcpy(&pf->dev_manager[i].in_pcm_cfg, &card_info->pcm_cfg,
                       sizeof(struct pcm_config));
            }
        }
    }
    memcpy(&pf->info.cards[pf->info.card_num], card_info, sizeof(card_info_t));
    pf->info.card_num++;
}

void platform_set_misc_config(void *platform, void *misc) {
    struct platform_data *pf = (struct platform_data *)platform;
    memcpy(&pf->info.misc, misc, sizeof(struct misc_feature));
    AALOGV("%d %d %d %d %d %d", pf->info.misc.audio_route, pf->info.misc.sw_in_src,
           pf->info.misc.sw_out_src, pf->info.misc.sw_ecnr, pf->info.misc.sw_gain,
           pf->info.misc.btcall);
}

void platform_set_bt_connect(void *platform, enum bt_chip_connect_e connect) {
    struct platform_data *pf = (struct platform_data *)platform;
    AALOGI("bt connect with %d", connect);
    pf->bt_type = connect;
}

void platform_set_slotmap_config(void *platform, uint32_t bus_id, slot_map_t *map) {
    struct platform_data *pf = (struct platform_data *)platform;
    memcpy(&pf->info.output_maps[bus_id], map, sizeof(slot_map_t));
#ifdef LOG_NDEBUG
    slot_map_t *bus_map = &pf->info.output_maps[bus_id];
    AALOGV("dump bus%d slots:", bus_id);
    for (int i = 0; i < bus_map->len; i++) {
        AALOGV("slot: %d %d", i, bus_map->slots[i]);
    }
    AALOGV("type: %d, len: %d", bus_map->overlay, bus_map->len);
#endif
}
