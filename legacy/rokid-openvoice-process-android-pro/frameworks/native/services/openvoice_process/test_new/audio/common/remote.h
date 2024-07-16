#ifndef SEMIDRIVE_REMOTE_H
#define SEMIDRIVE_REMOTE_H
#ifdef __cplusplus
extern "C" {
#endif
#ifdef AUDIO_HAL
#include "audio_hw.h"
#include <hardware/audio.h>
#include <system/audio.h>
#endif
#include <sys/ioctl.h>
typedef char *AM_PATH;
typedef signed int int32_t;
typedef unsigned char uint8_t;
#define IPCC_LEN_MAX 64
#define AUDIO_MONITOR_EPT (125)
#define AUDIO_SERVICE_EPT (128)

typedef enum {
    DP_CR5_SAF,
    DP_CR5_SEC,
    DP_CR5_MPC,
    DP_CA_AP1,
    DP_CA_AP2,
    DP_DSP_V,
    DP_CPU_MAX,
    R_DP_CR5_SAF = DP_CPU_MAX,
    R_DP_CR5_SEC,
    R_DP_CR5_MPC,
    R_DP_CA_AP1,
    R_DP_CA_AP2,
    R_DP_DSP_V,
    ALL_DP_CPU_MAX,
} domain_cpu_id_t;

struct ipcc_data {
    char data[IPCC_LEN_MAX];
    int32_t data_len;
    char data1[IPCC_LEN_MAX];
    int32_t data1_len;
    uint8_t cmd;
    uint8_t param;
    uint8_t reserved0;
    uint8_t reserved1;
};

struct ipcc_ret {
    int32_t code;
    uint8_t cmd;
    uint8_t type;
    uint8_t reserved0;
    uint8_t reserved;
};
typedef enum {
    OP_HANDSHAKE,
    OP_START,
    OP_STOP,
    OP_SET_MUTE,
    OP_GET_MUTE,
    OP_SETVOL,
    OP_SWITCH,
    OP_RESET,
    OP_RESTORE,
    OP_PARAM,
    OP_PLAY_AGENT_START,
    OP_PLAY_AGENT_STOP,
    OP_PCM_PLAYBACK_CTL,
    OP_PCM_CAPTURE_CTL,
    OP_PCM_PLAYBACK_STREAM,
    OP_PCM_CAPTURE_STREAM,
    OP_PCM_LOOPBACK_CTL,
    OP_CHIME_ON,
    OP_CHIME_OFF,
    OP_NUMB
} AM_OPCODE_TYPE;

typedef enum {
    RET_OK = 0,               ///< return OK
    RET_PATH_NOT_NEED_CHANGE, ///< audio path already in thi status,don't
                              ///< need
    ///< change
    RET_PATH_NOT_STARTED,        ///< can't not start audio path.
    RET_PATH_VOL_CHANGED,        ///< audio path's volume is changed
    RET_PATH_EXCLUDED,           ///< audio path is excluded.
    RET_PATH_DUCKED,             ///< audio path is ducked.
    RET_ERR_PATH_INVALID_VOLUME, ///< the volume is invalid value
    RET_ERR_INVALID_PATH,        ///< invalid path
    RET_ERR_INIT_FAILED,         ///< init failed
    RET_ERR_SYNC_FAILED,         ///< sync failed
    RET_ERR_SETVOL_FAILED,       ///< set volume failed
    RET_ERR_MUTE_FAILED,         ///< mute failed
    RET_ERR_PATH_INACTIVE,       ///< opcode failed for path inactive
    RET_ERR_CHECK_FAILED,        ///< check failed
    RET_ERR_NULL_FUNC,           ///< null function
    RET_ERR_PARAM_FAILD,         ///< null function
    RET_ERR_UNKNOWN
} AM_RESULT;

#define HIFI_PLAYBACK_TO_MAIN_SPK_48K "PATH_HIFI_PLAYBACK_TO_MAIN_SPK_48K"
#define HIFI_CAPTURE_FROM_MAIN_MIC_48K "PATH_HIFI_CAPTURE_FROM_MAIN_MIC_48K"
#define SAFETY_PLAYBACK_TO_MAIN_SPK_48K "PATH_SAFETY_PLAYBACK_TO_MAIN_SPK_48K"
#define FM_PLAYBACK_TO_MAIN_SPK_48K "PATH_FM_PLAYBACK_TO_MAIN_SPK_48K"
#define TBOX_PLAYBACK_TO_MAIN_SPK_48K "PATH_TBOX_PLAYBACK_TO_MAIN_SPK_48K"
#define NAV_PLAYBACK_TO_MAIN_SPK_48K "PATH_NAV_PLAYBACK_TO_MAIN_SPK_48K"
#define PLAYBACK_TO_REARSEAT_L_48K "PATH_PLAYBACK_TO_REARSEAT_L_48K"
#define PLAYBACK_TO_REARSEAT_R_48K "PATH_PLAYBACK_TO_REARSEAT_R_48K"
#define CAPTURE_FROM_REARSEAT_L_48K "PATH_CAPTURE_FROM_REARSEAT_L_48K"
#define CAPTURE_FROM_REARSEAT_R_48K "PATH_CAPTURE_FROM_REARSEAT_R_48K"
/* bt connect with dsp */
#define BT_PLAYBACK_TO_MAIN_SPK_16K "PATH_BT_PLAYBACK_TO_MAIN_SPK_16K"
#define BT_CAPTURE_FROM_MAIN_MIC_16K "PATH_BT_CAPTURE_FROM_MAIN_MIC_16K"
#define BT_PLAYBACK_TO_MAIN_SPK_8K "PATH_BT_PLAYBACK_TO_MAIN_SPK_8K"
#define BT_CAPTURE_FROM_MAIN_MIC_8K "PATH_BT_CAPTURE_FROM_MAIN_MIC_8K"
/* bt connect with soc */
#define PHONE_PLAYBACK_TO_MAIN_SPK_48K "PATH_PHONE_PLAYBACK_TO_MAIN_SPK_48K"
#define PHONE_CAPTURE_FROM_MAIN_MIC_48K "PATH_PHONE_CAPTURE_FROM_MAIN_MIC_48K"

#ifdef AUDIO_HAL
int32_t audio_remote_init(struct alsa_audio_device *adev);
int32_t audio_remote_start(struct alsa_audio_device *adev, AM_PATH path, uint32_t vol);
int32_t audio_remote_stop(struct alsa_audio_device *adev, AM_PATH path);
int32_t audio_remote_switch_path(struct alsa_audio_device *adev, AM_PATH src, AM_PATH dst, int32_t param);
int32_t audio_remote_set_volume(struct alsa_audio_device *adev, AM_PATH path, uint32_t vol);
#endif

int32_t audio_remote_ctrl_init();
int32_t audio_remote_set_param(int sock, char *path, int mode, char *param, int param_len);
int32_t audio_remote_sock_init(const char *ip_addr, int32_t port);
int32_t audio_remote_sock_close(int32_t sock);

#ifdef __cplusplus
}
#endif

#endif
