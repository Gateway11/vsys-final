/*
 * VAD.cpp
 *
 *  Created on: 2013-12-12
 *      Author: gaopeng
 */

//#include <Accelerate/Accelerate.h>

//#define LOG_TAG "VAD"
//#include <android/log.h>
//#undef LOGD
//#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#include "VAD.h"

inline float fast_log2(float val) {
	int * const exp_ptr = reinterpret_cast<int *>(&val);
	int x = *exp_ptr;
	const int log_2 = ((x >> 23) & 255) - 128;
	x &= ~(255 << 23);
	x += 127 << 23;
	*exp_ptr = x;

	val = ((-1.0f / 3) * val + 2) * val - 2.0f / 3;

	return (val + log_2);
}

inline float fast_log(const float &val) {
	return (fast_log2(val) * 0.69314718f);
}

#define USE_QUICK_EXP

//// FAST EXP APPROXIMATION
#ifdef USE_QUICK_EXP
#define QN_EXPQ_WORKSPACE union { double d; struct { int j,i;} n; } qn_d2i; qn_d2i.d = 0.0;

#ifndef M_LN2
#define M_LN2      0.693147180559945309417
#endif

#define EXP_A (1048576/M_LN2)
#define EXP_C 60801
#define FAST_EXP(y) (qn_d2i.n.i = EXP_A*(y)+(1072693248-EXP_C),qn_d2i.d)
#define QN_EXPQ(y) (qn_d2i.n.i = (int) (EXP_A*(y)) + (1072693248 - EXP_C), (y > -700.0f && y < 700.0f) ? qn_d2i.d : exp(y) )
#endif

/*static float g_fMinProb = 0.001f;
static float g_fMaxProb = 0.999f;
static float g_fMinLogProb = -6.907755279f;
static float g_fMaxLogProb = -0.001000500f;*/

static float g_fMinProb = 0.01f;
static float g_fMaxProb = 0.99f;
static float g_fMinLogProb = -4.605170186f;
static float g_fMaxLogProb = -0.010050336f;

static inline float sigmoid(float x)
{
#ifdef USE_QUICK_EXP
    QN_EXPQ_WORKSPACE
    return 1.0f / (1+QN_EXPQ(-x));
#else
    return 1.0f / (1+exp(-x));
#endif
}

VAD::VAD(int sampleRate, int frameLenMs) {
	m_nSampleRate = sampleRate;
	m_nFrameLenMs = frameLenMs;
	m_nFrameSampleNum = frameLenMs * m_nSampleRate / 1000;

	m_hFE = 0;

	// default VAD work mode
	//m_nMode = VAD_MODE_DNN;
	m_nMode = VAD_MODE_ENERGY;

	if (m_nMode == VAD_MODE_DNN || m_nMode == VAD_MODE_DNNENERGY) {
		PrepareNNMode();
	} else if (m_nMode == VAD_MODE_WEBRTC) {
		PrepareWebrtcMode();
	}

    m_fMinEnergy = 500;// * 500; // 500 is for normal 16bit
    m_fMinAecEnergy = 4500; // assume louder voice during playing music
    m_fMaxEnergy = 20000.f;// * 20000;
    m_fBaseRange = 1.25f;// * 1.25f;
    m_fMinDynaRange = 2.5f;// * 2.5f;
    m_fMaxDynaRange = 4.0f;// * 4.0f;
    m_fMinDynaEnergy = 1000;

    m_nCtxFrameNum = 300+30;
    m_fSmoothFactor = 0.5f;
    
    int nFftLen = 2;
    while (nFftLen < m_nFrameSampleNum)
        nFftLen *= 2;
    m_pFft = new FFTWrapper(nFftLen);
    m_bfFft.Init(nFftLen*sizeof(float));
    m_pFrameReal = new float[nFftLen];
    m_pFrameImg = new float[nFftLen];
    
    // init Hamming window
    m_pHamWin = new float[nFftLen];
    float a = 3.1415927f*2/(nFftLen - 1);
    for (int i=0; i<nFftLen; i++)
        m_pHamWin[i] = 0.54 - 0.46 * cos(a*i);

	m_pWebrtcVad = 0;

	Reset();
}

