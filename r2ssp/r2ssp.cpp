// r2ssp.cpp
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <list>

#define R2SSP_USING_BLIS

//#define _GNU_SOURCE
//#include <sched.h>
//#undef _GNU_SOURCE

#if defined(__arm__) || defined(__AARCH64EL__)
#ifdef __LP64__
#define CPU_SETSIZE 1024
#else
#define CPU_SETSIZE 32
#endif

#define __CPU_BITTYPE  unsigned long int  /* mandated by the kernel  */
#define __CPU_BITS     (8 * sizeof(__CPU_BITTYPE))
#define __CPU_ELT(x)   ((x) / __CPU_BITS)
#define __CPU_MASK(x)  ((__CPU_BITTYPE)1 << ((x) & (__CPU_BITS - 1)))

#if 0
#if defined(__arm__)
typedef struct {
	__CPU_BITTYPE __bits[ CPU_SETSIZE / __CPU_BITS];
} cpu_set_t;
#endif
#endif

#define CPU_ZERO_S(setsize, set)  __builtin_memset(set, 0, setsize)
#define CPU_SET_S(cpu, setsize, set) \
  do { \
    size_t __cpu = (cpu); \
    if (__cpu < 8 * (setsize)) \
      (set)->__bits[__CPU_ELT(__cpu)] |= __CPU_MASK(__cpu); \
  } while (0)
#define CPU_ZERO(set)          CPU_ZERO_S(sizeof(cpu_set_t), set)
#define CPU_SET(cpu, set)      CPU_SET_S(cpu, sizeof(cpu_set_t), set)

#include <sys/syscall.h>
#endif

#ifdef R2SSP_BUILD_JNI
#include <jni.h>
#define LOG_TAG "r2ssp"
#include <android/log.h>
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#else 
#define LOGE(...){ char info[1024]; sprintf(info,__VA_ARGS__); printf("%s\n", info);}
#define LOGD(...){ char info[1024]; sprintf(info,__VA_ARGS__); printf("%s\n", info);}
#endif

#include "r2ssp.h"
#include "bf/r2_beamformer.h"
#include "fe/PLP.h"
#include "fe/feintf.h"
#if defined(__arm__) || defined(__AARCH64EL__)
#include "blis/blis.h"
#endif

#include "webrtc/common_audio/signal_processing/include/signal_processing_library.h"

//#define ZTSSP_USE_WEBRTC_AECM
//#define ZTSSP_USE_WEBRTC_AEC
#define ZTSSP_USE_SPEEX_AEC

#ifdef ZTSSP_USE_WEBRTC_AECM
#include "webrtc/modules/audio_processing/aecm/include/echo_control_mobile.h"
#else
#ifdef ZTSSP_USE_WEBRTC_AEC
#include "webrtc/modules/audio_processing/aec/include/echo_cancellation.h"
#else
#ifdef ZTSSP_USE_SPEEX_AEC
#include "speexdsp/config.h"
#include "speexdsp/libspeexdsp/arch.h"
#include "speex/speex_echo.h"
#include "speex/speex_preprocess.h"
#endif
#endif
#endif

#include "webrtc/modules/audio_processing/ns/include/noise_suppression.h"
#include "webrtc/modules/audio_processing/beamformer/nonlinear_beamformer.h"
#include "webrtc/modules/audio_processing/beamformer/beam_localizer.h"
#include "webrtc/common_audio/resampler/include/push_resampler.h"
#include "webrtc/modules/audio_processing/beamformer/beam_filter.h"
#include "webrtc/modules/audio_processing/beamformer/beam_adapter.h"

#include "webrtc/modules/audio_processing/agc/agc.h"
#include "webrtc/modules/audio_processing/agc/legacy/gain_control.h"

#if defined(__arm__) || defined(__AARCH64EL__)
#ifdef BF_USE_NE10_FFT
#include "NE10.h"
#endif
#endif

using namespace webrtc;

#ifdef __cplusplus
extern "C" {
#endif

static const char *g_version = "\nr2ssp_version_20151217_204502\n";

#ifdef R2SSP_BUILD_JNI
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	//LOGD("JNI_OnLoad begin.\n");

	JNIEnv* env = NULL;

	jint result = -1;
	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		LOGE("ERROR: GetEnv failed\n");
		goto bail;
	}

	assert(env != NULL);

	// success -- return valid version number
	result = JNI_VERSION_1_4;

	srand(time(NULL));

	WebRtcSpl_Init();
	FFTWrapper::Init();
#ifdef R2SSP_USING_BLIS
	bli_init();
#endif
#ifdef BF_USE_NE10_FFT
	ne10_init();
#endif

	//LOGD("JNI_OnLoad ok.\n");

	bail: return result;
}
#endif

int r2ssp_ssp_init() {
	WebRtcSpl_Init();
	FFTWrapper::Init();
#if defined(__arm__) || defined(__AARCH64EL__)
#ifdef R2SSP_USING_BLIS
	bli_init();
#endif
#ifdef BF_USE_NE10_FFT
	ne10_init();
#endif
#endif

	return 0;
}

int r2ssp_ssp_exit() {
	return 0;
}

#define ZTSSP_SPEEX_AEC_TAIL 1024

#ifdef ZTSSP_USE_SPEEX_AEC
typedef struct _r2ssp_aec_handle {
	SpeexEchoState *st;
	SpeexPreprocessState **dens;
	int speaker;
	int channel;
	float *mic_buf;
	float *ref_buf;
	float *out_buf;

	float prev_scale; // auto scale
    int zero_frame_count; // to reset aec
    int voice_frame_count; // to reset aec

	// multi-thread
	int thread_num;
	pthread_t *threads;
	sem_t *start_sems;
	sem_t *stop_sems;
	sem_t start_sems_;
	sem_t stop_sems_;
	pthread_mutex_t task_mutex;
	std::list<int> tasks;
	int thread_exit_flag;
	int thread_error_flag;
	SpeexEchoState **echo_states;
	// thread affinity
	int *thread_affinities;
	int thread_affi_num;
} r2ssp_aec_handle;
#endif

r2ssp_handle r2ssp_aec_create(int mode) {
#ifdef ZTSSP_USE_SPEEX_AEC
	r2ssp_aec_handle *pAec = new r2ssp_aec_handle();
	pAec->st = 0;
	pAec->dens = 0;
	pAec->speaker = 0;
	pAec->channel = 0;
	pAec->mic_buf = 0;
	pAec->ref_buf = 0;
	pAec->out_buf = 0;
	pAec->prev_scale = 1.0;
    pAec->zero_frame_count = 0;
    pAec->voice_frame_count = 0;
	pAec->thread_num = 0;
	pAec->threads = 0;
	pAec->start_sems = 0;
	pAec->stop_sems = 0;
	pAec->thread_error_flag = 0;
	pAec->thread_exit_flag = 0;
	pAec->echo_states = 0;

	pAec->thread_affinities = new int[2];
	pAec->thread_affi_num = 2;
	pAec->thread_affinities[0] = 1;
	pAec->thread_affinities[1] = 3;

	return (r2ssp_handle) pAec;
#endif

#ifdef ZTSSP_USE_WEBRTC_AECM
	void *pAec = 0;
	if (WebRtcAecm_Create(&pAec) == 0)
	return (r2ssp_handle) pAec;
	else
	return 0;
#endif
}

int r2ssp_aec_free(r2ssp_handle hAec) {
#ifdef ZTSSP_USE_SPEEX_AEC
	r2ssp_aec_handle *pAec = (r2ssp_aec_handle*) hAec;
	if (!pAec)
		return -1;
	if (pAec->st)
		speex_echo_state_destroy(pAec->st);
	if (pAec->dens) {
		for (int i = 0; i < pAec->channel; i++)
			speex_preprocess_state_destroy(pAec->dens[i]);
		delete[] pAec->dens;
	}
	if (pAec->mic_buf)
		delete[] pAec->mic_buf;
	if (pAec->ref_buf)
		delete[] pAec->ref_buf;
	if (pAec->out_buf)
		delete[] pAec->out_buf;

	// multi-thread
	if (pAec->thread_num > 0) {
		pAec->thread_exit_flag = 1;
		for (int i = 0; i < pAec->thread_num; i++) {
			sem_post(pAec->start_sems);
		}
		for (int i = 0; i < pAec->thread_num; i++) {
			pthread_join(pAec->threads[i], 0);
		}
		for (int i = 0; i < pAec->channel; i++) {
			speex_echo_state_destroy(pAec->echo_states[i]);
		}
#if defined(__APPLE__) && defined(__MACH__)
		sem_close(pAec->start_sems);
		sem_close(pAec->stop_sems);
		sem_unlink("sem_r2ssp_aec_start");
		sem_unlink("sem_r2ssp_aec_stop");
#else
		sem_destroy(pAec->start_sems);
		sem_destroy(pAec->stop_sems);
#endif
		pthread_mutex_destroy(&pAec->task_mutex);
		delete[] pAec->threads;
		delete[] pAec->echo_states;
	}

	if (pAec->thread_affinities) {
		delete[] pAec->thread_affinities;
	}

	delete pAec;
	return 0;
#endif

#ifdef ZTSSP_USE_WEBRTC_AECM
	void *pAec = (void*)hAec;
	int nRet = WebRtcAecm_Free(pAec);
	if (nRet != 0)
	nRet = WebRtcAecm_get_error_code(pAec);
	return nRet;
#endif
}

int r2ssp_aec_set_thread_affinities(r2ssp_handle hAec, int *thread_affinities, int thread_num) {
	if (thread_num <= 0 || !thread_affinities || thread_num > 256)
		return -1;
	for (int i=0; i<thread_num; i++) {
		if (thread_affinities[i] < 0 || thread_affinities[i] >= 256) {
			return -2;
		}
	}

#ifdef ZTSSP_USE_SPEEX_AEC
	r2ssp_aec_handle *pAec = (r2ssp_aec_handle*) hAec;
	if (!pAec)
		return -1;
	if (pAec->thread_affinities) {
		delete[] pAec->thread_affinities;
	}
	pAec->thread_affinities = new int[thread_num];
	pAec->thread_affi_num = thread_num;
	memcpy(pAec->thread_affinities, thread_affinities, thread_num * sizeof(int));
#endif

	return 0;
}

