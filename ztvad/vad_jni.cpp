/* Header for class com_ztspeech_vad */

#include <jni.h>
#include <stdlib.h>
#include <assert.h>

#include "webrtc/common_audio/signal_processing/include/signal_processing_library.h"

#define LOG_TAG "vad_jni"
#include <android/log.h>
#undef LOGD
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#undef LOGE
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#include "NNVadIntf.h"
#include "felib/fftwrapper.h"
#include "blis/blis.h"
#ifdef FE_USE_NE10_FFT
#include "NE10.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	JNIEnv* env = NULL;

	jint result = -1;
	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		LOGE("ERROR: GetEnv failed\n");
		goto bail;
	}

	assert(env != NULL);

	/* success -- return valid version number */
	result = JNI_VERSION_1_4;

	bail:

	WebRtcSpl_Init();
	FFTWrapper::Init();
	bli_init();
#ifdef FE_USE_NE10_FFT
	ne10_init();
#endif

	return result;
}

JNIEXPORT jlong JNICALL Java_com_ztspeech_vad_VadJni_newVad(JNIEnv* env,
		jobject thiz, jint sampleRate, jint frameLenMs, jint mode) {
	return jlong(NNV_NewVad(sampleRate, frameLenMs, mode));
}

JNIEXPORT jboolean JNICALL Java_com_ztspeech_vad_VadJni_resetVad(JNIEnv* env,
		jobject thiz, jlong hVad) {
	return NNV_ResetVad((NNV_HANDLE) hVad);
}

JNIEXPORT jboolean JNICALL Java_com_ztspeech_vad_VadJni_restartVad(JNIEnv* env,
		jobject thiz, jlong hVad) {
	return NNV_RestartVad((NNV_HANDLE) hVad);
}

JNIEXPORT jboolean JNICALL Java_com_ztspeech_vad_VadJni_delVad(JNIEnv* env,
		jobject thiz, jlong hVad) {
	return NNV_DelVad((NNV_HANDLE) hVad);
}

JNIEXPORT jint JNICALL Java_com_ztspeech_vad_VadJni_setIntParam(JNIEnv* env,
		jobject thiz, jlong hVad, jint param, jint val) {
	return NNV_SetVadParam((NNV_HANDLE) hVad, param, (void*) &val);
}

JNIEXPORT jint JNICALL Java_com_ztspeech_vad_VadJni_setFloatParam(JNIEnv* env,
		jobject thiz, jlong hVad, jint param, jfloat val) {
	return NNV_SetVadParam((NNV_HANDLE) hVad, param, (void*) &val);
}

JNIEXPORT jboolean JNICALL Java_com_ztspeech_vad_VadJni_inputWave(JNIEnv* env,
		jobject thiz, jlong hVad, jbyteArray waveData, jint offset,
		jint frameNum, jboolean isEnd) {
	jbyte *pWaveData = 0;
	jsize nWaveLen = 0;
	if (waveData != 0) {
		pWaveData = env->GetByteArrayElements(waveData, 0);
		nWaveLen = env->GetArrayLength(waveData);
	}

	int bRet = 0;
	if (offset + VAD_SAMPLE_RATE * VAD_FRAME_LENMS / 1000 * frameNum
			<= nWaveLen)
		bRet = NNV_InputWave((NNV_HANDLE) hVad, (char*) (pWaveData + offset),
				frameNum, (isEnd ? 1 : 0), 0);

	if (waveData != 0)
		env->ReleaseByteArrayElements(waveData, pWaveData, JNI_ABORT);
	return bRet;
}

JNIEXPORT jboolean JNICALL Java_com_ztspeech_vad_VadJni_inputFloatWave(
		JNIEnv* env, jobject thiz, jlong hVad, jfloatArray waveData,
		jint offset, jint frameNum, jboolean isEnd) {
	jfloat *pWaveData = 0;
	jsize nWaveLen = 0;
	if (waveData != 0) {
		pWaveData = env->GetFloatArrayElements(waveData, 0);
		nWaveLen = env->GetArrayLength(waveData);
	}

	int bRet = 0;
	if (offset + VAD_SAMPLE_RATE * VAD_FRAME_LENMS / 1000 * frameNum
			<= nWaveLen)
		bRet = NNV_InputFloatWave((NNV_HANDLE) hVad,
				(float*) (pWaveData + offset), frameNum, (isEnd ? 1 : 0), 0);

	if (waveData != 0)
		env->ReleaseFloatArrayElements(waveData, pWaveData, JNI_ABORT);
	return bRet;
}

