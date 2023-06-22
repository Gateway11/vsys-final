#include "NNVadIntf.h"
#include "VAD.h"
#include "VoiceDetector.h"

#include "webrtc/common_audio/signal_processing/include/signal_processing_library.h"
#ifdef __arm__
#include "blis/blis.h"
#endif

//#define LOG_TAG "vad_jni"
//#include <android/log.h>
//#undef LOGD
//#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)

static const char *g_szVersion = "\nztvad_version_20151217_204502\n";


int NNVAPI VAD_SysInit() {
	WebRtcSpl_Init();
	FFTWrapper::Init();
#ifdef __arm__
	bli_init();
#ifdef FE_USE_NE10_FFT
	ne10_init();
#endif
#endif

	return 0;
}

int NNVAPI VAD_SysExit() {
	return 0;
}

NNV_HANDLE NNVAPI NNV_NewVad(int sampleRate, int frameLenMs, int mode) {
	if (sampleRate < 4000 || frameLenMs < 10)
		return 0;

	//LOGD("NNV_NewVad, mode: %d\n", mode);
	VAD *pVad = new VAD(sampleRate, frameLenMs);
	pVad->SetMode(mode);
	return (NNV_HANDLE) pVad;
}

int NNVAPI NNV_ResetVad(NNV_HANDLE hVad) {
	VAD *pVad = (VAD*) hVad;
	if (pVad) {
		pVad->Reset();
		return 1;
	} else
		return 0;
}

int NNVAPI NNV_RestartVad(NNV_HANDLE hVad) {
    VAD *pVad = (VAD*) hVad;
    if (pVad) {
        pVad->Restart();
        return 1;
    } else
        return 0;
}

int NNVAPI NNV_DelVad(NNV_HANDLE hVad) {
	VAD *pVad = (VAD*) hVad;
	if (pVad) {
		delete pVad;
		return 1;
	} else
		return 0;
}

int NNVAPI NNV_SetVadParam(VD_HANDLE hVad, int nParam, void *pVal) {
	VAD *pVad = (VAD*) hVad;
	if (pVad) {
        switch (nParam) {
            case VD_PARAM_ESTIFRAMENUM:
                pVad->SetEstiFrameNum(*(int*) pVal);
                break;
            case VD_PARAM_MINENERGY:
                pVad->SetMinEnergyThreshold(*(int*) pVal);
                break;
            case VD_PARAM_MAXENERGY:
                pVad->SetMaxEnergyThreshold(*(int*) pVal);
                break;
            case VD_PARAM_WORKMODE:
                pVad->SetMode(*(int*) pVal);
                break;
            case VD_PARAM_WEBRTCMODE:
                pVad->SetWebrtcMode(*(int*) pVal);
                break;
            case VD_PARAM_CTXFRAMENUM:
                pVad->SetCtxFrameNum(*(int*) pVal);
                break;
            case VD_PARAM_BASERANGE:
                pVad->SetBaseRange(*(float*) pVal);
                break;
            case VD_PARAM_MINDYNARANGE:
                pVad->SetMinDynaRange(*(float*) pVal);
                break;
            case VD_PARAM_MAXDYNARANGE:
                pVad->SetMaxDynaRange(*(float*) pVal);
                break;
            case VD_PARAM_MINAECENERGY:
                pVad->SetMinAecEnergy(*(float*) pVal);
                break;
            case VD_PARAM_MINDYNAENERGY:
                pVad->SetMinDynaEnergy(*(float*) pVal);
                break;
            default:
                return 0;
        }
		return 1;
	} else
		return 0;
}

int NNVAPI NNV_InputWave(NNV_HANDLE hVad, char *pWaveData, int nFrameNum,
		int bIsEnd, int nIsAec) {
	VAD *pVad = (VAD*) hVad;
	if (pVad) {
		int bRet = 0;
		bRet = pVad->InputWave((const short*) (pWaveData), nFrameNum, bIsEnd, nIsAec);
		return bRet;
	} else
		return 0;
}