#if defined(__arm__) || defined(__AARCH64EL__)
static void setCurrentThreadAffinityMask(cpu_set_t mask) {
	int err, syscallres;
#if defined SYS_gettid
  pid_t pid = syscall(SYS_gettid);
#else
  pid_t pid = gettid();
#endif
	syscallres = syscall(__NR_sched_setaffinity, pid, sizeof(mask), &mask);
	if (syscallres) {
		err = errno;
		//ALOGI("set affinity failed");
	} else {
		//ALOGI("set affinity done");
	}
}
#endif

static void *r2ssp_aec_thread(void *arg) {
	r2ssp_aec_handle *pAec = (r2ssp_aec_handle*) arg;

#if defined(__arm__) || defined(__AARCH64EL__)
	int thrIdx = -1;
	for (int i = 0; i < pAec->thread_num; i++) {
		if (pthread_equal(pthread_self(), pAec->threads[i])) {
			thrIdx = i;
			break;
		}
	}
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(pAec->thread_affinities[thrIdx], &mask);
	setCurrentThreadAffinityMask(mask);
#endif

	while (1) {
		sem_wait(pAec->start_sems);
		//LOGD("aecthread waited");
		if (pAec->thread_exit_flag)
			break;

		int taskId = 0;
		pthread_mutex_lock(&pAec->task_mutex);
		assert(pAec->tasks.size());
		if (!pAec->tasks.size()) {
#if defined(__arm__) || defined(__AARCH64EL__)
			LOGE("r2ssp_aec_error, no tasks in task queue");
#endif
		}
		taskId = *pAec->tasks.begin();
		//LOGD("aecthread gettask: %d", taskId);
		pAec->tasks.pop_front();
		pthread_mutex_unlock(&pAec->task_mutex);
		if (taskId < 0 || taskId >= pAec->channel) {
#if defined(__arm__) || defined(__AARCH64EL__)
			LOGE("r2ssp_aec_error, wrong task id in task queue: %d", taskId);
#endif
		}

		speex_echo_cancellation(pAec->echo_states[taskId],
				pAec->mic_buf + taskId * ZTSSP_AEC_FRAME_SAMPLE, pAec->ref_buf,
				pAec->out_buf + taskId * ZTSSP_AEC_FRAME_SAMPLE);
		speex_preprocess_run(pAec->dens[taskId],
				pAec->out_buf + taskId * ZTSSP_AEC_FRAME_SAMPLE);

		//LOGD("aecthread post");
		sem_post(pAec->stop_sems);
	}

	return 0;
}

int r2ssp_aec_init(r2ssp_handle hAec, int nSampleRate, int nChannelNum,
		int nSpeakerNum) {
#ifdef ZTSSP_USE_SPEEX_AEC
	r2ssp_aec_handle *pAec = (r2ssp_aec_handle*) hAec;
	if (!pAec)
		return -1;
	pAec->speaker = nSpeakerNum;
	pAec->channel = nChannelNum;
	pAec->mic_buf = new float[ZTSSP_AEC_FRAME_SAMPLE * nChannelNum];
	pAec->ref_buf = new float[ZTSSP_AEC_FRAME_SAMPLE * nSpeakerNum];
	pAec->out_buf = new float[ZTSSP_AEC_FRAME_SAMPLE * nChannelNum];

	pAec->dens = new SpeexPreprocessState*[nChannelNum];
	for (int i = 0; i < nChannelNum; i++) {
		pAec->dens[i] = speex_preprocess_state_init(ZTSSP_AEC_FRAME_SAMPLE,
				nSampleRate);
	}

	int nThreadNum = pAec->thread_affi_num;
	if (nThreadNum > 0 && nChannelNum > 1) {
		if (nThreadNum > 256)
			nThreadNum = 256;
		pAec->threads = new pthread_t[nThreadNum];
		memset(pAec->threads, 0, sizeof(pthread_t) * nThreadNum);
#if defined(__APPLE__) && defined(__MACH__)
		sem_unlink("sem_r2ssp_aec_start");
		sem_unlink("sem_r2ssp_aec_stop");
		pAec->start_sems = sem_open("sem_r2ssp_aec_start", O_CREAT|O_EXCL, 0644, 0);
		pAec->stop_sems = sem_open("sem_r2ssp_aec_stop", O_CREAT|O_EXCL, 0644, 0);
#else
		sem_init(&pAec->start_sems_, 0, 0);
		sem_init(&pAec->stop_sems_, 0, 0);
		pAec->start_sems = &pAec->start_sems_;
		pAec->stop_sems = &pAec->stop_sems_;
#endif
		pthread_mutex_init(&pAec->task_mutex, 0);
		pAec->thread_exit_flag = 0;
		pAec->thread_error_flag = 0;
		pAec->thread_num = nThreadNum;
		for (int i = 0; i < nThreadNum; i++) {
			pthread_create(&pAec->threads[i], 0, r2ssp_aec_thread, pAec);
		}
		pAec->echo_states = new SpeexEchoState*[nChannelNum];
		for (int i = 0; i < nChannelNum; i++) {
			pAec->echo_states[i] = speex_echo_state_init_mc(
			ZTSSP_AEC_FRAME_SAMPLE,
			ZTSSP_SPEEX_AEC_TAIL, 1, nSpeakerNum);
			speex_echo_ctl(pAec->echo_states[i], SPEEX_ECHO_SET_SAMPLING_RATE,
					&nSampleRate);
			speex_preprocess_ctl(pAec->dens[i], SPEEX_PREPROCESS_SET_ECHO_STATE,
					pAec->echo_states[i]);
		}
	} else {
		pAec->st = speex_echo_state_init_mc(ZTSSP_AEC_FRAME_SAMPLE,
		ZTSSP_SPEEX_AEC_TAIL, nChannelNum, nSpeakerNum);
		speex_echo_ctl(pAec->st, SPEEX_ECHO_SET_SAMPLING_RATE, &nSampleRate);
		for (int i = 0; i < nChannelNum; i++) {
			speex_preprocess_ctl(pAec->dens[i], SPEEX_PREPROCESS_SET_ECHO_STATE,
					pAec->st);
		}
		//int agc=1;
		//speex_preprocess_ctl(pAec->den, SPEEX_PREPROCESS_SET_AGC, &agc);
		//float agcLevel = 24000;
		//speex_preprocess_ctl(pAec->den, SPEEX_PREPROCESS_SET_AGC_LEVEL, &agcLevel);
	}

	return 0;
#endif

#ifdef ZTSSP_USE_WEBRTC_AECM
	void *pAec = (void*)hAec;
	int nRet = 0;
	nRet = WebRtcAecm_Init(pAec, nSampleRate);
	if (nRet == 0) {
		AecmConfig config;
		config.cngMode = AecmFalse;
		config.echoMode = 4;
		nRet = WebRtcAecm_set_config(pAec, config);
	}
	if (nRet != 0)
	nRet = WebRtcAecm_get_error_code(pAec);
	return nRet;
#endif
}

int r2ssp_aec_buffer_farend(r2ssp_handle hAec, const float *refSamples,
		int length) {
#ifdef ZTSSP_USE_SPEEX_AEC
	r2ssp_aec_handle *pAec = (r2ssp_aec_handle*) hAec;
	if (!pAec)
		return -1;
	if (length < ZTSSP_AEC_FRAME_SAMPLE * pAec->speaker)
		return -2;
	for (int i = 0; i < ZTSSP_AEC_FRAME_SAMPLE; i++) {
		for (int j = 0; j < pAec->speaker; j++)
			pAec->ref_buf[i * pAec->speaker + j] = refSamples[j
					* ZTSSP_AEC_FRAME_SAMPLE + i];
	}
	return 0;
#endif

#ifdef ZTSSP_USE_WEBRTC_AECM
	void *pAec = (void*)hAec;
	int nRet = WebRtcAecm_BufferFarend(pAec, refSamples, length);
	if (nRet != 0)
	nRet = WebRtcAecm_get_error_code(pAec);
	return nRet;
#endif
}