JNIEXPORT jboolean JNICALL Java_com_ztspeech_vad_VadJni_freeze(JNIEnv* env,
		jobject thiz, jlong hVad, jint bIsFreeze, jint bIsAec) {
	return NNV_Freeze((NNV_HANDLE) hVad, bIsFreeze, bIsAec);
}

JNIEXPORT jint JNICALL Java_com_ztspeech_vad_VadJni_getMaxFrame(JNIEnv* env,
		jobject thiz, jlong hVad) {
	return NNV_GetMaxFrame((NNV_HANDLE) hVad);
}

JNIEXPORT jfloat JNICALL Java_com_ztspeech_vad_VadJni_getFrameProb(JNIEnv* env,
		jobject thiz, jlong hVad, jint frame, jint dim) {
	return NNV_GetFrameProb((NNV_HANDLE) hVad, frame, dim);
}

JNIEXPORT jlong JNICALL Java_com_ztspeech_vad_VadJni_vdNewVad(JNIEnv* env,
		jobject thiz, jint mode) {
	return jlong(VD_NewVad(mode));
}

JNIEXPORT jboolean JNICALL Java_com_ztspeech_vad_VadJni_vdResetVad(JNIEnv* env,
		jobject thiz, jlong hVad) {
	return VD_ResetVad((VD_HANDLE) hVad);
}

JNIEXPORT jboolean JNICALL Java_com_ztspeech_vad_VadJni_vdRestartVad(JNIEnv* env,
		jobject thiz, jlong hVad) {
	return VD_RestartVad((VD_HANDLE) hVad);
}

JNIEXPORT jboolean JNICALL Java_com_ztspeech_vad_VadJni_vdDelVad(JNIEnv* env,
		jobject thiz, jlong hVad) {
	return VD_DelVad((VD_HANDLE) hVad);
}

JNIEXPORT jint JNICALL Java_com_ztspeech_vad_VadJni_vdSetIntParam(JNIEnv* env,
		jobject thiz, jlong hVad, jint param, jint val) {
	return VD_SetVadParam((VD_HANDLE) hVad, param, (void*) &val);
}

JNIEXPORT jint JNICALL Java_com_ztspeech_vad_VadJni_vdSetFloatParam(JNIEnv* env,
		jobject thiz, jlong hVad, jint param, jfloat val) {
	return VD_SetVadParam((VD_HANDLE) hVad, param, (void*) &val);
}

JNIEXPORT jboolean JNICALL Java_com_ztspeech_vad_VadJni_vdInputWave(JNIEnv* env,
		jobject thiz, jlong hVad, jbyteArray waveData, jint offset, jint length,
		jboolean isEnd) {
	jbyte *pWaveData = 0;
	jsize nWaveLen = 0;
	if (waveData != 0) {
		pWaveData = env->GetByteArrayElements(waveData, 0);
		nWaveLen = env->GetArrayLength(waveData);
	}

	int bRet = 0;
	if (offset + length <= nWaveLen)
		bRet = VD_InputWave((VD_HANDLE) hVad,
				(const short*) (pWaveData + offset), length / sizeof(short),
				(isEnd ? 1 : 0), 0);

	if (waveData != 0)
		env->ReleaseByteArrayElements(waveData, pWaveData, JNI_ABORT);
	return bRet;
}

JNIEXPORT jboolean JNICALL Java_com_ztspeech_vad_VadJni_vdInputFloatWave(
		JNIEnv* env, jobject thiz, jlong hVad, jfloatArray waveData,
		jint offset, jint length, jboolean isEnd) {
	jfloat *pWaveData = 0;
	jsize nWaveLen = 0;
	if (waveData != 0) {
		pWaveData = env->GetFloatArrayElements(waveData, 0);
		nWaveLen = env->GetArrayLength(waveData);
	}

	int bRet = 0;
	if (offset + length <= nWaveLen)
		bRet = VD_InputFloatWave((VD_HANDLE) hVad,
				(float*) (pWaveData + offset), length, (isEnd ? 1 : 0), 0);

	if (waveData != 0)
		env->ReleaseFloatArrayElements(waveData, pWaveData, JNI_ABORT);
	return bRet;
}