int NNVAPI NNV_InputFloatWave(NNV_HANDLE hVad, float *pWaveData, int nFrameNum,
		int bIsEnd, int nIsAec) {
	VAD *pVad = (VAD*) hVad;
	if (pVad) {
		int bRet = 0;
		bRet = pVad->InputFloatWave(pWaveData, nFrameNum, bIsEnd, nIsAec);
		return bRet;
	} else
		return 0;
}

int NNVAPI NNV_GetMaxFrame(NNV_HANDLE hVad) {
	VAD *pVad = (VAD*) hVad;
	if (pVad) {
		return pVad->GetMaxFrame();
	} else
		return 0;
}

int NNVAPI NNV_OffsetFrame(NNV_HANDLE hVad, int nFrame) {
    VAD *pVad = (VAD*) hVad;
    if (pVad) {
        return pVad->OffsetFrame(nFrame);
    } else
        return 0;
}

float NNVAPI NNV_GetFrameProb(NNV_HANDLE hVad, int nFrame, int nDim) {
	VAD *pVad = (VAD*) hVad;
	if (pVad) {
		return pVad->GetFrameProb(nFrame, nDim);
	} else
		return 0;
}

float NNVAPI NNV_GetLastFrameEnergy(NNV_HANDLE hVad) {
    VAD *pVad = (VAD*) hVad;
    if (pVad) {
        return pVad->GetLastEnergy();
    } else
        return 0;
}

float NNVAPI NNV_GetThresholdEnergy(NNV_HANDLE hVad) {
    VAD *pVad = (VAD*) hVad;
    if (pVad) {
        return pVad->GetThresholdEnergy();
    } else
        return 0;
}

int NNVAPI NNV_Freeze(NNV_HANDLE hVad, int bFreeze, int nIsAec) {
    VAD *pVad = (VAD*) hVad;
    if (pVad) {
        pVad->Freeze(bFreeze, nIsAec);
        return 1;
    } else
        return 0;
}

int NNVAPI NNV_FreezeSpeech(NNV_HANDLE hVad, int bFreeze, int nFrameNum) {
    VAD *pVad = (VAD*) hVad;
    if (pVad) {
        pVad->FreezeSpeech(bFreeze, nFrameNum);
        return 1;
    } else
        return 0;
}


VD_HANDLE NNVAPI VD_NewVad(int nMode) {
	return new VoiceDetector(nMode);
}

int NNVAPI VD_ResetVad(VD_HANDLE hVad) {
	VoiceDetector *pVad = (VoiceDetector*) hVad;
	if (pVad) {
		pVad->Reset();
		return 1;
	} else
		return 0;
}

int NNVAPI VD_RestartVad(VD_HANDLE hVad) {
    VoiceDetector *pVad = (VoiceDetector*) hVad;
    if (pVad) {
        pVad->Restart();
        return 1;
    } else
        return 0;
}

int NNVAPI VD_DelVad(VD_HANDLE hVad) {
	VoiceDetector *pVad = (VoiceDetector*) hVad;
	if (pVad) {
		delete pVad;
		return 1;
	} else
		return 0;
}

int NNVAPI VD_SetVadParam(VD_HANDLE hVad, int nParam, void *pVal) {
	VoiceDetector *pVad = (VoiceDetector*) hVad;
	if (pVad) {
		switch (nParam) {
		case VD_PARAM_MINSILFRAMENUM:
			pVad->SetMinSilFrameNum(*(int*) pVal);
			break;
		case VD_PARAM_PREFRAMENUM:
			pVad->SetPreFrameNum(*(int*) pVal);
			break;
		case VD_PARAM_MINVOCRATIO:
			pVad->SetMinVocRatio(*(float*) pVal);
			break;
		case VD_PARAM_MINVOCFRAMENUM:
			pVad->SetMinVocFrameNum(*(int*) pVal);
			break;
		case VD_PARAM_MINSILRATIO:
			pVad->SetMinSilRatio(*(float*) pVal);
			break;
        case VD_PARAM_MINSPEECHFRAMENUM:
            pVad->SetMinSpeechFrameNum(*(int*) pVal);
            break;
        case VD_PARAM_MAXSPEECHFRAMENUM:
            pVad->SetMaxSpeechFrameNum(*(int*) pVal);
            break;
        case VD_PARAM_ENBLEPITCH:
            pVad->EnablePitchDetection(*(int*) pVal);
            break;
		default:
			return pVad->SetFrameVadParam(nParam, pVal);
		}
		return 1;
	} else
		return 0;
}