int r2ssp_aec_process(r2ssp_handle hAec, const float *mixSamples, int length,
		float *outSamples, int delayMs) {
#ifdef ZTSSP_USE_SPEEX_AEC
	r2ssp_aec_handle *pAec = (r2ssp_aec_handle*) hAec;
	if (!pAec)
		return -1;
	if (length < ZTSSP_AEC_FRAME_SAMPLE * pAec->channel)
		return -2;

	// check all-zero of ref signals
	int bHasNoZero = 0;
	for (int i = 0; i < ZTSSP_AEC_FRAME_SAMPLE * pAec->speaker; i++) {
		if (((int*) (pAec->ref_buf))[i] & 0x7fffffff) {
			bHasNoZero = 1;
			break;
		}
	}
	if (!bHasNoZero) {
        pAec->zero_frame_count ++;
        pAec->voice_frame_count = 0;
        /*if (pAec->zero_frame_count == 10) {
            for (int i = 0; i < pAec->channel; i++) {
                speex_echo_state_reset(pAec->echo_states[i]);
            }
        }*/
		memcpy(outSamples, mixSamples,
		ZTSSP_AEC_FRAME_SAMPLE * pAec->channel * sizeof(float));
		return 1;
	} else {
        pAec->voice_frame_count ++;
        pAec->zero_frame_count = 0;
        // reset aec at the 30th seconds in a song
        // do not reset again because mdf adaption impovement (gaopeng 2015.12.17 on Z10 train)
        /*if (pAec->voice_frame_count == 30*1000*16/ZTSSP_AEC_FRAME_SAMPLE) { // 30 seconds
            for (int i = 0; i < pAec->channel; i++) {
                speex_echo_state_reset(pAec->echo_states[i]);
            }
            pAec->prev_scale = 1.0f;
        }*/
    }

	// rearrange sample sequence
	if (pAec->thread_num <= 0)
		for (int i = 0; i < ZTSSP_AEC_FRAME_SAMPLE; i++) {
			for (int j = 0; j < pAec->channel; j++)
				pAec->mic_buf[i * pAec->channel + j] = mixSamples[j
						* ZTSSP_AEC_FRAME_SAMPLE + i];
		}
	else
		memcpy(pAec->mic_buf, mixSamples,
		ZTSSP_AEC_FRAME_SAMPLE * pAec->channel * sizeof(float));

	// dynamic scale adjust
	float fAvgRef = 0;
	for (int i = 0; i < ZTSSP_AEC_FRAME_SAMPLE * pAec->speaker; i++) {
		fAvgRef += fabs(pAec->ref_buf[i]);
	}
	fAvgRef /= ZTSSP_AEC_FRAME_SAMPLE * pAec->speaker;
	float fAvgMic = 0;
	for (int i = 0; i < ZTSSP_AEC_FRAME_SAMPLE * pAec->channel; i++) {
		fAvgMic += fabs(pAec->mic_buf[i]);
	}
	fAvgMic /= ZTSSP_AEC_FRAME_SAMPLE * pAec->channel;
	int nAvgAmp = 1024 * 7;
	float fRefScale = nAvgAmp / fAvgRef;
	float fMicScale = nAvgAmp / fAvgMic;
	float fScale = fRefScale < fMicScale ? fRefScale : fMicScale;
	fScale = fScale > 8 ? 8 : fScale;
	fScale = fScale < 1 ? 1 : fScale;
	// smoothing
	float fSmoothFactor = 0.1;
	fScale = fSmoothFactor * fScale + (1 - fSmoothFactor) * pAec->prev_scale;
	pAec->prev_scale = fScale;
	//fScale = 1.0f; // disable dynamic-scale

	for (int i = 0; i < ZTSSP_AEC_FRAME_SAMPLE * pAec->speaker; i++) {
		pAec->ref_buf[i] *= fScale;
	}
	for (int i = 0; i < ZTSSP_AEC_FRAME_SAMPLE * pAec->channel; i++) {
		pAec->mic_buf[i] *= fScale;
	}

	if (pAec->thread_num > 0) {
		if (!pAec->thread_error_flag) {
			for (int i = 0; i < pAec->channel; i++) {
				pAec->tasks.push_back(i);
			}
			//LOGD("aec post tasks");
			for (int i = 0; i < pAec->channel; i++) {
				sem_post(pAec->start_sems);
			}
			for (int i = 0; i < pAec->channel; i++) {
				sem_wait(pAec->stop_sems);
			}
			//LOGD("aec post tasks finished");
			memcpy(outSamples, pAec->out_buf,
			ZTSSP_AEC_FRAME_SAMPLE * pAec->channel * sizeof(float));
		}
	} else {
		speex_echo_cancellation(pAec->st, pAec->mic_buf, pAec->ref_buf,
				pAec->out_buf);

		for (int i = 0; i < pAec->channel; i++) {
			for (int j = 0; j < ZTSSP_AEC_FRAME_SAMPLE; j++)
				outSamples[i * ZTSSP_AEC_FRAME_SAMPLE + j] = pAec->out_buf[j
						* pAec->channel + i];
			speex_preprocess_run(pAec->dens[i],
					outSamples + i * ZTSSP_AEC_FRAME_SAMPLE);
		}
	}

	// dynamic scale adjust
	for (int i = 0; i < ZTSSP_AEC_FRAME_SAMPLE * pAec->channel; i++) {
		outSamples[i] /= fScale;
	}

	if (pAec->thread_num > 0 && pAec->thread_error_flag)
		return -1;
	else
		return 0;
#endif

#ifdef ZTSSP_USE_WEBRTC_AECM
	void *pAec = (void*)hAec;
	int nRet = WebRtcAecm_Process(pAec, mixSamples, 0, outSamples, length, delayMs);
	if (nRet != 0)
	nRet = WebRtcAecm_get_error_code(pAec);
	return nRet;
#endif
}

int r2ssp_aec_reset(r2ssp_handle hAec) {
#ifdef ZTSSP_USE_SPEEX_AEC
	r2ssp_aec_handle *pAec = (r2ssp_aec_handle*) hAec;
	if (!pAec)
		return -1;
	if (pAec->st)
		speex_echo_state_reset(pAec->st);

	if (pAec->thread_num > 0) {
		for (int i = 0; i < pAec->channel; i++)
			speex_echo_state_reset(pAec->echo_states[i]);
	}
#endif

	return 0;
}

#ifdef R2SSP_BUILD_JNI

JNIEXPORT jlong JNICALL Java_com_rokid_ssp_R2SspWrapper_aecCreate(JNIEnv* env,
		jobject thiz) {
	void *pAec = 0;
	/*
	 #ifdef ZTSSP_USE_AECM
	 if (WebRtcAecm_Create(&pAec) == 0)
	 #else
	 if (WebRtcAec_Create(&pAec) == 0)
	 #endif
	 return (jlong) pAec;
	 else
	 return 0;
	 */
	return r2ssp_aec_create(0);
}

JNIEXPORT jint JNICALL Java_com_rokid_ssp_R2SspWrapper_aecFree(JNIEnv* env,
		jobject thiz, jlong hAec) {
	/*
	 void *pAec = (void*) hAec;
	 if (pAec)
	 #ifdef ZTSSP_USE_AECM
	 return WebRtcAecm_Free(pAec);
	 #else
	 return WebRtcAec_Free(pAec);
	 #endif
	 else
	 return -1;
	 */
	return r2ssp_aec_free(hAec);
}

JNIEXPORT jint JNICALL Java_com_rokid_ssp_R2SspWrapper_aecInit(JNIEnv* env,
		jobject thiz, jlong hAec, jint nSampleRate, jint nChannelNum,
		jint nSpeakerNum, jint nThreadNum) {
	/*
	 void *pAec = (void*) hAec;
	 if (!pAec)
	 return -1;

	 int nRet = 0;
	 #ifdef ZTSSP_USE_AECM
	 nRet = WebRtcAecm_Init(pAec, nSampleRate);
	 if (nRet == 0) {
	 AecmConfig config;
	 config.cngMode = AecmFalse;
	 config.echoMode = 2;
	 nRet = WebRtcAecm_set_config(pAec, config);
	 }
	 #else
	 nRet = WebRtcAec_Init(pAec, nSampleRate, nSampleRate);
	 #endif

	 if (nRet != 0)
	 #ifdef ZTSSP_USE_AECM
	 nRet = WebRtcAecm_get_error_code(pAec);
	 #else
	 nRet = WebRtcAec_get_error_code(pAec);
	 #endif

	 return nRet;
	 */
	return r2ssp_aec_init(hAec, nSampleRate, nChannelNum, nSpeakerNum);
}

JNIEXPORT jint JNICALL Java_com_rokid_ssp_R2SspWrapper_aecBufferFarend(
		JNIEnv* env, jobject thiz, jlong hAec, jfloatArray refSamples,
		jint offset, jint length) {
	void *pAec = (void*) hAec;
	if (!pAec || offset < 0 || length < 0)
		return -1;
	if (!refSamples || length == 0)
		return 0;

	jfloat *pRefSamples = 0;
	jsize nSampleNum = 0;
	pRefSamples = env->GetFloatArrayElements(refSamples, 0);
	nSampleNum = env->GetArrayLength(refSamples);
	if (nSampleNum < offset + length)
		return -1;

	int nRet = 0;
	/*
	 #ifdef ZTSSP_USE_AECM
	 int nRet = WebRtcAecm_BufferFarend(pAec, pRefSamples + offset, length);
	 #else
	 float *pFloatSamples = new float[length];
	 for (int i = 0; i < length; i++)
	 pFloatSamples[i] = (float) pRefSamples[offset + i] / 32768;
	 int nRet = WebRtcAec_BufferFarend(pAec, pFloatSamples, length);
	 delete[] pFloatSamples;
	 #endif
	 */

	nRet = r2ssp_aec_buffer_farend(hAec, pRefSamples + offset, length);
	if (pRefSamples != 0)
		env->ReleaseFloatArrayElements(refSamples, pRefSamples, JNI_ABORT);
	/*
	 if (nRet != 0)
	 #ifdef ZTSSP_USE_AECM
	 nRet = WebRtcAecm_get_error_code(pAec);
	 #else
	 nRet = WebRtcAec_get_error_code(pAec);
	 #endif
	 */
	return nRet;
}

JNIEXPORT jint JNICALL Java_com_rokid_ssp_R2SspWrapper_aecProcess(JNIEnv* env,
		jobject thiz, jlong hAec, jfloatArray mixSamples, jint offset,
		jint length, jfloatArray outSamples, jint offset2, jint delayMs) {
	void *pAec = (void*) hAec;
	if (!pAec || offset < 0 || length <= 0 || offset2 < 0 || !mixSamples
			|| !outSamples)
		return -1;

	jfloat *pMixSamples = 0;
	jsize nSampleNum = 0;
	pMixSamples = env->GetFloatArrayElements(mixSamples, 0);
	nSampleNum = env->GetArrayLength(mixSamples);
	if (nSampleNum < offset + length)
		return -1;

	jfloat *pOutSamples = 0;
	jsize nOutNum = 0;
	pOutSamples = env->GetFloatArrayElements(outSamples, 0);
	nOutNum = env->GetArrayLength(outSamples);
	if (nOutNum < offset2 + length)
		return -1;

	int nRet = 0;
	/*
	 #ifdef ZTSSP_USE_AECM
	 nRet = WebRtcAecm_Process(pAec, pMixSamples + offset, 0,
	 pOutSamples + offset2, length, delayMs);
	 #else
	 float *pFloatSamples = new float[length];
	 for (int i = 0; i < length; i++)
	 pFloatSamples[i] = (float) pMixSamples[offset + i] / 32768;
	 float *pFloatOutSamples = new float[length];
	 int nRet = WebRtcAec_Process(pAec, pFloatSamples, 0, pFloatOutSamples, 0,
	 length, delayMs, 0);
	 for (int i = 0; i < length; i++)
	 pOutSamples[offset2 + i] = pFloatOutSamples[i] * 32768;
	 delete[] pFloatSamples;
	 delete[] pFloatOutSamples;
	 #endif
	 */

	nRet = r2ssp_aec_process(hAec, pMixSamples + offset, length,
			pOutSamples + offset2, delayMs);
	/*
	 Primitive Array Release Modes
	 mode	actions
	 0	copy back the content and free the elems buffer
	 JNI_COMMIT	copy back the content but do not free the elems buffer
	 JNI_ABORT	free the buffer without copying back the possible changes
	 */
	if (pMixSamples != 0)
		env->ReleaseFloatArrayElements(mixSamples, pMixSamples, JNI_ABORT);
	if (pOutSamples != 0)
		env->ReleaseFloatArrayElements(outSamples, pOutSamples, 0);
	/*
	 #ifdef ZTSSP_USE_AECM
	 nRet = WebRtcAecm_get_error_code(pAec);
	 #else
	 nRet = WebRtcAec_get_error_code(pAec);
	 #endif
	 */
	return nRet;
}
#endif

