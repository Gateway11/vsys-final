#include <sys/socket.h>
#include <errno.h>
#include <mutex>
#include <string>

#include <log/log.h>

#include "remote.h"
#include "rpmsg_socket.h"
#include "dsp.h"

#undef LOG_TAG
#define LOG_TAG "dspdev"
//#define LOG_NDEBUG 0

#define IPCC_PACKED_PARAM(cmd, param)                               \
    {                                                               \
        .cmd = cmd, .param = param                                  \
    }

#define IPCC_PACKED_DATA(data, val)                                 \
    {                                                               \
        int32_t len = strlen(val);                                  \
        len = len > IPCC_LEN_MAX ? IPCC_LEN_MAX : len;              \
        strncpy(data, val, len);                                    \
    }

int32_t serverfd = -1;
std::mutex ipcc_mutex;

void dspdev_init() {
    struct sockaddr_rpmsg addr;
    addr.family = AF_RPMSG;
    addr.vproc_id = DP_CR5_MPC;
    addr.addr = AUDIO_MONITOR_EPT;
    serverfd = socket(PF_RPMSG, SOCK_SEQPACKET, 0);
    if (connect(serverfd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        serverfd = -1;
        ALOGE("%d, error: %s.\n", __LINE__, strerror(errno));
    }
}

void dspdev_init_locked() {
    std::lock_guard<decltype(ipcc_mutex)> lg(ipcc_mutex);
    if (serverfd <= 0) {
        dspdev_init();
    }
}

void dspdev_close() {
    std::lock_guard<decltype(ipcc_mutex)> lg(ipcc_mutex);
    if (serverfd > 0) {
        close(serverfd);
        serverfd = -1;
    }
}

int32_t ipcc_ctl__NoReply(void *data, uint32_t len) {
    int32_t ret = -1;
    dspdev_init_locked();
    std::lock_guard<decltype(ipcc_mutex)> lg(ipcc_mutex);
    if (data && serverfd > 0) {
        ret = sendto(serverfd, data, len, 0, nullptr, 0);
        if (ret <= 0) {
            ALOGE("%d, error: %s.\n", __LINE__, strerror(errno));
        }
    }
    return ret > 0 ? 0 : ret;
}

int32_t ipcc_ctl__Reply(void *data, uint32_t len, void *reply, uint32_t replen) {
    int32_t ret = -1;
    //dspdev_init_locked();
    std::lock_guard<decltype(ipcc_mutex)> lg(ipcc_mutex);
    if (data && serverfd > 0) {
        ret = sendto(serverfd, data, len, 0, nullptr, 0);
        if (ret > 0) {
            ret = recvfrom(serverfd, reply, replen, 0, nullptr, 0);
            if (ret <= 0) {
                ALOGE("%d, error: %s.\n", __LINE__, strerror(errno));
            }
        } else {
            ALOGE("%d, error: %s.\n", __LINE__, strerror(errno));
        }
    }
    return ret > 0 ? 0 : ret;
}

int32_t dspdev_set_volume(AM_AUDIO_TYPE type, uint8_t param) {
	uint8_t cmd = OP_SETVOL;

    char s_vol_type[4];
    sprintf(s_vol_type, "%d", type);

    struct ipcc_data pack = IPCC_PACKED_PARAM(cmd, param);
    IPCC_PACKED_DATA(pack.data, s_vol_type);
    return ipcc_ctl__NoReply(&pack, sizeof(pack));
}