int NNVAPI VD_InputWave(VD_HANDLE hVad, const short *pWaveData, int nSampleNum,
                        int bIsEnd, int nIsAec) {
	VoiceDetector *pVad = (VoiceDetector*) hVad;
	if (pVad)
		return pVad->InputWave(pWaveData, nSampleNum, bIsEnd, nIsAec);
	else
		return 0;
}

int NNVAPI VD_InputFloatWave(VD_HANDLE hVad, const float *pWaveData,
		int nSampleNum, int bIsEnd, int nIsAec) {
	VoiceDetector *pVad = (VoiceDetector*) hVad;
	if (pVad)
		return pVad->InputFloatWave(pWaveData, nSampleNum, bIsEnd, nIsAec);
	else
		return 0;
}

int NNVAPI VD_GetOffsetFrame(VD_HANDLE hVad) {
	VoiceDetector *pVad = (VoiceDetector*) hVad;
	if (pVad)
		return pVad->GetOffsetFrame();
	else
		return 0;
}

int NNVAPI VD_SetStart(VD_HANDLE hVad, int nIsAec) {
    VoiceDetector *pVad = (VoiceDetector*) hVad;
    if (pVad)
        return pVad->SetStart(nIsAec);
    else
        return 0;
}

int NNVAPI VD_GetVoiceStartFrame(VD_HANDLE hVad) {
	VoiceDetector *pVad = (VoiceDetector*) hVad;
	if (pVad)
		return pVad->GetVoiceStartFrame();
	else
		return -1;
}

int NNVAPI VD_GetVoiceStopFrame(VD_HANDLE hVad) {
	VoiceDetector *pVad = (VoiceDetector*) hVad;
	if (pVad)
		return pVad->GetVoiceStopFrame();
	else
		return -1;
}

int NNVAPI VD_GetVoiceFrameNum(VD_HANDLE hVad) {
	VoiceDetector *pVad = (VoiceDetector*) hVad;
	if (pVad)
		return pVad->GetVoiceFrameNum();
	else
		return 0;
}

const short * NNVAPI VD_GetVoice(VD_HANDLE hVad) {
	VoiceDetector *pVad = (VoiceDetector*) hVad;
	if (pVad)
		return pVad->GetVoice();
	else
		return 0;
}

const float * NNVAPI VD_GetFloatVoice(VD_HANDLE hVad) {
	VoiceDetector *pVad = (VoiceDetector*) hVad;
	if (pVad)
		return pVad->GetFloatVoice();
	else
		return 0;
}

float NNVAPI VD_GetLastFrameEnergy(NNV_HANDLE hVad) {
    VoiceDetector *pVad = (VoiceDetector*) hVad;
    if (pVad) {
        return pVad->GetLastEnergy();
    } else
        return 0;
}

float NNVAPI VD_GetThresholdEnergy(NNV_HANDLE hVad) {
    VoiceDetector *pVad = (VoiceDetector*) hVad;
    if (pVad) {
        return pVad->GetThresholdEnergy();
    } else
        return 0;
}

int NNVAPI VD_Freeze(NNV_HANDLE hVad, int bFreeze, int nSampleNum) {
    VoiceDetector *pVad = (VoiceDetector*) hVad;
    if (pVad) {
        pVad->Freeze(bFreeze, nSampleNum);
        return 1;
    } else
        return 0;
}