VAD::~VAD() {
	if (m_hFE)
		CloseFE(m_hFE);
    
    if (m_pFft) {
        delete m_pFft;
        m_pFft = 0;
    }
    if (m_pFrameReal) {
        delete[] m_pFrameReal;
        m_pFrameReal = 0;
    }
    if (m_pFrameImg) {
        delete[] m_pFrameImg;
        m_pFrameImg = 0;
    }
    if (m_pHamWin) {
        delete[] m_pHamWin;
        m_pHamWin = 0;
    }

    if (m_pWebrtcVad) {
		WebRtcVad_Free(m_pWebrtcVad);
        m_pWebrtcVad = 0;
    }
}

bool VAD::PrepareNNMode() {
	if (m_hFE)
		return true;

	//LOGD("PrepareNNMode\n");
	char dir[] = "";
	m_hFE = OpenFE(dir, m_feHdr);
	if (!m_hFE)
		return false;
	return m_dnn.load_model_cfile(VAD_DNN_LAYER);
}

bool VAD::PrepareWebrtcMode() {
	if (m_pWebrtcVad)
		return true;
	if (WebRtcVad_Create(&m_pWebrtcVad))
		return false;
	if (WebRtcVad_Init(m_pWebrtcVad)) {
		WebRtcVad_Free(m_pWebrtcVad);
		m_pWebrtcVad = 0;
		return false;
	}
	return true;
}

void VAD::Restart() {
	if (m_hFE)
		ResetFE(m_hFE);
    m_nOffsetFrameNum = 0;
	m_nCurFeMaxFrame = 0;
	m_nCurMaxFrame = 0;
	m_nCurFeMaxFrame2 = 0;
	m_nCurMaxFrame2 = 0;
	m_bFreeze = false;
}

void VAD::Reset() {
	Restart();

    m_fPrevEnergy = 0.0f;
    m_ctxEnergies.clear();
    m_ctxVoiceEnergies.clear();
    m_fThresholdEnergy = m_fMinEnergy;
    m_fVoiceAvgEnergy = 0;
    memset(m_bfFft.GetBytes(), 0, m_bfFft.GetBufSize());
    m_bfFft.SetByteNum(m_bfFft.GetBufSize());
    m_bFreezeSpeech = false;
}

bool VAD::InputWave(const short *pWaveData, int frameNum, bool isEnd, int nIsAec) {
	if (!pWaveData || frameNum <= 0)
		return false;
	float *pFloatData = new float[frameNum * m_nFrameSampleNum];
	if (!pFloatData)
		return false;
	for (int i = 0; i < frameNum * m_nFrameSampleNum; i++)
		pFloatData[i] = pWaveData[i];

	bool bRet = InputFloatWave(pFloatData, frameNum, isEnd, nIsAec);
	delete[] pFloatData;
	return bRet;
}

