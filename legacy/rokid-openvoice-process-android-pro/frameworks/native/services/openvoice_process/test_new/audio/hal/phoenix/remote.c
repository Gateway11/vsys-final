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

#include "audio_hw.h"
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

#include "audio_control_wrap.h"

static pthread_mutex_t g_ipcc_lock = PTHREAD_MUTEX_INITIALIZER;

#define IPCC_LEN_MAX 64

#define IPCC_PACKED_PARAM(cmd, param)                                          \
	{                                                                      \
		.cmd = cmd, .param = param                                     \
	}

#define IPCC_PACKED_PATH(data, path)                                           \
	{                                                                      \
		int32_t len = strlen(path);                                    \
		len = len > IPCC_LEN_MAX ? IPCC_LEN_MAX : len;                 \
		strncpy(data, path, len);                                 \
	}


int32_t audio_remote_handshake(struct alsa_audio_device *adev, uint32_t param);


int32_t ipcc_init(struct alsa_audio_device *adev)
{
	int ret;
	struct sockaddr_rpmsg addr;
	struct platform_data *platform = adev->platform;
	AALOGI("ipcc_init");
	platform->remote = socket(PF_RPMSG, SOCK_SEQPACKET, 0);
	if (platform->remote < 0)
	{
		AALOGE("ipcc init failed, errno: %d (%s)", platform->remote,
		       strerror(errno));
		return -1;
	}

	addr.family = AF_RPMSG;
#ifdef IPCC_RPC_RPROC_MP
	addr.vproc_id = DP_CR5_MPC;
#else
	addr.vproc_id = DP_CR5_SAF;
#endif
	addr.addr = AUDIO_SERVICE_EPT;

	ret = connect(platform->remote, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0)
	{
		AALOGE("ipcc init failed");
		platform->remote = -1;
		return -1;
	}
	AALOGI("ipcc init success");
	return 0;
}

int32_t ipcc_ctrl(struct alsa_audio_device *adev, void *data, uint32_t len)
{
	int32_t ret = 0;
	//struct ipcc_ret ipcc_return;
	struct platform_data *platform = adev->platform;
	//struct timeval start, end;
	if (data && platform->remote > 0)
	{
		pthread_mutex_lock(&g_ipcc_lock);
		//gettimeofday(&start, NULL);
		ret = write(platform->remote, data, len);
		pthread_mutex_unlock(&g_ipcc_lock);
	}
	else
	{
		ret = -1;
		AALOGE("fd or data is invalid fd %d", platform->remote);
		ipcc_init(adev);
	}
	return ret;
}


void *audio_ipcc_thread_loop(void *context)
{
	int32_t ret = 0;
	char data[160];
	struct ipcc_ret ipcc_return;
	struct ipcc_data ipcc_read;
	struct alsa_audio_device *adev = (struct alsa_audio_device *)context;
	struct platform_data *platform = adev->platform;
	ALOGD("audio_ipcc_thread_loop start");
	audio_remote_handshake(adev, 1);
	while(1)
	{
		if (platform->remote > 0)
		{
			ret = read(platform->remote, data, 160);
			if(ret <= 0)
			{
				AALOGE("Read From R5 Failed");
			}
			else if(ret == sizeof(struct ipcc_data))
			{
				memcpy(&ipcc_read, data, sizeof(struct ipcc_data));
				ALOGD("ipcc_read cmd %d", ipcc_read.cmd);
				if(ipcc_read.cmd == OP_CHIME_ON)
				{
					current_audio_bus_status.is_chime_on = 1;
#if 0
					for (int j = 0; j < 8; j++)
					{
						switch (j)
						{
							case BUS0:
							case BUS1:
							case BUS2:
							case BUS3:
							case BUS6:
								set_device_address_is_ducked(j, current_audio_bus_status.is_ducked[j]);
								break;
							default:
								break;
						}
					}
#endif
					request_audio_focus(HAL_AUDIO_USAGE_SAFETY, 0, HAL_AUDIO_FOCUS_GAIN_TRANSIENT);
				}
				else if(ipcc_read.cmd == OP_CHIME_OFF)
				{
					current_audio_bus_status.is_chime_on = 0;
#if 0
					for (int j = 0; j < 8; j++)
					{
						switch (j)
						{
							case BUS0:
							case BUS1:
							case BUS2:
							case BUS3:
							case BUS6:
								set_device_address_is_ducked(j, current_audio_bus_status.is_ducked[j]);
								break;
							default:
								break;
						}
					}
#endif
					abandon_audio_focus(HAL_AUDIO_USAGE_SAFETY, 0);
				}
				else
				{

				}
			}
			else if(ret == sizeof(struct ipcc_ret))
			{
				memcpy(&ipcc_return, data, sizeof(struct ipcc_ret));
				ALOGD("ipcc_read cmd %d, code %d", ipcc_return.cmd, ipcc_return.code);
			}
			else
			{
				AALOGE("Read Unknow Data Len");
			}
		}
		else
		{
			ipcc_init(adev);
		}
		//gettimeofday(&end, NULL);
		//float interval = 1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
		//AALOGI("ipcc_ctrl len %d ret %d return: %d, cost: %f ms", len,bret, ipcc_return.code, interval / 1000.0);
		//ret = ipcc_return.code;
	}
}


