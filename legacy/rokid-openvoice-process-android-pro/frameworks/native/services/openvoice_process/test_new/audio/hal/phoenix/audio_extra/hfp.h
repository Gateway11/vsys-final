#ifndef SEMIDRIVE_HFP_H
#define SEMIDRIVE_HFP_H
#include "audio_hw.h"

#define CTRL_DSP1_HFP_VOL "DSP1 HFP VOL"

typedef struct ecnr_config {
    int32_t rate;
    int32_t ch;
    enum pcm_format fmt;
} ecnr_config_t;

typedef struct ecnr_param {
    ecnr_config_t recv_in_cfg;
    ecnr_config_t recv_out_cfg;
    ecnr_config_t ref_in_cfg;
    ecnr_config_t mic_in_cfg;
    ecnr_config_t mic_out_cfg;
} ecnr_param_t;

struct ecnr_common_ops {
    void *(*ecnr_init)(ecnr_param_t *);
    int32_t (*ecnr_ref_in)(int32_t hdl, void *ref, int32_t ref_frames, void *args);
    int32_t (*ecnr_process)(int32_t hdl, void *in, int32_t in_frames, void *processed,
                            int32_t *processed_frames, void *args);
    int32_t (*ecnr_recv_in)(int32_t hdl, void *recv, int32_t recv_frames, void *processed,
                            int32_t *processed_frames, void *args);
    bool (*ecnr_deinit)(void *);
};

int32_t init_hfp(struct alsa_audio_device *adev);
int32_t start_hfp(struct alsa_audio_device *adev);
int32_t stop_hfp(struct alsa_audio_device *adev);
int32_t hfp_set_volume(struct alsa_audio_device *adev, int volume);
float hfp_get_amplitude_ratio(void);
int32_t hfp_get_volume_idx(struct alsa_audio_device *adev);  // %(percent)
int32_t hfp_set_mic_mute(struct alsa_audio_device *adev, bool state);
#endif /* SD_HFP_H */