/*
 r2ssp_handle r2ssp_srp_create() {
 return (r2ssp_handle) new srp_phat();
 }

 JNIEXPORT jlong JNICALL Java_com_rokid_ssp_R2SspWrapper_srpCreate(
 JNIEnv* env, jobject thiz) {
 return (jlong) r2ssp_srp_create();
 }

 int r2ssp_srp_free(r2ssp_handle hSrp) {
 srp_phat *pSrp = (srp_phat*) hSrp;
 if (pSrp)
 delete pSrp;
 return 0;
 }

 JNIEXPORT jint JNICALL Java_com_rokid_ssp_R2SspWrapper_srpFree(JNIEnv* env,
 jobject thiz, jlong hSrp) {
 return r2ssp_srp_free(hSrp);
 }

 int r2ssp_srp_setparam(r2ssp_handle hSrp, int sft_ms, int frm_ms, int sam_khz,
 float temp_c, float micd_m) {
 srp_phat *pSrp = (srp_phat*) hSrp;
 if (!pSrp)
 return -1;
 if (pSrp->setpram(sft_ms, frm_ms, sam_khz, temp_c, micd_m))
 return 0;
 else
 return -2;
 }

 JNIEXPORT jint JNICALL Java_com_rokid_ssp_R2SspWrapper_srpSetParam(
 JNIEnv* env, jobject thiz, jlong hSrp, jint sft_ms, jint frm_ms,
 jint sam_khz, jfloat temp_c, jfloat micd_m) {
 return r2ssp_srp_setparam(hSrp, sft_ms, frm_ms, sam_khz, temp_c, micd_m);
 }

 float r2ssp_srp_process(r2ssp_handle hSrp, short *pSpeech1, short *pSpeech2,
 int nSampleNum) {
 srp_phat *pSrp = (srp_phat*) hSrp;
 if (!pSrp || !pSpeech1 || !pSpeech2 || nSampleNum <= 0)
 return INT_MIN;
 return pSrp->gccPhat(pSpeech1, pSpeech2, nSampleNum);
 }

 JNIEXPORT jfloat JNICALL Java_com_rokid_ssp_R2SspWrapper_srpProcess(
 JNIEnv* env, jobject thiz, jlong hSrp, jshortArray speech1,
 jshortArray speech2) {
 if (!hSrp || !speech1 || !speech2)
 return INT_MIN;

 jshort *pSpeech1 = 0;
 jsize nSpeech1Num = 0;
 pSpeech1 = env->GetShortArrayElements(speech1, 0);
 nSpeech1Num = env->GetArrayLength(speech1);
 jshort *pSpeech2 = 0;
 jsize nSpeech2Num = 0;
 pSpeech2 = env->GetShortArrayElements(speech2, 0);
 nSpeech2Num = env->GetArrayLength(speech2);
 if (!pSpeech1 || !pSpeech2 || nSpeech1Num <= 0 || nSpeech2Num <= 0
 || nSpeech1Num != nSpeech2Num)
 return INT_MIN;

 float fRet = r2ssp_srp_process(hSrp, pSpeech1, pSpeech2, nSpeech1Num);

 if (pSpeech1 != 0)
 env->ReleaseShortArrayElements(speech1, pSpeech1, JNI_ABORT);
 if (pSpeech2 != 0)
 env->ReleaseShortArrayElements(speech2, pSpeech2, JNI_ABORT);
 return fRet;
 }
 */

typedef struct _r2ssp_mix_handle {
	int mode;
	int shift;
	short frame_buf[160];
} r2ssp_mix_handle;

r2ssp_handle r2ssp_mix_create(int mode) {
	r2ssp_mix_handle *pAec = new r2ssp_mix_handle();
	pAec->mode = mode;
	return (r2ssp_handle) pAec;
}

int r2ssp_mix_free(r2ssp_handle hMix) {
	r2ssp_mix_handle *pMix = (r2ssp_mix_handle*) hMix;
	if (!pMix)
		return -1;
	delete pMix;
	return 0;
}

int r2ssp_mix_process(r2ssp_handle hMix, short *pSpeech1, short *pSpeech2,
		int nSampleNum, short *pOutSpeech) {
	r2ssp_mix_handle *pMix = (r2ssp_mix_handle*) hMix;
	if (!pMix || !pSpeech1 || !pSpeech2 || !pOutSpeech || nSampleNum < 160)
		return -1;

	for (int i = 0; i < nSampleNum; i++) {
		pOutSpeech[i] = (pSpeech1[i] + pSpeech2[i]) / 2;
	}

	return 0;
}

r2ssp_handle r2ssp_ns_create() {
	NsHandle *pHandle;
	if (WebRtcNs_Create(&pHandle))
		return 0;
	else
		return (r2ssp_handle) pHandle;
}

int r2ssp_ns_free(r2ssp_handle hNs) {
	NsHandle *pHandle = (NsHandle*) hNs;
	return WebRtcNs_Free(pHandle);
}

int r2ssp_ns_init(r2ssp_handle hNs, int nSampleRate) {
	NsHandle *pHandle = (NsHandle*) hNs;
	return WebRtcNs_Init(pHandle, nSampleRate);
}

int r2ssp_ns_set_mode(r2ssp_handle hNs, int nMode) {
	NsHandle *pHandle = (NsHandle*) hNs;
	return WebRtcNs_set_policy(pHandle, nMode);
}

int r2ssp_ns_process(r2ssp_handle hNs, const float *pFrame, float *pOutFrame) {
	NsHandle *pHandle = (NsHandle*) hNs;
	WebRtcNs_Analyze(pHandle, pFrame);
	WebRtcNs_Process(pHandle, &pFrame, 1, &pOutFrame);
	return 0;
}
/*
 JNIEXPORT jlong JNICALL Java_com_rokid_ssp_R2SspWrapper_nsCreate(JNIEnv* env,
 jobject thiz, jfloatArray mics) {
 r2ssp_handle hNs = r2ssp_ns_create();
 return hNs;
 }

 JNIEXPORT jint JNICALL Java_com_rokid_ssp_R2SspWrapper_nsFree(JNIEnv* env,
 jobject thiz, jlong hNs) {
 return r2ssp_ns_free(hNs);
 }

 JNIEXPORT jint JNICALL Java_com_rokid_ssp_R2SspWrapper_nsInit(JNIEnv* env,
 jobject thiz, jlong hNs, jint sampleRate) {
 return r2ssp_ns_init(hNs, sampleRate);
 }

 JNIEXPORT jint JNICALL Java_com_rokid_ssp_R2SspWrapper_nsSetMode(JNIEnv* env,
 jobject thiz, jlong hNs, jint mode) {
 return r2ssp_ns_set_mode(hNs, mode);
 }

 JNIEXPORT jint JNICALL Java_com_rokid_ssp_R2SspWrapper_nsProcess(JNIEnv* env,
 jobject thiz, jlong hNs, jfloatArray inFrames, jfloatArray outFrame) {
 if (!hNs || !inFrames || !outFrame)
 return -1;

 jfloat *pInFrames = 0;
 jsize nInFrameLength = 0;
 pInFrames = env->GetFloatArrayElements(inFrames, 0);
 nInFrameLength = env->GetArrayLength(inFrames);
 jfloat *pOutFrame = 0;
 jsize nOutFrameLength = 0;
 pOutFrame = env->GetFloatArrayElements(outFrame, 0);
 nOutFrameLength = env->GetArrayLength(outFrame);

 int nRet = r2ssp_ns_process(hNs, pInFrames, pOutFrame);

 if (pInFrames != 0)
 env->ReleaseFloatArrayElements(inFrames, pInFrames, JNI_ABORT);
 if (pOutFrame != 0)
 env->ReleaseFloatArrayElements(outFrame, pOutFrame, 0);
 return nRet;
 }
 */
r2ssp_handle r2ssp_bf_create(float *pMics, int nMicNum) {
	if (!pMics || nMicNum <= 1)
		return 0;

	std::vector<Point> pts;
	for (int i = 0; i < nMicNum; i++) {
		Point pt(pMics[i * 3], pMics[i * 3 + 1], pMics[i * 3 + 2]);
		pts.push_back(pt);
	}

	NonlinearBeamformer *pBf = new NonlinearBeamformer(pts);
	return (r2ssp_handle) pBf;
}

int r2ssp_bf_free(r2ssp_handle hBf) {
	NonlinearBeamformer *pBf = (NonlinearBeamformer*) hBf;
	if (!pBf)
		return -1;

	delete pBf;
	return 0;
}

int r2ssp_bf_set_mic_delays(r2ssp_handle hBf, float *pDelays, int nMicNum) {
    NonlinearBeamformer *pBf = (NonlinearBeamformer*) hBf;
    if (!pBf)
        return -1;

    return pBf->SetMicDelays(pDelays, nMicNum);
}

//static bf_handle hfd1, hfd2, hfd3, hfd4;
//static bf_handle hfd0, hfd5, hfd6, hfd7;

