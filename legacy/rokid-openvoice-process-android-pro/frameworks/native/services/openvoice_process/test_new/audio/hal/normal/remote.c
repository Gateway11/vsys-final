/*
 * @FilePath: \audio\remote.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * @Author: mengmeng.chang
 * @Date: 2021-05-19 16:33:17
 * @Descripttion:
 * @Revision History:
 * ----------------
 */
#define LOG_TAG "audio_hw_remote"
// #define LOG_NDEBUG 0

#include "remote.h"
#include "audio_extra.h"
#include "platform.h"
#include "rpmsg_socket.h"
#include <dlfcn.h>
#include <hardware/audio_effect.h>
#include <log/log.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
static pthread_mutex_t g_ipcc_lock = PTHREAD_MUTEX_INITIALIZER;

#define IPCC_LEN_MAX 64

#define IPCC_PACKED_PARAM(cmd, param) \
    { .cmd = cmd, .param = param }

#define IPCC_PACKED_PATH(data, path)                   \
    {                                                  \
        int32_t len = strlen(path);                    \
        len = len > IPCC_LEN_MAX ? IPCC_LEN_MAX : len; \
        strncpy(data, path, len);                      \
    }

#define RE_TRY_MAX 2

bool ipcc_init(struct alsa_audio_device *adev);

int32_t ipcc_ctrl(struct alsa_audio_device *adev, void *data, uint32_t len) {
    int32_t ret = 0;
    int32_t re_try_cnt = 0;
    struct ipcc_ret ipcc_return;
    struct platform_data *platform = adev->platform;
    struct timeval start, end;
    if (data && platform->remote > 0) {
        gettimeofday(&start, NULL);
        pthread_mutex_lock(&g_ipcc_lock);
        while(1)
        {
            ret = write(platform->remote, data, len);
            if (ret > 0)
                break;

            if (re_try_cnt++ > RE_TRY_MAX)
            {
                AALOGE("Write time out error %d (%s)", errno, strerror(errno));
                goto ctrl_exit;
            }
        }

        re_try_cnt = 0;

        while(1) {
            ret = read(platform->remote, &ipcc_return, sizeof(struct ipcc_ret));
            if (ret > 0)
                break;

            if (re_try_cnt++ > RE_TRY_MAX)
            {
                AALOGE("Read time out error %d (%s)", errno, strerror(errno));
                goto ctrl_exit;
            }
        }
        ret = ipcc_return.code;
ctrl_exit:
        pthread_mutex_unlock(&g_ipcc_lock);
        gettimeofday(&end, NULL);
        float interval = 1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
        AALOGI("ipcc_ctrl len %d ret %d return: %d, cost: %f ms", len, ret, ipcc_return.code,
               interval / 1000.0);
    } else {
        ret = -1;
        AALOGE("fd or data is invalid fd %d", platform->remote);
        ipcc_init(adev);
    }
    return ret;
}

bool ipcc_init(struct alsa_audio_device *adev) {
    int ret;
    struct timeval tv = { .tv_usec = 300/* ms */ * 1000 };
    struct sockaddr_rpmsg addr;
    struct platform_data *platform = adev->platform;
    AALOGI("ipcc_init");
    platform->remote = socket(PF_RPMSG, SOCK_SEQPACKET, 0);
    if (platform->remote < 0) {
        AALOGE("ipcc init failed, errno: %d (%s)", platform->remote, strerror(errno));
        return false;
    }

    addr.family = AF_RPMSG;
#ifdef IPCC_RPC_RPROC_MP
    addr.vproc_id = DP_CR5_MPC;
#else
    addr.vproc_id = DP_CR5_SAF;
#endif
    addr.addr = AUDIO_MONITOR_EPT;

    if (setsockopt(platform->remote, SOL_SOCKET, SO_RCVTIMEO,
            &tv, sizeof(tv)) < 0)
        AALOGE("Set rcv timeo failed");

    if (setsockopt(platform->remote, SOL_SOCKET, SO_SNDTIMEO,
            &tv, sizeof(tv)) < 0)
        AALOGE("Set snd timeo failed");

    ret = connect(platform->remote, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        AALOGE("ipcc connect failed(%s)", strerror(errno));
        platform->remote = -1;
        return false;
    }
    AALOGI("ipcc init success");
    return true;
}

int32_t audio_remote_init(struct alsa_audio_device *adev) {
    return ipcc_init(adev);
}

int32_t audio_remote_start(struct alsa_audio_device *adev, AM_PATH path, uint32_t param) {
    int ret;
    int cmd = OP_START;
    struct ipcc_data pack = IPCC_PACKED_PARAM(cmd, param);
    IPCC_PACKED_PATH(pack.data, path);
    ret = ipcc_ctrl(adev, &pack, sizeof(struct ipcc_data));
    AALOGI("path : %s, vol: %d, ret: %d", pack.data, pack.param, ret);
    return ret;
}

int32_t audio_remote_switch_path(struct alsa_audio_device *adev, AM_PATH src, AM_PATH dst,
                                 int32_t param) {
    int ret;
    int cmd = OP_START;
    struct ipcc_data pack = IPCC_PACKED_PARAM(cmd, param);
    IPCC_PACKED_PATH(pack.data, src);
    IPCC_PACKED_PATH(pack.data1, dst);
    ret = ipcc_ctrl(adev, &pack, sizeof(struct ipcc_data));
    AALOGV("path : %s, vol: %d, ret: %d", pack.data, pack.param, ret);
    return ret;
}

int32_t audio_remote_stop(struct alsa_audio_device *adev, AM_PATH path) {
    int32_t ret;
    int32_t cmd = OP_STOP;
    int32_t param = 0;
    struct ipcc_data pack = IPCC_PACKED_PARAM(cmd, param);
    IPCC_PACKED_PATH(pack.data, path);
    ret = ipcc_ctrl(adev, &pack, sizeof(struct ipcc_data));
    AALOGI("path : %s, vol: %d, ret: %d", pack.data, pack.param, ret);
    return 0;
}

int32_t audio_remote_set_volume(struct alsa_audio_device *adev, AM_PATH path, uint32_t param) {
    int32_t ret;
    int32_t cmd = OP_SETVOL;
    struct ipcc_data pack = IPCC_PACKED_PARAM(cmd, param);
    IPCC_PACKED_PATH(pack.data, path);
    ret = ipcc_ctrl(adev, &pack, sizeof(struct ipcc_data));

    AALOGV("setvol path : %s, vol: %d, ret: %d", path, param, ret);
    return ret;
}