bool VAD::InputFloatWave(float *pWaveData, int frameNum, bool isEnd, int nIsAec) {
	//LOGD("input frame %d isEnd %d", frameNum, isEnd);
	/*if (m_nCurFeMaxFrame + frameNum > MAX_SPEECH) {
		frameNum = MAX_SPEECH - m_nCurFeMaxFrame;
		isEnd = true;
	}*/

    if (frameNum < 0)
        return false;

	bool bRet = false;
	if (m_nMode == VAD_MODE_ENERGY || m_nMode == VAD_MODE_DNNENERGY) {
		bRet = DoEnergyVad(pWaveData, frameNum, isEnd, nIsAec);
		if (m_nMode == VAD_MODE_ENERGY)
			return bRet;
	} else if (m_nMode == VAD_MODE_WEBRTC)
		return DoWebrtcVad(pWaveData, frameNum, isEnd);

	if (!m_hFE)
		return false;

	if (pWaveData && frameNum > 0)
		ContinueFE(m_hFE, pWaveData, m_nFrameSampleNum * frameNum);

	float * pFloatFeature = 0;
	int * pIntSNR = 0, nFrm = 0;
	if (FE_OK == NormDiffFE(m_hFE, pFloatFeature, pIntSNR, nFrm, isEnd)) {
		//LOGD("feature frame: %d\n", nFrm);
		if (nFrm > m_nCurMaxFrame) {
			int nNewFrm = nFrm - m_nCurMaxFrame;
			float *pFeature = pFloatFeature + m_nCurMaxFrame * ALN_FUL_DIM;
			// clear pitch feature dim when using plp feature
			/*if (CEP_DIM != VEC_DIM) {
				for (int i = 0; i < nNewFrm; i++) {
					float *pCurFea = pFeature + i * ALN_FUL_DIM;
					for (int j = 0; j < CONTEXT_SPAN; j++) {
						float *pCurVec = pCurFea + j * (VEC_DIM * 3);
						pCurVec[VEC_DIM - 1] = 0;
						pCurVec[VEC_DIM * 2 - 1] = 0;
						pCurVec[VEC_DIM * 3 - 1] = 0;
					}
				}
			}*/
			int nOutDim = VAD_OUT_DIM;
			float *pOutFeature = m_frameProbs + (m_nCurMaxFrame-m_nOffsetFrameNum) * VAD_OUT_DIM;
			m_dnn.get_batch_output_batch(pFeature, nNewFrm, ALN_FUL_DIM,
					nOutDim, pOutFeature, SOFTMAX);
			for (int i = 0; i < nNewFrm * VAD_OUT_DIM; i++) {
				float &prob = pOutFeature[i];
				if (prob > g_fMaxLogProb)
					prob = g_fMaxLogProb;
				else if (prob < g_fMinLogProb)
					prob = g_fMinLogProb;
			}
			//for (int i=0; i<nNewFrm; i++) {
			//	LOGD("vad out of frame %d : %f\t%f\n", m_nCurMaxFrame+i,
			//			pOutFeature[i*VAD_OUT_DIM+0], pOutFeature[i*VAD_OUT_DIM+1]);
			//}
			m_nCurMaxFrame += nNewFrm;
		}
	}
	m_nCurFeMaxFrame += frameNum;

	return true;
}

int VAD::OffsetFrame(int frameNum) {
    if (m_nCurMaxFrame-m_nOffsetFrameNum >= frameNum) {
        memmove(m_frameProbs, m_frameProbs+frameNum*VAD_OUT_DIM,
                (m_nCurMaxFrame-m_nOffsetFrameNum-frameNum)*sizeof(float)*VAD_OUT_DIM);
    }
    if (m_nCurMaxFrame2-m_nOffsetFrameNum >= frameNum) {
        memmove(m_frameProbs2, m_frameProbs2+frameNum*VAD_OUT_DIM,
                (m_nCurMaxFrame2-m_nOffsetFrameNum-frameNum)*sizeof(float)*VAD_OUT_DIM);
    }
    m_nOffsetFrameNum += frameNum;
    return 0;
}