int r2ssp_bf_init(r2ssp_handle hBf, int nFrameSizeMs, int nSampleRate) {
	NonlinearBeamformer *pBf = (NonlinearBeamformer*) hBf;
	if (!pBf)
		return -1;

	pBf->Initialize(nFrameSizeMs, nSampleRate);
	/*hfd1 = r2_dl_create();
	 hfd2 = r2_dl_create();
	 hfd3 = r2_dl_create();
	 hfd4 = r2_dl_create();
	 hfd5 = r2_dl_create();
	 hfd6 = r2_dl_create();
	 hfd7 = r2_dl_create();
	 hfd0 = r2_dl_create();
	 r2_dl_init(hfd0, nFrameSizeMs*nSampleRate/1000, 0.5+0.1706667f);
	 r2_dl_init(hfd2, nFrameSizeMs*nSampleRate/1000, 0.5+0.1706667f);
	 r2_dl_init(hfd4, nFrameSizeMs*nSampleRate/1000, 0.5+0.1706667f);
	 r2_dl_init(hfd6, nFrameSizeMs*nSampleRate/1000, 0.5+0.1706667f);
	 r2_dl_init(hfd1, nFrameSizeMs*nSampleRate/1000, 0.5);
	 r2_dl_init(hfd3, nFrameSizeMs*nSampleRate/1000, 0.5);
	 r2_dl_init(hfd5, nFrameSizeMs*nSampleRate/1000, 0.5);
	 r2_dl_init(hfd7, nFrameSizeMs*nSampleRate/1000, 0.5);*/
	return 0;
}

int r2ssp_bf_steer(r2ssp_handle hBf, float targetAngle, float targetAngle2,
		float interfAngle, float interfAngle2) {
	NonlinearBeamformer *pBf = (NonlinearBeamformer*) hBf;
	if (!pBf)
		return -1;

	pBf->SteerToDirection(targetAngle, targetAngle2, interfAngle, interfAngle2);
	return 0;
}

int r2ssp_bf_process(r2ssp_handle hBf, const float *pInFrames, int nChunkSize,
		int nChannels, float *pOutFrame) {
	NonlinearBeamformer *pBf = (NonlinearBeamformer*) hBf;
	if (!pBf)
		return -1;
	if (!pInFrames || !pOutFrame || nChunkSize <= 2 || nChannels <= 1)
		return -1;

	int nFrameSize = nChunkSize / nChannels;
	ChannelBuffer<float> inFloatBuffer(nFrameSize, nChannels, 1);
	ChannelBuffer<float> outFloatBuffer(nFrameSize, 1);

	/*float *pvInFrames = (float*) (void*) pInFrames;
	 float outFrame[1024];
	 r2_dl_process(hfd0, pInFrames+nFrameSize*0, outFrame);
	 memcpy(pvInFrames+nFrameSize*0, outFrame, nFrameSize*sizeof(float));
	 r2_dl_process(hfd2, pInFrames+nFrameSize*2, outFrame);
	 memcpy(pvInFrames+nFrameSize*2, outFrame, nFrameSize*sizeof(float));
	 r2_dl_process(hfd4, pInFrames+nFrameSize*4, outFrame);
	 memcpy(pvInFrames+nFrameSize*4, outFrame, nFrameSize*sizeof(float));
	 r2_dl_process(hfd6, pInFrames+nFrameSize*6, outFrame);
	 memcpy(pvInFrames+nFrameSize*6, outFrame, nFrameSize*sizeof(float));
	 r2_dl_process(hfd1, pInFrames+nFrameSize*1, outFrame);
	 memcpy(pvInFrames+nFrameSize*1, outFrame, nFrameSize*sizeof(float));
	 r2_dl_process(hfd3, pInFrames+nFrameSize*3, outFrame);
	 memcpy(pvInFrames+nFrameSize*3, outFrame, nFrameSize*sizeof(float));
	 r2_dl_process(hfd5, pInFrames+nFrameSize*5, outFrame);
	 memcpy(pvInFrames+nFrameSize*5, outFrame, nFrameSize*sizeof(float));
	 r2_dl_process(hfd7, pInFrames+nFrameSize*7, outFrame);
	 memcpy(pvInFrames+nFrameSize*7, outFrame, nFrameSize*sizeof(float));*/

	inFloatBuffer.SetDataForTesting(pInFrames, nChunkSize);
	pBf->ProcessChunk(&inFloatBuffer, &outFloatBuffer);
	for (int i = 0; i < nFrameSize; i++)
		pOutFrame[i] = outFloatBuffer.channels(0)[0][i];

	return (pBf->is_target_present() ? 1 : 0);
}

r2ssp_handle r2ssp_bfsl_create(float *pMics, int nMicNum) {
	if (!pMics || nMicNum <= 1)
		return 0;

	std::vector<Point> pts;
	for (int i = 0; i < nMicNum; i++) {
		Point pt(pMics[i * 3], pMics[i * 3 + 1], pMics[i * 3 + 2]);
		pts.push_back(pt);
	}

	BeamLocalizer *pBfSl = new BeamLocalizer(pts);
	return (r2ssp_handle) pBfSl;
}

int r2ssp_bfsl_free(r2ssp_handle hBfSl) {
	BeamLocalizer *pBfSl = (BeamLocalizer*) hBfSl;
	if (!pBfSl)
		return -1;

	delete pBfSl;
	return 0;
}

int r2ssp_bfsl_init(r2ssp_handle hBfSl, int nFrameSizeMs, int nSampleRate) {
	BeamLocalizer *pBfSl = (BeamLocalizer*) hBfSl;
	if (!pBfSl)
		return -1;

	pBfSl->Initialize(nFrameSizeMs, nSampleRate);
	return 0;
}

int r2ssp_bfsl_reset(r2ssp_handle hBfSl) {
	BeamLocalizer *pBfSl = (BeamLocalizer*) hBfSl;
	if (!pBfSl)
		return -1;
	pBfSl->Reset();
	return 0;
}

int r2ssp_bfsl_process(r2ssp_handle hBfSl, const float *pInFrames,
		int nChunkSize, int nChannels) {
	BeamLocalizer *pBfSl = (BeamLocalizer*) hBfSl;
	if (!pBfSl)
		return -1;
	if (!pInFrames || nChunkSize <= 2 || nChannels <= 1)
		return -1;

	int nFrameSize = nChunkSize / nChannels;
	ChannelBuffer<float> inFloatBuffer(nFrameSize, nChannels, 1);
	ChannelBuffer<float> outFloatBuffer(nFrameSize, 1);

	inFloatBuffer.SetDataForTesting(pInFrames, nChunkSize);
	pBfSl->ProcessChunk(&inFloatBuffer, &outFloatBuffer);

	return 0;
}

int r2ssp_bfsl_get_candidate_num(r2ssp_handle hBfSl) {
	BeamLocalizer *pBfSl = (BeamLocalizer*) hBfSl;
	if (!pBfSl)
		return -1;
	return pBfSl->GetCandidateNum();
}

int r2ssp_bfsl_get_candidate(r2ssp_handle hBfSl, int nCandi,
		r2_sound_candidate *pCandidate) {
	BeamLocalizer *pBfSl = (BeamLocalizer*) hBfSl;
	if (!pBfSl || !pCandidate)
		return -1;
	R2SoundCandidate sc;
	if (pBfSl->GetCandidate(nCandi, sc) == 0) {
		pCandidate->serial = sc.serial;
		pCandidate->azimuth = sc.azimuth;
		pCandidate->elevation = sc.elevation;
		pCandidate->confidence = sc.confidence;
		pCandidate->ticks = sc.ticks;
		return 0;
	} else
		return -2;
}

/*
 JNIEXPORT jlong JNICALL Java_com_rokid_ssp_R2SspWrapper_bf2Create(
 JNIEnv* env, jobject thiz, jfloatArray mics) {
 if (!mics)
 return 0;
 jfloat *pMics = 0;
 jsize nMicNum = 0;
 pMics = env->GetFloatArrayElements(mics, 0);
 nMicNum = env->GetArrayLength(mics) / 3;
 if (nMicNum <= 1) {
 if (pMics != 0)
 env->ReleaseFloatArrayElements(mics, pMics, JNI_ABORT);
 return 0;
 }

 bf_point *pts = new bf_point[nMicNum];
 for (int i = 0; i < nMicNum; i++) {
 pts[i].x = pMics[i * 3];
 pts[i].y = pMics[i * 3 + 1];
 pts[i].z = pMics[i * 3 + 2];
 }
 bf_handle hBf = r2_bf_create(pts, nMicNum);
 delete[] pts;

 if (pMics != 0)
 env->ReleaseFloatArrayElements(mics, pMics, JNI_ABORT);
 return hBf;
 }

 JNIEXPORT jint JNICALL Java_com_rokid_ssp_R2SspWrapper_bf2Free(JNIEnv* env,
 jobject thiz, jlong hBf) {
 return r2_bf_free(hBf);
 }

 JNIEXPORT jint JNICALL Java_com_rokid_ssp_R2SspWrapper_bf2Init(JNIEnv* env,
 jobject thiz, jlong hBf, jint frameSizeMs, jint sampleRate,
 jfloat azimuth, jfloat elevation) {
 return r2_bf_init(hBf, frameSizeMs, sampleRate, azimuth, elevation);
 }

 JNIEXPORT jint JNICALL Java_com_rokid_ssp_R2SspWrapper_bf2Process(
 JNIEnv* env, jobject thiz, jlong hBf, jshortArray inFrames,
 jshortArray outFrame) {
 if (!hBf || !inFrames || !outFrame)
 return -1;

 jshort *pInFrames = 0;
 jsize nInFrameLength = 0;
 pInFrames = env->GetShortArrayElements(inFrames, 0);
 nInFrameLength = env->GetArrayLength(inFrames);
 jshort *pOutFrame = 0;
 jsize nOutFrameLength = 0;
 pOutFrame = env->GetShortArrayElements(outFrame, 0);
 nOutFrameLength = env->GetArrayLength(outFrame);

 float *pInFloatFrames = new float[nInFrameLength];
 for (int i = 0; i < nInFrameLength; i++)
 pInFloatFrames[i] = pInFrames[i];
 float *pOutFloatFrame = new float[nOutFrameLength];

 int nRet = r2_bf_process(hBf, pInFloatFrames, pOutFloatFrame);
 for (int i = 0; i < nOutFrameLength; i++)
 pOutFrame[i] = pOutFloatFrame[i];
 delete[] pInFloatFrames;
 delete[] pOutFloatFrame;

 if (pInFrames != 0)
 env->ReleaseShortArrayElements(inFrames, pInFrames, JNI_ABORT);
 if (pOutFrame != 0)
 env->ReleaseShortArrayElements(outFrame, pOutFrame, 0);
 return nRet;
 }
 */

