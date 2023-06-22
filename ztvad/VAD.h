/*
 * VAD.h
 *
 *  Created on: 2013-12-12
 *      Author: gaopeng
 */

#ifndef VAD_H_
#define VAD_H_

#include <vector>

#include "felib/PLP.h"
#include "felib/feintf.h"
#include "amlib/dnn.h"
#include "felib/fftwrapper.h"
#include "ByteBuffer.h"
#include "webrtc/common_audio/vad/include/webrtc_vad.h"

/* VAD work mode */
#define VAD_MODE_DNN			0
#define VAD_MODE_ENERGY			1
#define VAD_MODE_DNNENERGY		2
#define VAD_MODE_WEBRTC			3

#define VAD_OUT_DIM		2
#define	VAD_DNN_LAYER	3

class VAD {
public:
	VAD(int sampleRate, int frameLenMs);
	virtual ~VAD();

	void SetMode(int nMode) {
		m_nMode = nMode;
		if (m_nMode == VAD_MODE_DNN || m_nMode == VAD_MODE_DNNENERGY) {
			PrepareNNMode();
		} else if (m_nMode == VAD_MODE_WEBRTC) {
			PrepareWebrtcMode();
		}
	}

	void Reset();
    void Restart();
    void Freeze(bool bFreeze, int nIsAec);
    void FreezeSpeech(bool bFreeze, int nFrameNum);
	bool InputWave(const short *pWaveData, int frameNum, bool isEnd, int nIsAec);
	bool InputFloatWave(float *pWaveData, int frameNum, bool isEnd, int nIsAec);
    int OffsetFrame(int frameNum);
	int GetMaxFrame() {
		if (m_nMode == VAD_MODE_DNN)
			return m_nCurMaxFrame;
		else if (m_nMode == VAD_MODE_ENERGY)
			return m_nCurMaxFrame2;
		else if (m_nMode == VAD_MODE_DNNENERGY)
			return (m_nCurMaxFrame > m_nCurMaxFrame2 ?
				m_nCurMaxFrame2 : m_nCurMaxFrame);
		if (m_nMode == VAD_MODE_WEBRTC)
			return m_nCurMaxFrame;
		else
			return m_nCurMaxFrame;
	}
	float GetFrameProb(int frame, int dim);
    float GetLastEnergy() { return m_fEnergyProb; }
    float GetThresholdEnergy() { return m_fThresholdEnergy; }

	int GetSampleRate() {
		return m_nSampleRate;
	}
	int GetFrameLenMs() {
		return m_nFrameLenMs;
	}

	void SetEstiFrameNum(int nNum) {
		//m_nEstiFrameNum = nNum;
	}

    void SetCtxFrameNum(int nNum) {
        m_nCtxFrameNum = nNum;
    }
    
	void SetMinEnergyThreshold(int threshold) {
		m_fMinEnergy = threshold;
	}

	void SetMaxEnergyThreshold(int threshold) {
		m_fMaxEnergy = threshold;
	}
    void SetBaseRange(float fParam) {
        m_fBaseRange = fParam;
    }
    void SetMinDynaRange(float fParam) {
        m_fMinDynaRange = fParam;
    }
    void SetMaxDynaRange(float fParam) {
        m_fMaxDynaRange = fParam;
    }
    void SetMinAecEnergy(float fParam) {
        m_fMinAecEnergy = fParam;
    }
    void SetMinDynaEnergy(float fParam) {
        m_fMinDynaEnergy = fParam;
    }

	void SetWebrtcMode(int nMode) {
		if (m_pWebrtcVad)
			WebRtcVad_set_mode(m_pWebrtcVad, nMode);
	}

protected:
	int m_nSampleRate;
	int m_nFrameLenMs;
	int m_nFrameSampleNum;

	FE_HDR m_feHdr;
	HANDLE m_hFE;
	CDNN2 m_dnn;

	float m_frameProbs[MAX_SPEECH * VAD_OUT_DIM];
	int m_nCurFeMaxFrame;
	int m_nCurMaxFrame;
	float m_frameProbs2[MAX_SPEECH * VAD_OUT_DIM];
	int m_nCurFeMaxFrame2;
	int m_nCurMaxFrame2;
    int m_nOffsetFrameNum;

	int m_nMode; /* 0 dnn, 1 energy */

	bool PrepareNNMode();

	/* energy vad */
    int m_nCtxFrameNum;
    bool m_bFreeze; // stop update during voice frames
    float m_fPrevEnergy; // previous frame energy, for smoothing
    float m_fSmoothFactor; // energy smoothing factor
    std::vector<float> m_ctxEnergies;
    std::vector<float> m_ctxVoiceEnergies;

    bool m_bFreezeSpeech; // stop update when activated by keywords
    float m_fFreezeThreshold; // using this fixed threshold
    
	float m_fMinEnergy;
    float m_fMinAecEnergy;
    float m_fMaxEnergy;
    float m_fBaseRange;
    float m_fMinDynaRange;
    float m_fMaxDynaRange;
    float m_fMinDynaEnergy;
	float m_fThresholdEnergy;
    float m_fVoiceAvgEnergy;
	bool DoEnergyVad(float *pWaveData, int nFrameNum, bool bIsEnd, int nIsAec);
	float GetFrameEnergy(float *pFrameData);
	float CalcFrameEnergyProb(float energy);
    float m_fEnergyProb;
    
    FFTWrapper  *m_pFft;
    ByteBuffer  m_bfFft;
    float   *m_pHamWin;
    float   *m_pFrameReal;
    float   *m_pFrameImg;

    /* webrtc vad */
	VadInst *m_pWebrtcVad;
	bool PrepareWebrtcMode();
	bool DoWebrtcVad(float *pWaveData, int nFrameNum, bool bIsEnd);
};

#endif /* VAD_H_ */