bool VAD::DoEnergyVad(float *pWaveData, int nFrameNum, bool bIsEnd, int nIsAec) {
    if (!pWaveData || nFrameNum < 0)
        return false;

	for (int i = 0; i < nFrameNum; i++) {
		float energy = GetFrameEnergy(pWaveData + m_nFrameSampleNum * i);

        if (nIsAec) {
            if (m_fThresholdEnergy < m_fMinAecEnergy)
                m_fThresholdEnergy = m_fMinAecEnergy;
        }
        else { // update threshold under normal status (no aec or other ns process)
            // smoothing
            energy = m_fSmoothFactor * energy + (1-m_fSmoothFactor)*m_fPrevEnergy;
            m_fPrevEnergy = energy;
            //LOGD("frame energy: %u\n", energy);
            //printf("frame energy: %f\n", energy);

            /*if (m_nEstiFrameCount < m_nCtxFrameNum) {
             m_nEstiFrameCount++;
             m_fThresholdEnergy *= (m_nEstiFrameCount - 1.0f)
             / m_nEstiFrameCount;
             m_fThresholdEnergy += energy / m_nEstiFrameCount;
             } else if (!m_bFreeze) {
             m_fThresholdEnergy *= (m_nCtxFrameNum - 1.0f) / m_nCtxFrameNum;
             m_fThresholdEnergy += energy / m_nCtxFrameNum;
             }*/

            m_ctxVoiceEnergies.push_back(energy);
            if (m_ctxVoiceEnergies.size() > m_nCtxFrameNum)
                m_ctxVoiceEnergies.erase(m_ctxVoiceEnergies.begin());
            /*else if (m_ctxVoiceEnergies.size() > 50 && m_ctxVoiceEnergies.size() < 200) {
             while (m_ctxVoiceEnergies.size() < 200)
             m_ctxVoiceEnergies.push_back(m_fVoiceAvgEnergy);
             }*/

            // over all average energy
            m_fVoiceAvgEnergy = m_fMinEnergy;
            for (int i=0; i<m_ctxVoiceEnergies.size(); i++)
                m_fVoiceAvgEnergy += m_ctxVoiceEnergies[i];
            if (m_ctxVoiceEnergies.size() > 0)
                m_fVoiceAvgEnergy /= m_ctxVoiceEnergies.size();

            // silence average energy
            if (!m_bFreeze) { // in silence mode
                if (energy >= m_fMinEnergy && energy <= m_fMaxEnergy)
                    m_ctxEnergies.push_back(energy);
                if (m_ctxEnergies.size() > m_nCtxFrameNum)
                    m_ctxEnergies.erase(m_ctxEnergies.begin());

                if (m_ctxEnergies.size() > 0) {
                    m_fThresholdEnergy = 0;
                    for (int i=0; i<m_ctxEnergies.size(); i++)
                        m_fThresholdEnergy += m_ctxEnergies[i];
                    m_fThresholdEnergy /= m_ctxEnergies.size();

                    if(m_fThresholdEnergy > m_fVoiceAvgEnergy)
                        m_fThresholdEnergy = m_fVoiceAvgEnergy;

                    if (m_fThresholdEnergy < m_fMinEnergy)
                        m_fThresholdEnergy = m_fMinEnergy;
                    else if (m_fThresholdEnergy > m_fMaxEnergy)
                        m_fThresholdEnergy = m_fMaxEnergy;
                }
            }
            
            if (m_fVoiceAvgEnergy < m_fThresholdEnergy)
                m_fVoiceAvgEnergy = m_fThresholdEnergy;
        }

		//printf("frame energy: %f\tthreshold: %f\t\n", energy, m_fThresholdEnergy);
		//printf("%f\t%f\t%f\n", energy, m_fThresholdEnergy, m_fVoiceAvgEnergy);
        //printf("%f\t%f\t%f\n", sqrt(energy), sqrt(m_fThresholdEnergy), sqrt(m_fVoiceAvgEnergy));

        float prob = 0;
		/*if (m_nCurFeMaxFrame2 < m_nEstiFrameNum) {
			m_frameProbs2[m_nCurFeMaxFrame2 * VAD_OUT_DIM] = energy;
		} else {
			if (m_nCurFeMaxFrame2 == m_nEstiFrameNum) {
				for (int j = 0; j < m_nEstiFrameNum; j++) {
                    prob = CalcFrameEnergyProb(
							(unsigned) m_frameProbs2[j * VAD_OUT_DIM]);
					m_frameProbs2[j * VAD_OUT_DIM] = (fast_log(1 - prob));
					m_frameProbs2[j * VAD_OUT_DIM + 1] = (fast_log(prob));
				}
				m_nCurMaxFrame2 = m_nCurFeMaxFrame2;
			}
            prob = CalcFrameEnergyProb(energy);
			m_frameProbs2[(m_nCurMaxFrame2-m_nOffsetFrameNum) * VAD_OUT_DIM] = (fast_log(1 - prob));
			m_frameProbs2[(m_nCurMaxFrame2-m_nOffsetFrameNum) * VAD_OUT_DIM + 1] = (fast_log(prob));
			m_nCurMaxFrame2++;
		}*/

        prob = CalcFrameEnergyProb(energy);
        m_fEnergyProb = prob;
        m_frameProbs2[(m_nCurMaxFrame2-m_nOffsetFrameNum) * VAD_OUT_DIM] = (fast_log(1 - prob));
        m_frameProbs2[(m_nCurMaxFrame2-m_nOffsetFrameNum) * VAD_OUT_DIM + 1] = (fast_log(prob));
        m_nCurMaxFrame2++;
        //printf("\t%f\n", prob*1000);
		m_nCurFeMaxFrame2++;
	}
	return true;
}