#ifdef R2SSP_BUILD_JNI
JNIEXPORT jlong JNICALL Java_com_rokid_ssp_R2SspWrapper_bfCreate(JNIEnv* env,
		jobject thiz, jfloatArray mics) {
	if (!mics)
		return 0;
	jfloat *pMics = 0;
	jsize nMicNum = 0;
	pMics = env->GetFloatArrayElements(mics, 0);
	nMicNum = env->GetArrayLength(mics) / 3;
	if (nMicNum <= 1) {
		if (pMics != 0)
			env->ReleaseFloatArrayElements(mics, pMics, JNI_ABORT);
		return 0;
	}

	r2ssp_handle hBf = r2ssp_bf_create(pMics, nMicNum);

	if (pMics != 0)
		env->ReleaseFloatArrayElements(mics, pMics, JNI_ABORT);
	return hBf;
}

JNIEXPORT jint JNICALL Java_com_rokid_ssp_R2SspWrapper_bfFree(JNIEnv* env,
		jobject thiz, jlong hBf) {
	return r2ssp_bf_free(hBf);
}

JNIEXPORT jint JNICALL Java_com_rokid_ssp_R2SspWrapper_bfInit(JNIEnv* env,
		jobject thiz, jlong hBf, jint frameSizeMs, jint sampleRate) {
	return r2ssp_bf_init(hBf, frameSizeMs, sampleRate);
}

JNIEXPORT jint JNICALL Java_com_rokid_ssp_R2SspWrapper_bfSteer(JNIEnv* env,
		jobject thiz, jlong hBfUla, jfloat targetAngle, jfloat targetAngle2,
		jfloat interfAngle, jfloat interfAngle2) {
	return r2ssp_bf_steer(hBfUla, targetAngle, targetAngle2, interfAngle,
			interfAngle2);
}

JNIEXPORT jint JNICALL Java_com_rokid_ssp_R2SspWrapper_bfProcess(JNIEnv* env,
		jobject thiz, jlong hBf, jfloatArray inFrames, jint channels,
		jfloatArray outFrame) {
	if (!hBf || !inFrames || channels <= 1 || !outFrame)
		return -1;

	jfloat *pInFrames = 0;
	jsize nInFrameLength = 0;
	pInFrames = env->GetFloatArrayElements(inFrames, 0);
	nInFrameLength = env->GetArrayLength(inFrames);
	jfloat *pOutFrame = 0;
	jsize nOutFrameLength = 0;
	pOutFrame = env->GetFloatArrayElements(outFrame, 0);
	nOutFrameLength = env->GetArrayLength(outFrame);
	if (nInFrameLength != nOutFrameLength * channels)
		return -1;

	int nRet = r2ssp_bf_process(hBf, pInFrames, nInFrameLength, channels,
			pOutFrame);

	if (pInFrames != 0)
		env->ReleaseFloatArrayElements(inFrames, pInFrames, JNI_ABORT);
	if (pOutFrame != 0)
		env->ReleaseFloatArrayElements(outFrame, pOutFrame, 0);
	return nRet;
}

#endif

/*
 static const int g_kFeFrameSizeMs = 10;
 static const int g_kFeSampleRate = 16000;
 static const int g_kFeFBankFeaLen = ALN_FUL_DIM; // 957/960 (aligned) floats!
 static const int g_kFeVadFeaLen = (((VEC_DIM * 3) * 3) + 3) / 4 * 4; // 264 floats
 static const int g_kFeMaxEnergyFrames = 100;

 struct FEWrapper {
 FEWrapper() {
 m_hFE = 0;
 m_pFeatures = 0;
 m_nPreFeaFrameNum = 0;
 m_nFeaFrameNum = 0;
 }
 ~FEWrapper() {
 if (m_hFE)
 CloseFE(m_hFE);
 }

 HANDLE m_hFE;
 float* m_pFeatures;
 int m_nPreFeaFrameNum;
 int m_nFeaFrameNum;
 float m_energies[g_kFeMaxEnergyFrames];
 };

 r2ssp_handle r2ssp_fe_create(int nType, int nFrameSizeMs, int nSampleRate) {
 if (nType != 0 || nFrameSizeMs != g_kFeFrameSizeMs
 || nSampleRate != g_kFeSampleRate)
 return 0;

 FE_HDR m_feHdr;
 char dir[] = "";
 HANDLE hFE = OpenFE(dir, m_feHdr);
 if (!hFE)
 return 0;
 FEWrapper *pFe = new FEWrapper();
 pFe->m_hFE = hFE;
 pFe->m_nPreFeaFrameNum = 0;

 return (r2ssp_handle) pFe;
 }

 int r2ssp_fe_free(r2ssp_handle hFe) {
 FEWrapper *pFe = (FEWrapper*) hFe;
 if (!pFe)
 return -1;

 delete pFe;
 return 0;
 }

 int r2ssp_fe_get_featurelen(r2ssp_handle hFe, int type) {
 FEWrapper *pFe = (FEWrapper*) hFe;
 if (!pFe)
 return -1;

 if (type == 0)
 return g_kFeFBankFeaLen;
 else if (type == 1)
 return g_kFeVadFeaLen;
 else
 return 1;
 }

 int r2ssp_fe_reset(r2ssp_handle hFe) {
 FEWrapper *pFe = (FEWrapper*) hFe;
 if (!pFe)
 return -1;

 return ResetFE(pFe->m_hFE);
 }

 int r2ssp_fe_process(r2ssp_handle hFe, const float *pInFrames, int nFrameNum) {
 FEWrapper *pFe = (FEWrapper*) hFe;
 if (!pFe)
 return -1;

 static const int nFrameSize = g_kFeFrameSizeMs * g_kFeSampleRate / 1000;
 ContinueFE(pFe->m_hFE, pInFrames, nFrameNum * nFrameSize);

 float * pFloatFeature = 0;
 int * pIntSNR = 0;
 int nFrm = 0;
 pFe->m_nFeaFrameNum = 0;
 if (FE_OK != NormDiffFE(pFe->m_hFE, pFloatFeature, pIntSNR, nFrm, false))
 return -1;

 //LOGD("NormDiffFE: %d\n", nFrm);
 pFe->m_nFeaFrameNum = nFrm - pFe->m_nPreFeaFrameNum;
 pFe->m_pFeatures = pFloatFeature
 + pFe->m_nPreFeaFrameNum * g_kFeFBankFeaLen;
 pFe->m_nPreFeaFrameNum = nFrm;

 float *pEnergies = 0;
 GetEnergyFE(pFe->m_hFE, pEnergies);
 if (pEnergies) {
 memcpy(pFe->m_energies, pEnergies, nFrameNum*sizeof(float));
 }

 return 0;
 }

 int r2ssp_fe_get_featurenum(r2ssp_handle hFe) {
 FEWrapper *pFe = (FEWrapper*) hFe;
 if (!pFe)
 return -1;

 return pFe->m_nFeaFrameNum;
 }

 int r2ssp_fe_get_features(r2ssp_handle hFe, float *pOutFeatures,
 int nOutFrameNum, int type) {
 FEWrapper *pFe = (FEWrapper*) hFe;
 if (!pFe)
 return -1;

 if (nOutFrameNum > pFe->m_nFeaFrameNum)
 nOutFrameNum = pFe->m_nFeaFrameNum;
 if (nOutFrameNum > 0 && pOutFeatures) {
 if (type == 0) // left 5 + center + right 5
 memcpy(pOutFeatures, pFe->m_pFeatures,
 nOutFrameNum * g_kFeFBankFeaLen);
 else if (type == 1) { // left 1 + center + right 1 (for VAD)
 const int kSinglFeaLen = (VEC_DIM * 3);
 float *pFeatures = pFe->m_pFeatures + 4 * kSinglFeaLen;
 for (int i = 0; i < nOutFrameNum; i++) {
 memcpy(pOutFeatures, pFeatures, 3 * kSinglFeaLen);
 pFeatures += g_kFeFBankFeaLen;
 pOutFeatures += g_kFeVadFeaLen;
 }
 } else { // return energy of each frame
 if (nOutFrameNum > g_kFeMaxEnergyFrames)
 nOutFrameNum= g_kFeMaxEnergyFrames;
 memcpy(pOutFeatures, pFe->m_energies, nOutFrameNum*sizeof(float));
 }
 }

 return nOutFrameNum;
 }
 */
