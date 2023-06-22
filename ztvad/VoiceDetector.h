#ifndef _VOICE_DETECTOR_H_
#define _VOICE_DETECTOR_H_

#include <blis/cblas.h>
#include "NNVadIntf.h"
#include "ByteBuffer.h"
#include "r2ssp.h"

#include "feat/pitch-functions.h"
using namespace kaldi;


class VoiceDetector {
public:
	VoiceDetector(int nMode);
	~VoiceDetector(void);

	void Reset();
	void Restart();

	int InputWave(const short *pWaveData, int nSampleNum, bool bIsEnd, int nIsAec);
	int InputFloatWave(const float *pWaveData, int nSampleNum, bool bIsEnd, int nIsAec);
    int SetStart(int nIsAec); // force vad start
	int GetOffsetFrame() {
		return m_nOffsetFrame;
	}
	int GetVoiceStartFrame() {
		if (m_nVocStartFrame >= 0)
			return m_nVocStartFrame + m_nOffsetFrame;
		else
			return -1;
	}
	int GetVoiceStopFrame() {
		if (m_nVocStopFrame >= 0)
			return m_nVocStopFrame + m_nOffsetFrame;
		else
			return -1;
	}
	int GetVoiceFrameNum();
	const short *GetVoice();
	const float *GetFloatVoice();
    float GetLastEnergy() { return NNV_GetLastFrameEnergy(m_hVad); }
    float GetThresholdEnergy() { return NNV_GetThresholdEnergy(m_hVad); }

	void SetPreFrameNum(int nNum) {
		m_nPreFrameNum = nNum;
	}
	void SetMinVocFrameNum(int nNum) {
		m_nMinVocFrameNum = nNum;
	}
	void SetMinVocRatio(float fRatio) {
		m_fMinVoiceRatio = fRatio;
	}
    void SetMinSpeechFrameNum(int nNum) {
        m_nMinSpeechFrameNum = nNum;
    }
    void SetMaxSpeechFrameNum(int nNum) {
        m_nMaxSpeechFrameNum = nNum;
    }
	void SetMinSilFrameNum(int nNum) {
		m_nMinSilFrameNum = nNum;
	}
	void SetMinSilRatio(float fRatio) {
		m_fMinSilRatio = fRatio;
	}
    void EnablePitchDetection(bool bEnable) {
        m_bEnablePitch = bEnable;
    }

	int SetFrameVadParam(int nParam, void *pVal) {
		return NNV_SetVadParam(m_hVad, nParam, pVal);
	}
	/*void SetEstiFrameNum(int nNum) {
	 NNV_SetVadParam(m_hVad, VD_PARAM_ESTIFRAMENUM, &nNum);
	 }
	 void SetMinEnergyThreshold(int threshold) {
	 NNV_SetVadParam(m_hVad, VD_PARAM_MINENERGY, &threshold);
	 }
	 void SetMaxEnergyThreshold(int threshold) {
	 NNV_SetVadParam(m_hVad, VD_PARAM_MAXENERGY, &threshold);
	 }*/
    void Freeze(bool bFreeze, int nSampleNum);

private:
	NNV_HANDLE m_hVad;

	int m_nPreFrameNum;
	int m_nMinVocFrameNum;
	float m_fMinVoiceRatio;
    int m_nMinSpeechFrameNum;
    int m_nMaxSpeechFrameNum;
	int m_nMinSilFrameNum;
	float m_fMinSilRatio;

	ByteBuffer m_bbWave;
	ByteBuffer m_bbFloatWave;

	int m_nCurProbFrameNum;
    float m_probs[VAD_MAX_FRAME_NUM][2];
    char m_bprobs[VAD_MAX_FRAME_NUM];
	int m_nVocStartFrame;
	int m_nVocStopFrame;
	int m_nOffsetFrame;
	int m_nVocStartOffset;
	bool m_bIsEnd;

	int InternalInputWave(const void *pWaveData, int isFloat, int nSampleNum,
			bool bIsEnd, int nIsAec);
	void JudgeVoiceBegin(int nIsAec);
	void JudgeVoiceEnd();
	void OffsetWaveFrame(int nFrame);

    // kaldi pitch feature
    bool m_bEnablePitch;
    int m_nPitchExtrRecreateFrameNum;
    float m_fPitchProbThreshold;
    int m_nMinBeginPitchFrameNum;
    int m_nMinEndPitchFrameNum;
    OnlinePitchFeature  *m_pPitchExtr;
    PitchExtractionOptions  m_pitchOpts;
    int m_nCurPitchProbFrameNum;
    float m_pitchprobs[VAD_MAX_FRAME_NUM];
    r2ssp_handle    m_hBpf; // band-pass filter, remove low-freq part
    void RecreatePitchExtractor();
    void CalcPitchFeature(char *pWaveData, int nFrameNum, bool isFloat);
    bool JudgePitchVoiceBegin(int nCurFrameIdx);
    bool JudgePitchVoiceEnd(int nCurFrameIdx);
};

#endif // _VOICE_DETECTOR_H_