int32_t audio_remote_init(struct alsa_audio_device *adev)
{
	int32_t ret = 0;
	ret = ipcc_init(adev);
	if(ret == 0)
	{
	    ret = pthread_create(&adev->ipcc_thread, (const pthread_attr_t *)NULL, audio_ipcc_thread_loop, adev);
		if(ret < 0)
		{
			AALOGE("ipcc_thread create failed");
		}
	}

    return ret;
}


int32_t audio_remote_handshake(struct alsa_audio_device *adev, uint32_t param)
{
	int ret;
	int cmd = OP_HANDSHAKE;
	struct ipcc_data pack = IPCC_PACKED_PARAM(cmd, param);
	ret = ipcc_ctrl(adev, &pack, sizeof(struct ipcc_data));
	AALOGI("path : %s, vol: %d, ret: %d", pack.data, pack.param, ret);
	return ret;
}


int32_t audio_remote_start(struct alsa_audio_device *adev, AM_PATH path, uint32_t param)
{
	int ret;
	int cmd = OP_START;
	struct ipcc_data pack = IPCC_PACKED_PARAM(cmd, param);
	IPCC_PACKED_PATH(pack.data, path);
	ret = ipcc_ctrl(adev, &pack, sizeof(struct ipcc_data));
	AALOGI("path : %s, vol: %d, ret: %d", pack.data, pack.param, ret);
	return ret;
}

int32_t audio_remote_switch_path(struct alsa_audio_device *adev, AM_PATH src, AM_PATH dst, int32_t param)
{
	int ret;
	int cmd = OP_START;
	struct ipcc_data pack = IPCC_PACKED_PARAM(cmd, param);
	IPCC_PACKED_PATH(pack.data, src);
	IPCC_PACKED_PATH(pack.data1, dst);
	ret = ipcc_ctrl(adev, &pack, sizeof(struct ipcc_data));
	AALOGV("path : %s, vol: %d, ret: %d", pack.data, pack.param, ret);
	return ret;
}

int32_t audio_remote_stop(struct alsa_audio_device *adev, AM_PATH path)
{
	int32_t ret;
	int32_t cmd = OP_STOP;
	int32_t param = 0;
	struct ipcc_data pack = IPCC_PACKED_PARAM(cmd, param);
	IPCC_PACKED_PATH(pack.data, path);
	ret = ipcc_ctrl(adev, &pack, sizeof(struct ipcc_data));
	AALOGI("path : %s, vol: %d, ret: %d", pack.data, pack.param, ret);
	return 0;
}

int32_t audio_remote_set_volume(struct alsa_audio_device *adev, AM_PATH path, uint32_t param)
{
	int32_t ret;
	int32_t cmd = OP_SETVOL;
	struct ipcc_data pack = IPCC_PACKED_PARAM(cmd, param);
	IPCC_PACKED_PATH(pack.data, path);
	ret = ipcc_ctrl(adev, &pack, sizeof(struct ipcc_data));

	AALOGV("setvol path : %s, vol: %d, ret: %d", path, param, ret);
	return ret;
}