float VAD::CalcFrameEnergyProb(float energy) {
	float prob = 0.f;
    float fThresholdEnergy = m_fThresholdEnergy;
    if (m_bFreezeSpeech)
        fThresholdEnergy = m_fFreezeThreshold;
    
	/*float max = m_fThresholdEnergy * 2.f;
    float min = m_fThresholdEnergy / 2.f;
    if (energy >= max)
        prob = 1.f;
    else if (energy >= m_fThresholdEnergy)
        prob = 0.5f
        + (energy - m_fThresholdEnergy) * 0.5f
        / (max - m_fThresholdEnergy);
    else if (energy <= min)
        prob = 0.f;
    else
        prob = (energy - min) * 0.5f / (m_fThresholdEnergy - min);*/

    float fDynaRange = m_fMinDynaRange;
    if (m_fVoiceAvgEnergy > 0) {
        fDynaRange = m_fVoiceAvgEnergy / fThresholdEnergy;
        if (fDynaRange < m_fMinDynaRange)
            fDynaRange = m_fMinDynaRange;
        else if (fDynaRange > m_fMaxDynaRange)
            fDynaRange = m_fMaxDynaRange;
    }
	float max = fThresholdEnergy * fDynaRange;
	float min = fThresholdEnergy * m_fBaseRange;
    if (max - min < m_fMinDynaEnergy)
        max = min + m_fMinDynaEnergy;
	if (energy >= max)
		prob = 1.f;
	else if (energy <= min)
		prob = 0.f;
	else
		prob = (energy - min) / (max - min);

	//LOGD("frame energy prob: %f\n", prob);
    
	if (prob > g_fMaxProb)
		prob = g_fMaxProb;
	else if (prob < g_fMinProb)
		prob = g_fMinProb;
	return prob;
}

float VAD::GetFrameEnergy(float *pFrameData) {
    // remove dc
	//float avg = 0.f;
	//for (int i = 0; i < m_nFrameSampleNum; i++) {
	//	avg += pFrameData[i];
	//}
	//avg /= m_nFrameSampleNum;

	float energy = 0.f;
	
    // time domain, all energy
    /*for (int i = 0; i < m_nFrameSampleNum; i++) {
		float sample = pFrameData[i] - avg;
		energy += sample * sample;
	}
	return sqrt(energy / m_nFrameSampleNum);*/
    
    // frequency domain
    // prepare fft data
    int nFeedLen = m_nFrameSampleNum*sizeof(float);
    int nFftSize = m_bfFft.GetBufSize() / sizeof(float);
    m_bfFft.Shift(nFeedLen);
    m_bfFft.Feed((const char*)pFrameData, nFeedLen);
    memcpy(m_pFrameReal, m_bfFft.GetBytes(), m_bfFft.GetBufSize());
    memset(m_pFrameImg, 0, nFftSize*sizeof(float));

    // hamming window
    for (int i=0; i<nFftSize; i++)
        m_pFrameReal[i] *= m_pHamWin[i];

    // pre-emphasize & hamming window
    /*float fPreEm = 0.97f ;
    for (int j = 0; j < nFftSize ; j ++) {
        if (j == 0) {
            m_pFrameReal[j] = m_pFrameReal[j] * (1-fPreEm) * m_pHamWin[j] ;
        }
        else {
            m_pFrameReal[j] = (m_pFrameReal[j] -  fPreEm * m_pFrameReal[j-1]) * m_pHamWin[j] ;
        }
    }*/

    // fft
    m_pFft->Execute(m_pFrameReal, m_pFrameImg);

    // sub-band energy
    int nStartBin = 2;
    int nStopBin = (nFftSize/2) * 3/4; // use 125 ~ 6k (16khz sample rate)
    for (int i=nStartBin; i<nStopBin; i++)
        energy += m_pFrameReal[i]*m_pFrameReal[i] + m_pFrameImg[i]*m_pFrameImg[i];

    // use average power
    return sqrtf(energy / (nStopBin-nStartBin));
    // use average energy
    //return (energy / (nStopBin-nStartBin));
}