JNIEXPORT jint JNICALL Java_com_ztspeech_vad_VadJni_vdGetOffsetFrame(
		JNIEnv* env, jobject thiz, jlong hVad) {
	return VD_GetOffsetFrame((VD_HANDLE) hVad);
}

JNIEXPORT jint JNICALL Java_com_ztspeech_vad_VadJni_vdGetVoiceStartFrame(
		JNIEnv* env, jobject thiz, jlong hVad) {
	return VD_GetVoiceStartFrame((VD_HANDLE) hVad);
}

JNIEXPORT jint JNICALL Java_com_ztspeech_vad_VadJni_vdGetVoiceStopFrame(
		JNIEnv* env, jobject thiz, jlong hVad) {
	return VD_GetVoiceStopFrame((VD_HANDLE) hVad);
}

JNIEXPORT jint JNICALL Java_com_ztspeech_vad_VadJni_vdGetVoiceFrameNum(
		JNIEnv* env, jobject thiz, jlong hVad) {
	return VD_GetVoiceFrameNum((VD_HANDLE) hVad);
}

JNIEXPORT jbyteArray JNICALL Java_com_ztspeech_vad_VadJni_vdGetVoice(
		JNIEnv* env, jobject thiz, jlong hVad, jint startFrame, jint frameNum) {
	//LOGD("vdGetVoice %d %d", startFrame, frameNum);
	jint nStartFrame = VD_GetVoiceStartFrame((VD_HANDLE) hVad);
	jint nFrameNum = VD_GetVoiceFrameNum((VD_HANDLE) hVad);
	jint nOffsetFrame = VD_GetOffsetFrame((VD_HANDLE) hVad);
	if (startFrame + frameNum > nStartFrame + nFrameNum)
		frameNum = nStartFrame + nFrameNum - startFrame;
	const short *pVadWaveData = VD_GetVoice((VD_HANDLE) hVad);
	if (startFrame < 0 || frameNum <= 0 || !pVadWaveData || nStartFrame < 0
			|| nFrameNum <= 0 || startFrame < nStartFrame)
		return 0;

	jbyteArray waveData = env->NewByteArray(frameNum * VAD_FRAME_SIZE * sizeof(short));
	jbyte *pWaveData = env->GetByteArrayElements(waveData, 0);
	memcpy(pWaveData,
			pVadWaveData + (startFrame - nOffsetFrame) * VAD_FRAME_SIZE,
			frameNum * VAD_FRAME_SIZE * sizeof(short));
	env->ReleaseByteArrayElements(waveData, pWaveData, 0);

	//LOGD("vdGetVoiceReturn %d %d", startFrame, frameNum);
	return waveData;
}

JNIEXPORT jfloatArray JNICALL Java_com_ztspeech_vad_VadJni_vdGetFloatVoice(
		JNIEnv* env, jobject thiz, jlong hVad, jint startFrame, jint frameNum) {
	//LOGD("vdGetVoice %d %d", startFrame, frameNum);
	jint nStartFrame = VD_GetVoiceStartFrame((VD_HANDLE) hVad);
	jint nFrameNum = VD_GetVoiceFrameNum((VD_HANDLE) hVad);
	jint nOffsetFrame = VD_GetOffsetFrame((VD_HANDLE) hVad);
	if (startFrame + frameNum > nStartFrame + nFrameNum)
		frameNum = nStartFrame + nFrameNum - startFrame;
	const float *pVadWaveData = VD_GetFloatVoice((VD_HANDLE) hVad);
	if (startFrame < 0 || frameNum <= 0 || !pVadWaveData || nStartFrame < 0
			|| nFrameNum <= 0 || startFrame < nStartFrame)
		return 0;

	jfloatArray waveData = env->NewFloatArray(frameNum * VAD_FRAME_SIZE);
	jfloat *pWaveData = env->GetFloatArrayElements(waveData, 0);
	memcpy(pWaveData,
			pVadWaveData + (startFrame - nOffsetFrame) * VAD_FRAME_SIZE,
			frameNum * VAD_FRAME_SIZE * sizeof(float));
	env->ReleaseFloatArrayElements(waveData, pWaveData, 0);

	//LOGD("vdGetVoiceReturn %d %d", startFrame, frameNum);
	return waveData;
}

#ifdef __cplusplus
}
#endif