/*
 JNIEXPORT jlong JNICALL Java_com_rokid_ssp_R2SspWrapper_feCreate(JNIEnv* env,
 jobject thiz, jint type, jint frameSizeMs, jint sampleRate) {
 return r2ssp_fe_create(type, frameSizeMs, sampleRate);
 }

 JNIEXPORT jint JNICALL Java_com_rokid_ssp_R2SspWrapper_feFree(JNIEnv* env,
 jobject thiz, jlong hFe) {
 return r2ssp_fe_free(hFe);
 }

 JNIEXPORT jint JNICALL Java_com_rokid_ssp_R2SspWrapper_feGetFeatureLen(
 JNIEnv* env, jobject thiz, jlong hFe, jint type) {
 return r2ssp_fe_get_featurelen(hFe, type);
 }

 JNIEXPORT jint JNICALL Java_com_rokid_ssp_R2SspWrapper_feReset(JNIEnv* env,
 jobject thiz, jlong hFe) {
 return r2ssp_fe_reset(hFe);
 }

 JNIEXPORT jint JNICALL Java_com_rokid_ssp_R2SspWrapper_feProcess(JNIEnv* env,
 jobject thiz, jlong hFe, jfloatArray inFrames, jint frameNum) {
 if (!hFe || !inFrames || frameNum <= 0)
 return -1;

 jfloat *pInFrames = 0;
 jsize nInFrameLength = 0;
 pInFrames = env->GetFloatArrayElements(inFrames, 0);
 nInFrameLength = env->GetArrayLength(inFrames);
 if (!pInFrames || nInFrameLength < frameNum ) { // * frameLength
 if (pInFrames != 0)
 env->ReleaseFloatArrayElements(inFrames, pInFrames, JNI_ABORT);
 return -1;
 }

 int nRet = r2ssp_fe_process(hFe, pInFrames, frameNum);

 if (pInFrames != 0)
 env->ReleaseFloatArrayElements(inFrames, pInFrames, JNI_ABORT);
 return nRet;
 }

 JNIEXPORT jint JNICALL Java_com_rokid_ssp_R2SspWrapper_feGetFeatureNum(
 JNIEnv* env, jobject thiz, jlong hFe) {
 return r2ssp_fe_get_featurenum(hFe);
 }

 JNIEXPORT jint JNICALL Java_com_rokid_ssp_R2SspWrapper_feGetFeatures(
 JNIEnv* env, jobject thiz, jlong hFe, jfloatArray outFeatures,
 jint outFrameNum, jint type) {
 if (!hFe || !outFeatures || outFrameNum <= 0)
 return -1;

 jfloat *pOutFrames = 0;
 jsize nOutFrameLength = 0;
 pOutFrames = env->GetFloatArrayElements(outFeatures, 0);
 nOutFrameLength = env->GetArrayLength(outFeatures);
 if (!pOutFrames
 || nOutFrameLength
 < outFrameNum * r2ssp_fe_get_featurelen(hFe, type)) {
 if (pOutFrames != 0)
 env->ReleaseFloatArrayElements(outFeatures, pOutFrames, JNI_ABORT);
 return -1;
 }

 int nRet = r2ssp_fe_get_features(hFe, pOutFrames, outFrameNum, type);

 if (pOutFrames != 0)
 env->ReleaseFloatArrayElements(outFeatures, pOutFrames, 0);
 return nRet;
 }
 */
r2ssp_handle r2ssp_rs_create() {
	PushResampler<float> *pResampler = new PushResampler<float>();
	return (r2ssp_handle) pResampler;
}

int r2ssp_rs_free(r2ssp_handle hRs) {
	PushResampler<float> *pResampler = (PushResampler<float>*) hRs;
	if (!pResampler)
		return -1;
	delete pResampler;
	return 0;
}

int r2ssp_rs_init(r2ssp_handle hRs, int inSampleRate, int outSampleRate) {
	PushResampler<float> *pResampler = (PushResampler<float>*) hRs;
	if (!pResampler)
		return -1;
	return pResampler->InitializeIfNeeded(inSampleRate, outSampleRate, 1);
}

int r2ssp_rs_process(r2ssp_handle hRs, const float *pInFrame, int inFrameLen,
		float *pOutFrame, int outFrameCapacity) {
	PushResampler<float> *pResampler = (PushResampler<float>*) hRs;
	if (!pResampler)
		return -1;
	return pResampler->Resample(pInFrame, inFrameLen, pOutFrame,
			outFrameCapacity);
}

#ifdef R2SSP_BUILD_JNI
JNIEXPORT jlong JNICALL Java_com_rokid_ssp_R2SspWrapper_rsCreate(JNIEnv* env,
		jobject thiz) {
	return r2ssp_rs_create();
}

JNIEXPORT jint JNICALL Java_com_rokid_ssp_R2SspWrapper_rsFree(JNIEnv* env,
		jobject thiz, jlong hRs) {
	return r2ssp_rs_free(hRs);
}

JNIEXPORT jint JNICALL Java_com_rokid_ssp_R2SspWrapper_rsInit(JNIEnv* env,
		jobject thiz, jlong hRs, jint inSampleRate, jint outSampleRate) {
	return r2ssp_rs_init(hRs, inSampleRate, outSampleRate);
}

JNIEXPORT jint JNICALL Java_com_rokid_ssp_R2SspWrapper_rsProcess(JNIEnv* env,
		jobject thiz, jlong hRs, jfloatArray inFrame, jfloatArray outFrame) {
	if (!hRs || !inFrame || !outFrame)
		return -1;

	jfloat *pInFrame = 0;
	jsize nInFrameLength = 0;
	pInFrame = env->GetFloatArrayElements(inFrame, 0);
	nInFrameLength = env->GetArrayLength(inFrame);
	jfloat *pOutFrame = 0;
	jsize nOutFrameLength = 0;
	pOutFrame = env->GetFloatArrayElements(outFrame, 0);
	nOutFrameLength = env->GetArrayLength(outFrame);

	int nRet = r2ssp_rs_process(hRs, pInFrame, nInFrameLength, pOutFrame,
			nOutFrameLength);

	if (pInFrame != 0)
		env->ReleaseFloatArrayElements(inFrame, pInFrame, JNI_ABORT);
	if (pOutFrame != 0)
		env->ReleaseFloatArrayElements(outFrame, pOutFrame, 0);
	return nRet;
}
#endif

/*
 r2ssp_handle r2ssp_sl_create(float *pMics, int nMicNum) {
 return 12345678;
 }

 int r2ssp_sl_free(r2ssp_handle hSl) {
 return 0;
 }

 int r2ssp_sl_init(r2ssp_handle hSl, int nFrameSizeMs, int nSampleRate) {
 return 0;
 }

 int r2ssp_sl_set_param(r2ssp_handle hSl, int param, float value) {
 return 0;
 }

 int r2ssp_sl_target_sound(r2ssp_handle hSl, int startFrame, int stopFrame) {
 return 0;
 }

 int r2ssp_sl_process(r2ssp_handle hSl, const float *pInFrame, int *steerFlag) {
 if (steerFlag)
 if (rand() % 300 == 0)
 *steerFlag = 1;
 else
 *steerFlag = 0;
 return 0;
 }

 int r2ssp_sl_get_candidates(r2ssp_handle hSl, float *pCandidates,
 int nCandiNum) {
 if (pCandidates && nCandiNum >= 1) {
 pCandidates[0] = 0.f;
 pCandidates[1] = 0.f;
 return 1;
 }
 return 0;
 }
 */

#ifdef R2SSP_BUILD_JNI
#ifdef R2SSP_USING_BLIS
JNIEXPORT jint JNICALL Java_com_rokid_ssp_R2SspWrapper_cblasSetThreading(
		JNIEnv* env, jobject thiz, jint IR, jint JR, jint IC, jint JC) {
	if (IR < 1 || JR < 1 || IC < 1 || JC < 1)
		return -1;

	char threadNum[16];
	sprintf(threadNum, "%d", IR);
	setenv("BLIS_IR_NT", threadNum, 1);
	sprintf(threadNum, "%d", JR);
	setenv("BLIS_JR_NT", threadNum, 1);
	sprintf(threadNum, "%d", IC);
	setenv("BLIS_IC_NT", threadNum, 1);
	sprintf(threadNum, "%d", JC);
	setenv("BLIS_JC_NT", threadNum, 1);
	return 0;
}

JNIEXPORT jint JNICALL Java_com_rokid_ssp_R2SspWrapper_cblasSgemm(JNIEnv* env,
		jobject thiz, jboolean rowMajor, jboolean transA, jboolean transB,
		jint M, jint N, jint K, jfloat alpha, jfloatArray A, jint lda,
		jfloatArray B, jint ldb, jfloat beta, jfloatArray C, jint ldc) {
	if (!A || !B || !C || M <= 0 || N <= 0 || K <= 0 || lda <= 0 || ldb <= 0
			|| ldc <= 0)
		return -1;

	jfloat *pA = 0;
	jsize nA = 0;
	pA = (jfloat*) env->GetPrimitiveArrayCritical(A, 0);
	nA = env->GetArrayLength(A);
	jfloat *pB = 0;
	jsize nB = 0;
	pB = (jfloat*) env->GetPrimitiveArrayCritical(B, 0);
	nB = env->GetArrayLength(B);
	jfloat *pC = 0;
	jsize nC = 0;
	pC = (jfloat*) env->GetPrimitiveArrayCritical(C, 0);
	nC = env->GetArrayLength(C);
	if (nA < M * K || nB < K * N || nC < M * N)
		return -2;

	cblas_sgemm((rowMajor ? CblasRowMajor : CblasColMajor),
			(transA ? CblasTrans : CblasNoTrans),
			(transB ? CblasTrans : CblasNoTrans), M, N, K, alpha, pA, lda, pB,
			ldb, beta, pC, ldc);

	if (pA)
		env->ReleasePrimitiveArrayCritical(A, pA, JNI_ABORT);
	if (pB)
		env->ReleasePrimitiveArrayCritical(B, pB, JNI_ABORT);
	if (pC)
		//env->ReleasePrimitiveArrayCritical(C, pC, 0);
		env->ReleasePrimitiveArrayCritical(C, pC, JNI_ABORT); // for debug speed
	return 0;
}
#endif
#endif

// jni ssp test thread functions
/*
 JavaVM *g_jvm = 0;
 jobject g_obj = 0;
 pthread_t g_pt = 0;
 int g_stop = 0;

 static int call_java_notify_method(int source, int type, int ival1, int ival2,
 float fval1, float fval2, float fval3, float fval4, jstring sval1,
 jstring sval2) {
 JNIEnv *env;
 jclass cls;
 jmethodID mid;

 //Attach java thread
 if (g_jvm->AttachCurrentThread(&env, NULL) != JNI_OK) {
 LOGE("%s: AttachCurrentThread() failed", __FUNCTION__);
 return -1;
 }
 // find class
 cls = env->GetObjectClass(g_obj);
 if (cls == NULL) {
 LOGE("FindClass() Error.....");
 goto error;
 }
 // get method
 mid = env->GetStaticMethodID(cls, "sspNotify",
 "(IIIIFFFFLjava/lang/String;Ljava/lang/String;)I");
 if (mid == NULL) {
 LOGE("%s: GetMethodID() Error.....", __FUNCTION__);
 goto error;
 }
 // call java static method
 int ret;
 ret = env->CallStaticIntMethod(cls, mid, source, type, ival1, ival2,
 fval1, fval2, fval3, fval4, sval1, sval2);

 error:
 //Detach java thread
 if (g_jvm->DetachCurrentThread() != JNI_OK) {
 LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
 }
 return ret;
 }

 static void *r2ssp_threadRun(void* arg) {

 pthread_exit(0);
 return 0;
 }

 JNIEXPORT int JNICALL Java_com_rokid_ssp_R2SspWrapper_sspStart(JNIEnv* env,
 jobject thiz, jint param) {
 if (g_obj)
 return -1;
 env->GetJavaVM(&g_jvm);
 g_obj = env->NewGlobalRef(thiz);

 int ret;
 if ((ret = pthread_create(&g_pt, NULL, r2ssp_threadRun, (void *) param))
 != 0) {
 g_pt = 0;
 env->DeleteGlobalRef(g_obj);
 g_jvm = 0;
 g_obj = 0;
 }

 return ret;
 }

 JNIEXPORT int JNICALL Java_com_rokid_ssp_R2SspWrapper_sspStop(JNIEnv* env,
 jobject thiz) {
 if (!g_pt)
 return -1;

 void *thread_ret = 0;
 g_stop = 1;
 pthread_join(g_pt, &thread_ret);
 g_stop = 0;

 env->DeleteGlobalRef(g_obj);
 g_jvm = 0;
 g_obj = 0;
 return 0;
 }
 */