/*float VAD::GetFrameEnergy(float *pFrameData) {
    // remove dc
    float avg = 0.f;
    for (int i = 0; i < m_nFrameSampleNum; i++) {
        avg += pFrameData[i];
    }
    avg /= m_nFrameSampleNum;
    float energy = 0.f;

    // time domain, all energy
    //for (int i = 0; i < m_nFrameSampleNum; i++) {
    // float sample = pFrameData[i] - avg;
    // energy += sample * sample;
    // }
    // return sqrt(energy / m_nFrameSampleNum);

    // frequency domain
    // prepare fft data
    int nFeedLen = m_nFrameSampleNum*sizeof(float);
    int nFftSize = m_bfFft.GetBufSize() / sizeof(float);
    m_bfFft.Shift(nFeedLen);
    m_bfFft.Feed((const char*)pFrameData, nFeedLen);
    memcpy(m_pFrameReal, m_bfFft.GetBytes(), m_bfFft.GetBufSize());
    // hamming window
    for (int i=0; i<nFftSize; i++)
        m_pFrameReal[i] *= m_pHamWin[i];
    // fft
    memset(m_pFrameImg, 0, nFftSize*sizeof(float));
    m_pFft->Execute(m_pFrameReal, m_pFrameImg);
    // sub-band energy
    int nStartBin = 3;
    int nStopBin = (nFftSize/2) * 3/4; // use 125 ~ 6k (16khz sample rate)
    for (int i=nStartBin; i<nStopBin; i++)
        energy += m_pFrameReal[i]*m_pFrameReal[i] + m_pFrameImg[i]*m_pFrameImg[i];
    // use average power
    return sqrtf(energy / (nStopBin-nStartBin));
}*/

float VAD::GetFrameProb(int frame, int dim) {
    frame -= m_nOffsetFrameNum;
	if (frame < 0 || dim < 0 || dim >= VAD_OUT_DIM
			|| frame >= GetMaxFrame())
		return 0.0;
	if (m_nMode == VAD_MODE_DNN) {
		return m_frameProbs[frame * VAD_OUT_DIM + dim];
	} else if (m_nMode == VAD_MODE_ENERGY) {
		return m_frameProbs2[frame * VAD_OUT_DIM + dim];
	} else if (m_nMode == VAD_MODE_DNNENERGY) {
		//return m_frameProbs[frame * VAD_OUT_DIM + dim]
		//		+ m_frameProbs2[frame * VAD_OUT_DIM + dim];
		float &p1 = m_frameProbs[frame * VAD_OUT_DIM + dim];
		float &p2 = m_frameProbs2[frame * VAD_OUT_DIM + dim];
        QN_EXPQ_WORKSPACE;
        float pp1 = FAST_EXP(p1); // dnn prob
        float pp2 = FAST_EXP(p2); // energy prob
        float pp12 = fast_log(pp1*0.3f+pp2*0.7f);
        //if (dim) printf("%f\t%f\n", pp1, pp2);
        return pp12;
        /*if (dim) // voice prob
            return (p1<p2?p1:p2);
        else // silence prob
            return (p1>p2?p1:p2);*/
	} else if (m_nMode == VAD_MODE_WEBRTC) {
		return m_frameProbs[frame * VAD_OUT_DIM + dim];
	} else {
		return m_frameProbs[frame * VAD_OUT_DIM + dim];
	}
}