r2ssp_handle r2ssp_agc_create() {
	/*Agc *pAgc = new Agc();
	 //pAgc->set_target_level_dbfs(0);
	 pAgc->EnableStandaloneVad(true);
	 return (r2ssp_handle)pAgc;*/

	void *agcInst;
	WebRtcAgc_Create(&agcInst);
	return (r2ssp_handle) agcInst;
}

int r2ssp_agc_free(r2ssp_handle hAgc) {
	/*Agc *pAgc = (Agc*)hAgc;
	 if (!pAgc)
	 return -1;
	 delete pAgc;*/

	void *agcInst = (void*) hAgc;
	if (!agcInst)
		return -1;
	return WebRtcAgc_Free(agcInst);
}

int r2ssp_agc_init(r2ssp_handle hAgc, int nFrameSizeMs, int nSampleRate) {
	if (nSampleRate != 16000 || nFrameSizeMs != 10)
		return -2;

	/*Agc *pAgc = (Agc*)hAgc;
	 if (!pAgc)
	 return -1;*/

	void *agcInst = (void*) hAgc;
	if (!agcInst)
		return -1;
	return WebRtcAgc_Init(agcInst, 0, 0, kAgcModeAdaptiveDigital, nSampleRate);
}

int r2ssp_agc_reset(r2ssp_handle hAgc) {
	/*Agc *pAgc = (Agc*)hAgc;
	 if (!pAgc)
	 return -1;
	 pAgc->Reset();*/

	return 0;
}

int r2ssp_agc_process(r2ssp_handle hAgc, short *pInFrame) {
	/*Agc *pAgc = (Agc*)hAgc;
	 if (!pAgc)
	 return -1;
	 return pAgc->Process(pInFrame, 160, 16000);*/

	void *agcInst = (void*) hAgc;
	if (!agcInst)
		return -1;
	WebRtcAgc_AddMic(agcInst, &pInFrame, 1, 160);
	short outFrame[160];
	short *pOutFrame = outFrame;
	int outMicLevel;
	unsigned char satWarn;
	int ret = WebRtcAgc_Process(agcInst, &pInFrame, 1, 160, &pOutFrame, 0,
			&outMicLevel, 0, &satWarn);
	memcpy(pInFrame, outFrame, 160 * sizeof(short));
	return ret;
}

int r2ssp_agc_get_diff_dbfs(r2ssp_handle hAgc, int *bNeedUpdate, int *dbfs) {
	/*Agc *pAgc = (Agc*)hAgc;
	 if (!pAgc || !bNeedUpdate || !dbfs)
	 return -1;
	 *bNeedUpdate = pAgc->GetRmsErrorDb(dbfs);*/

	return 0;
}

r2ssp_handle r2ssp_bpf_create() {
    BeamFilter *pBpf = new BeamFilter();
    return (r2ssp_handle)pBpf;
}

int r2ssp_bpf_free(r2ssp_handle hBpf) {
    BeamFilter *pBpf = (BeamFilter*)hBpf;
    if (!pBpf)
        return -1;
    delete pBpf;
    return 0;
}

int r2ssp_bpf_init(r2ssp_handle hBpf, int nFrameSizeMs, int nSampleRate,
                       int nStartFreqBin, int nStopFreqBin, int bBandPass) {
    BeamFilter *pBpf = (BeamFilter*)hBpf;
    if (!pBpf)
        return -1;
	pBpf->Initialize(nFrameSizeMs, nSampleRate, nStartFreqBin, nStopFreqBin,
			bBandPass);
    return 0;
}

int r2ssp_bpf_set_freqscales(r2ssp_handle hBpf, float *pScales, int nLength) {
    BeamFilter *pBpf = (BeamFilter*)hBpf;
    if (!pBpf)
        return -1;
    return pBpf->SetFreqScales(pScales, nLength);
}

int r2ssp_bpf_process(r2ssp_handle hBpf, const float *pFrame, int nFrameSize,
		float *pOutFrame) {
    BeamFilter *pBpf = (BeamFilter*)hBpf;
    if (!pBpf)
        return -1;
    if (!pFrame || !pOutFrame || nFrameSize <= 0)
        return -1;

    ChannelBuffer<float> inFloatBuffer(nFrameSize, 1, 1);
    ChannelBuffer<float> outFloatBuffer(nFrameSize, 1);

    inFloatBuffer.SetDataForTesting(pFrame, nFrameSize);
    pBpf->ProcessChunk(&inFloatBuffer, &outFloatBuffer);
    
    for (int i = 0; i < nFrameSize; i++)
        pOutFrame[i] = outFloatBuffer.channels(0)[0][i];

    return 0;
}

r2ssp_handle r2ssp_fa_create() {
    BeamAdapter *pFa = new BeamAdapter();
    return (r2ssp_handle)pFa;
}

int r2ssp_fa_free(r2ssp_handle hFa) {
    BeamAdapter *pFa = (BeamAdapter*)hFa;
    if (!pFa)
        return -1;
    delete pFa;
    return 0;
}

int r2ssp_fa_init(r2ssp_handle hFa, int nFrameSizeMs, int nSampleRate) {
    BeamAdapter *pFa = (BeamAdapter*)hFa;
    if (!pFa)
        return -1;
    pFa->Initialize(nFrameSizeMs, nSampleRate);
    return 0;
}

int r2ssp_fa_is_adapted(r2ssp_handle hFa) {
    BeamAdapter *pFa = (BeamAdapter*)hFa;
    if (!pFa)
        return -1;
    return pFa->IsAdapted();
}

int r2ssp_fa_get_freqscales(r2ssp_handle hFa, float *pScales, int nLength) {
    BeamAdapter *pFa = (BeamAdapter*)hFa;
    if (!pFa)
        return -1;
    return pFa->GetFreqScales(pScales, nLength);
}

int r2ssp_fa_process(r2ssp_handle hFa, const float *pFrames, int nChunkSize,
		float *pOutFrame) {
    BeamAdapter *pFa = (BeamAdapter*)hFa;
    if (!pFa)
        return -1;
    if (!pFrames || !pOutFrame || nChunkSize <= 0)
        return -1;

    int nFrameSize = nChunkSize / 2;
    ChannelBuffer<float> inFloatBuffer(nFrameSize, 2, 1);
    ChannelBuffer<float> outFloatBuffer(nFrameSize, 1);

    inFloatBuffer.SetDataForTesting(pFrames, nChunkSize);
    pFa->ProcessChunk(&inFloatBuffer, &outFloatBuffer);

    for (int i = 0; i < nFrameSize; i++)
        pOutFrame[i] = outFloatBuffer.channels(0)[0][i];
        
    return 0;
}

r2ssp_handle r2ssp_sa_create() {
    r2ssp_sa_handle *pSa = new r2ssp_sa_handle();
    pSa->agc_handle = r2ssp_agc_create();
    pSa->ns_handle = r2ssp_ns_create();
    return (r2ssp_handle)pSa;
}

int r2ssp_sa_free(r2ssp_handle hSa) {
    r2ssp_sa_handle *pSa = (r2ssp_sa_handle*)hSa;
    if (!pSa)
        return -1;
    r2ssp_agc_free(pSa->agc_handle);
    r2ssp_ns_free(pSa->ns_handle);
    delete pSa;
    return 0;
}

int r2ssp_sa_init(r2ssp_handle hSa, int nFrameSizeMs, int nSampleRate) {
    r2ssp_sa_handle *pSa = (r2ssp_sa_handle*)hSa;
    if (!pSa)
        return -1;

    int nRet = r2ssp_agc_init(pSa->agc_handle, nFrameSizeMs, nSampleRate);
    if (nRet)
        return nRet;
    nRet = r2ssp_ns_init(pSa->ns_handle, nSampleRate);
    if (nRet)
        return nRet;
    nRet = r2ssp_ns_set_mode(pSa->ns_handle, 0);
    return nRet;
}

int r2ssp_sa_reset(r2ssp_handle hSa) {
    r2ssp_sa_handle *pSa = (r2ssp_sa_handle*)hSa;
    if (!pSa)
        return -1;

    r2ssp_agc_reset(pSa->agc_handle);
    return 0;
}

int r2ssp_sa_process(r2ssp_handle hSa, short *pInFrame) {
    r2ssp_sa_handle *pSa = (r2ssp_sa_handle*)hSa;
    if (!pSa)
        return -1;

    int nRet = r2ssp_agc_process(pSa->agc_handle, pInFrame);
    if (nRet)
        return nRet;

    const int nFrameSize = 160;
    float inFloatFrame[nFrameSize];
    float outFloatFrame[nFrameSize];
    for (int i=0; i<nFrameSize; i++)
        inFloatFrame[i] = pInFrame[i];
    nRet = r2ssp_ns_process(pSa->ns_handle, inFloatFrame, outFloatFrame);
    if (nRet)
        return nRet;
    for (int i=0; i<nFrameSize; i++)
        pInFrame[i] = outFloatFrame[i];
    return 0;
}

#ifdef __cplusplus
}
#endif