void VAD::Freeze(bool bFreeze, int nIsAec) {
    m_bFreeze = bFreeze;

    if (bFreeze && !nIsAec) { // vad start, remove head voices already in context
        size_t nEraseVoiceHead = 30; // frames
        if (nEraseVoiceHead > m_ctxEnergies.size())
            nEraseVoiceHead = m_ctxEnergies.size();
        m_ctxEnergies.erase(m_ctxEnergies.end()-nEraseVoiceHead, m_ctxEnergies.end());

        // recalculate threshold energy
        if (m_ctxEnergies.size() > 0) {
            m_fThresholdEnergy = 0;
            for (int i=0; i<m_ctxEnergies.size(); i++)
                m_fThresholdEnergy += m_ctxEnergies[i];
            m_fThresholdEnergy /= m_ctxEnergies.size();

            if(m_fThresholdEnergy > m_fVoiceAvgEnergy)
                m_fThresholdEnergy = m_fVoiceAvgEnergy;
            if (m_fThresholdEnergy < m_fMinEnergy)
                m_fThresholdEnergy = m_fMinEnergy;
            else if (m_fThresholdEnergy > m_fMaxEnergy)
                m_fThresholdEnergy = m_fMaxEnergy;
        }
    }

}

void VAD::FreezeSpeech(bool bFreeze, int nFrameNum) {
    m_bFreezeSpeech = bFreeze;
    if (bFreeze) {
        if (nFrameNum > 0 && nFrameNum < m_ctxVoiceEnergies.size()) {
            m_fFreezeThreshold = 0;
            int i= (int)m_ctxVoiceEnergies.size() - nFrameNum;
            for (; i<m_ctxVoiceEnergies.size(); i++)
                m_fFreezeThreshold += m_ctxVoiceEnergies[i];
            m_fFreezeThreshold /= nFrameNum;
            m_fFreezeThreshold /= 3;
            if (m_fFreezeThreshold < m_fVoiceAvgEnergy)
                m_fFreezeThreshold = m_fVoiceAvgEnergy;
        }
        else {
            m_fFreezeThreshold = m_fVoiceAvgEnergy;
        }
    }
}


bool VAD::DoWebrtcVad(float *pWaveData, int nFrameNum, bool bIsEnd) {
	if (!pWaveData || nFrameNum < 0)
		return false;
	short *pShortData = new short[nFrameNum * m_nFrameSampleNum];
	if (!pShortData)
		return false;
	for (int i = 0; i < nFrameNum * m_nFrameSampleNum; i++)
		pShortData[i] = pWaveData[i];

	bool bRet = true;
	for (int i = 0; i < nFrameNum; i++) {
		int ret = WebRtcVad_Process(m_pWebrtcVad, m_nSampleRate,
				pShortData + i * m_nFrameSampleNum, m_nFrameSampleNum);
		if (ret == -1) {
			bRet = false;
			break;
		}
		float prob;
		if (ret == 1)
			prob = g_fMaxProb;
		else
			prob = g_fMinProb;
		m_frameProbs[(m_nCurMaxFrame-m_nOffsetFrameNum) * VAD_OUT_DIM] = (fast_log(1 - prob));
		m_frameProbs[(m_nCurMaxFrame-m_nOffsetFrameNum) * VAD_OUT_DIM + 1] = (fast_log(prob));
		//LOGD("%d %d\n", m_nCurMaxFrame, ret);
		m_nCurMaxFrame++;
		m_nCurFeMaxFrame++;
	}
	delete[] pShortData;

	return bRet;
}
