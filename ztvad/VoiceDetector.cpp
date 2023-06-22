#include <stdio.h>
#include "VoiceDetector.h"


//#define LOG_TAG "vad_jni"
//#include <android/log.h>
//#undef LOGD
//#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)

VoiceDetector::VoiceDetector(int nMode) {
	m_hVad = NNV_NewVad(VAD_SAMPLE_RATE, VAD_FRAME_LENMS, nMode);
	//m_bbWave.Init(VAD_SAMPLE_RATE * sizeof(short) * VAD_MAX_LENGTH_SEC);
	//m_bbFloatWave.Init(VAD_SAMPLE_RATE * sizeof(float) * VAD_MAX_LENGTH_SEC);

	m_nPreFrameNum = 10;
	m_nMinVocFrameNum = 30; // change the value in vad.cpp:freeze
	m_fMinVoiceRatio = 0.5f;
    m_nMinSpeechFrameNum = 90; // 0.9 seconds
    m_nMaxSpeechFrameNum = 100*10; // 10 seconds
	m_nMinSilFrameNum = 45;
	m_fMinSilRatio = 0.9f;

    // pitch verification parameters
    m_bEnablePitch = true;
    m_nPitchExtrRecreateFrameNum = 100 * 60; // reset pitch extractor every 60 seconds
    m_fPitchProbThreshold = 0.25f;
    m_nMinBeginPitchFrameNum = 4;
    m_nMinEndPitchFrameNum = 0;
    m_pPitchExtr = 0;
    m_nCurPitchProbFrameNum = 0;

    // init band-pass filter for pitch extraction
    if (m_bEnablePitch) {
        m_hBpf = r2ssp_bpf_create();
        // sub-band energy
        int nFftSize = 256; // 10ms frame length (160 samples)
        int nStartBin = 2;
        int nStopBin = (nFftSize/2) * 3/4; // use 64 ~ 6k (16khz sample rate)
        int bBandPass = 1;
        r2ssp_bpf_init(m_hBpf, VAD_FRAME_LENMS, VAD_SAMPLE_RATE, nStartBin, nStopBin, bBandPass);

        static const char* argv[] = {
            "command",
            "--frames-per-chunk=1",
            "--min-f0=64",
            "--delta-pitch=0.07",
            "--print-args=false",
        };
        ParseOptions po("");
        m_pitchOpts.Register(&po);
        po.Read(sizeof(argv)/sizeof(char*), argv);
    }

	Reset();
}

VoiceDetector::~VoiceDetector(void) {
    if (m_hVad) {
		NNV_DelVad(m_hVad);
        m_hVad = 0;
    }

    if (m_pPitchExtr) {
        delete m_pPitchExtr;
        m_pPitchExtr = 0;
    }

    if (m_hBpf) {
        r2ssp_bpf_free(m_hBpf);
        m_hBpf = 0;
    }
}

void VoiceDetector::Restart() {
    if (m_hVad) {
		NNV_RestartVad(m_hVad); // automatically un-freeze m_hVad
    }
	m_bbWave.Clear();
	m_bbFloatWave.Clear();
	m_nCurProbFrameNum = 0;
	m_nVocStartFrame = -1;
	m_nVocStopFrame = -1;
	m_nOffsetFrame = 0;
	m_nVocStartOffset = 0;
	m_bIsEnd = false;

    if (m_bEnablePitch) {
        RecreatePitchExtractor();
        m_nCurPitchProbFrameNum = 0;
    }
}

void VoiceDetector::Reset() {
	Restart();
	if (m_hVad)
		NNV_ResetVad(m_hVad);
}

int VoiceDetector::InputWave(const short *pWaveData, int nSampleNum,
		bool bIsEnd, int nIsAec) {
	return InternalInputWave(pWaveData, 0, nSampleNum, bIsEnd, nIsAec);
}

int VoiceDetector::InputFloatWave(const float *pWaveData, int nSampleNum,
		bool bIsEnd, int nIsAec) {
	return InternalInputWave(pWaveData, 1, nSampleNum, bIsEnd, nIsAec);
}

int VoiceDetector::InternalInputWave(const void *pWaveData, int isFloat,
		int nSampleNum, bool bIsEnd, int nIsAec) {
	ByteBuffer *pWaveBuf = 0;
	int nSampleSize = 2;
	if (!isFloat) {
		pWaveBuf = &m_bbWave;
		nSampleSize = sizeof(short);
	} else {
		pWaveBuf = &m_bbFloatWave;
		nSampleSize = sizeof(float);
	}
	// allocate wave space when first called
	if (!pWaveBuf->GetBytes()) {
		pWaveBuf->Init(VAD_SAMPLE_RATE * nSampleSize * VAD_MAX_LENGTH_SEC);
		if (!pWaveBuf->GetBytes())
			return 0;
	}
	if (m_bIsEnd)
		return 0;

	// save wave data in buffer
	int nPrevFrameNum = pWaveBuf->GetByteNum() / nSampleSize / VAD_FRAME_SIZE;
	int nWaveLen = nSampleNum * nSampleSize;
	if (pWaveData && nWaveLen > 0)
		pWaveBuf->Feed((const char*) pWaveData, nWaveLen);
	int nCurrFrameNum = pWaveBuf->GetByteNum() / nSampleSize / VAD_FRAME_SIZE;
	if (pWaveBuf->IsFull())
		bIsEnd = true;
    if (GetVoiceFrameNum() >= m_nMaxSpeechFrameNum)
        bIsEnd = true;
	m_bIsEnd = bIsEnd;

	// calculate voice probability of each wave frame
	int nNewFrameNum = nCurrFrameNum - nPrevFrameNum;
	char *pNewWave = pWaveBuf->GetBytes()
			+ nPrevFrameNum * VAD_FRAME_SIZE * nSampleSize;
	if (!isFloat)
		NNV_InputWave(m_hVad, pNewWave, nNewFrameNum, bIsEnd, nIsAec);
	else
		NNV_InputFloatWave(m_hVad, (float*) pNewWave, nNewFrameNum, bIsEnd, nIsAec);
    if (m_bEnablePitch)
        CalcPitchFeature(pNewWave, nNewFrameNum, isFloat);

	int nMaxProbFrame = NNV_GetMaxFrame(m_hVad) - m_nOffsetFrame;
	if (nMaxProbFrame > VAD_MAX_FRAME_NUM)
		nMaxProbFrame = VAD_MAX_FRAME_NUM;
	for (int i = m_nCurProbFrameNum; i < nMaxProbFrame; i++) {
		m_probs[i][0] = NNV_GetFrameProb(m_hVad, i+m_nOffsetFrame, 0);
		m_probs[i][1] = NNV_GetFrameProb(m_hVad, i+m_nOffsetFrame, 1);
        m_bprobs[i] = (m_probs[i][0] <= m_probs[i][1])?1:0;
        //printf("%f\t%f\n", m_probs[i][0], m_probs[i][1]);
	}
	m_nCurProbFrameNum = nMaxProbFrame;

	// judge voice begin
	if (m_nVocStartFrame == -1) {
		JudgeVoiceBegin(nIsAec);
        if (m_nVocStartFrame == -1) {
            int nSilFrames = m_nCurProbFrameNum - m_nMinVocFrameNum - m_nPreFrameNum;
            if (nSilFrames < 0)
                nSilFrames = 0;
            // too much silence frames, remove them
            if (nSilFrames >= VAD_MAX_FRAME_NUM / 2 && !m_bIsEnd) {
                OffsetWaveFrame(nSilFrames-100); // 100: for pitch recreation
            }
        }
	}

	// judge voice end
	if (m_nVocStartFrame >= 0 && m_nVocStopFrame == -1) {
		JudgeVoiceEnd();
		if (m_nVocStopFrame >= 0)
			m_bIsEnd = true;
		else if (bIsEnd)
			m_nVocStopFrame = m_nCurProbFrameNum;
	}
    
	return 0;
}

void VoiceDetector::JudgeVoiceBegin(int nIsAec) {
	if (m_nCurProbFrameNum <= m_nMinVocFrameNum)
		return;

	float fSilProb = 0;
	float fVocProb = 0;
	int nVoiceFrames = 0;
	int nVoiceFrameThreshold = m_nMinVocFrameNum * m_fMinVoiceRatio;
	for (int i=0; i < m_nCurProbFrameNum; i++) {
		if (i >= m_nMinVocFrameNum) {
			if (/*fVocProb >= fSilProb &&*/ nVoiceFrames >= nVoiceFrameThreshold
                    && m_bprobs[i] && (!m_bEnablePitch || JudgePitchVoiceBegin(i))) {
				// voice begin detected
				m_nVocStartFrame = i - m_nMinVocFrameNum - m_nPreFrameNum;
				m_nVocStartOffset = i;
				if (m_nVocStartFrame < 0) {
					m_nVocStartFrame = 0;
				}
				// remove preceding silence frames
				if (!m_bIsEnd && m_nVocStartFrame > 0) {
					OffsetWaveFrame(m_nVocStartFrame);
					m_nVocStartOffset -= m_nVocStartFrame;
					m_nVocStartFrame = 0;
				}
                NNV_Freeze(m_hVad, 1, nIsAec);
                //printf("find start: %f %f %d\n", fSilProb, fVocProb, nVoiceFrames);
				return;
			}

			fSilProb -= m_probs[i - m_nMinVocFrameNum][0];
			fVocProb -= m_probs[i - m_nMinVocFrameNum][1];
			if (m_bprobs[i - m_nMinVocFrameNum])
				nVoiceFrames--;
		}
		fSilProb += m_probs[i][0];
		fVocProb += m_probs[i][1];
		if (m_bprobs[i])
			nVoiceFrames++;
	}
    //printf("judge start: %f %f %d\n", fSilProb, fVocProb, nVoiceFrames);
}

int VoiceDetector::SetStart(int nIsAec) {
    if (m_nVocStartFrame >= 0)  // already started
        return -1;

    // voice begin detected
    m_nVocStartFrame = m_nCurProbFrameNum - 1;
    m_nVocStartOffset = m_nCurProbFrameNum - 1;
    if (m_nVocStartFrame < 0) {
        m_nVocStartFrame = 0;
    }
    // remove preceding silence frames
    if (!m_bIsEnd && m_nVocStartFrame > 0) {
        OffsetWaveFrame(m_nVocStartFrame);
        m_nVocStartOffset -= m_nVocStartFrame;
        m_nVocStartFrame = 0;
    }
    NNV_Freeze(m_hVad, 1, nIsAec);
    //printf("force start: %d\n", m_nCurProbFrameNum-1);
    return 0;
}

void VoiceDetector::OffsetWaveFrame(int nFrame) {
	//LOGD("OffsetWaveFrame %d isEnd %d", nFrame, m_bIsEnd);
    
    if (nFrame <= 0)
        return;
    if (nFrame > m_nCurProbFrameNum)
        return;

	ByteBuffer *pWaveBuf = 0;
	int nSampleSize = 2;
	int isFloat = 0;
	if (m_bbWave.GetByteNum()) {
		pWaveBuf = &m_bbWave;
		nSampleSize = sizeof(short);
	} else {
		pWaveBuf = &m_bbFloatWave;
		nSampleSize = sizeof(float);
		isFloat = 1;
	}
	pWaveBuf->Shift(VAD_FRAME_SIZE * nSampleSize * nFrame);

	/*
	 // in order to avoid FE module can not process too long voice
	 NNV_ResetVad(m_hVad);
	 int nFrameNum = pWaveBuf->GetByteNum() / nSampleSize / VAD_FRAME_SIZE;
	 char *pWave = pWaveBuf->GetBytes();
	 if (!isFloat)
	 NNV_InputWave(m_hVad, pWave, nFrameNum, m_bIsEnd);
	 else
	 NNV_InputFloatWave(m_hVad, (float*) pWave, nFrameNum, m_bIsEnd);
	 int nMaxProbFrame = NNV_GetMaxFrame(m_hVad);
	 if (nMaxProbFrame > VAD_MAX_FRAME_NUM)
	 nMaxProbFrame = VAD_MAX_FRAME_NUM;
	 for (int i = 0; i < nMaxProbFrame; i++) {
	 m_probs[i][0] = NNV_GetFrameProb(m_hVad, i, 0);
	 m_probs[i][1] = NNV_GetFrameProb(m_hVad, i, 1);
	 m_bprobs[i] = (m_probs[i][0] <= m_probs[i][1]);
	 }
	 m_nCurProbFrameNum = nMaxProbFrame;
	 */

    if (m_nCurProbFrameNum > nFrame) {
        memmove(m_probs, &m_probs[nFrame][0],
                (m_nCurProbFrameNum - nFrame) * 2 * sizeof(float));
        memmove(m_bprobs, &m_bprobs[nFrame],
                (m_nCurProbFrameNum - nFrame) * sizeof(char));
        m_nCurProbFrameNum -= nFrame;
    }
    else {
        m_nCurProbFrameNum = 0;
    }

    // shift pitch
    if (m_nCurPitchProbFrameNum > nFrame) {
        memmove(m_pitchprobs, &m_pitchprobs[nFrame],
                (m_nCurPitchProbFrameNum - nFrame) * sizeof(float));
        m_nCurPitchProbFrameNum -= nFrame;
    }
    else {
        m_nCurPitchProbFrameNum = 0;
    }

    NNV_OffsetFrame(m_hVad, nFrame);

	m_nOffsetFrame += nFrame;
}

void VoiceDetector::JudgeVoiceEnd() {
    if (m_nCurProbFrameNum < m_nVocStartFrame + m_nMinSpeechFrameNum)
        return;
	if (m_nCurProbFrameNum <= m_nVocStartOffset + m_nMinSilFrameNum)
		return;

	float fSilProb = 0;
	float fVocProb = 0;
	int nSilFrames = 0;
    int nStartPos = m_nVocStartFrame+m_nMinSpeechFrameNum-m_nMinSilFrameNum;
    if (nStartPos < m_nVocStartOffset)
        nStartPos = m_nVocStartOffset;
	for (int i = nStartPos; i < m_nCurProbFrameNum; i++) {
		if (i >= nStartPos + m_nMinSilFrameNum) {
			if ((fSilProb > fVocProb
					&& nSilFrames >= m_fMinSilRatio * m_nMinSilFrameNum
                    && !m_bprobs[i])
                    || (m_bEnablePitch && JudgePitchVoiceEnd(i))) {
				m_nVocStopFrame = i;
                //printf("find end: %f %f %d\n", fSilProb, fVocProb, nSilFrames);
                return;
			}
			fSilProb -= m_probs[i - m_nMinSilFrameNum][0];
			fVocProb -= m_probs[i - m_nMinSilFrameNum][1];
			if (!m_bprobs[i - m_nMinSilFrameNum])
				nSilFrames--;
		}
		fSilProb += m_probs[i][0];
		fVocProb += m_probs[i][1];
		if (!m_bprobs[i])
			nSilFrames++;
	}
    //printf("judge end: %f %f %d\n", fSilProb, fVocProb, nSilFrames);
}

int VoiceDetector::GetVoiceFrameNum() {
	if (m_nVocStartFrame >= 0)
		if (m_nVocStopFrame >= 0)
			return m_nVocStopFrame - m_nVocStartFrame;
		else
			return m_nCurProbFrameNum - m_nVocStartFrame;
	else
		return 0;
}

const short *VoiceDetector::GetVoice() {
	if (m_nVocStartFrame >= 0) {
		return (short*) m_bbWave.GetBytes();
	} else
		return 0;
}

const float *VoiceDetector::GetFloatVoice() {
	if (m_nVocStartFrame >= 0) {
		return (float*) m_bbFloatWave.GetBytes();
	} else
		return 0;
}

void VoiceDetector::Freeze(bool bFreeze, int nSampleNum) {
    NNV_FreezeSpeech(m_hVad, bFreeze, nSampleNum/VAD_FRAME_SIZE);
}

void VoiceDetector::RecreatePitchExtractor() {
    if (m_pPitchExtr) {
        delete m_pPitchExtr;
        m_pPitchExtr = 0;
    }

    m_pPitchExtr = new OnlinePitchFeature(m_pitchOpts);
}

static BaseFloat NccfToPov(BaseFloat n) {
    BaseFloat ndash = fabs(n);
    if (ndash > 1.0) ndash = 1.0;  // just in case it was slightly outside [-1, 1]

    BaseFloat r = -5.2 + 5.4 * exp(7.5 * (ndash - 1.0)) + 4.8 * ndash -
    2.0 * exp(-10.0 * ndash) + 4.2 * exp(20.0 * (ndash - 1.0));
    // r is the approximate log-prob-ratio of voicing, log(p/(1-p)).
    BaseFloat p = 1.0 / (1 + expf(-1.0 * r));
    //KALDI_ASSERT(p - p == 0);  // Check for NaN/inf
    return p;
}

void VoiceDetector::CalcPitchFeature(char *pWaveData, int nFrameNum, bool isFloat) {
    if (!isFloat) {
        return;
    }

    const int nFrameSize = VAD_FRAME_SIZE * sizeof(float);
    int32 cur_frame = m_pPitchExtr->NumFramesReady();
    //printf("cur_pitch_frame: %d\n", cur_frame);
    if (cur_frame >= m_nPitchExtrRecreateFrameNum) {
        RecreatePitchExtractor();
        cur_frame = 0;

        // there are 4 frame-delay of kaldi OnlinePitchFeature
        // re-input 4 frame speech into pitch extractor
        // caution: 4 maybe vary according to some pitch parameters
        int nPrevFrameNum = 4;
        float *pPrevFrames = (float*)m_bbFloatWave.GetBytes();
        int nBufFrameNum = m_bbFloatWave.GetByteNum() / nFrameSize;
        if (nBufFrameNum < nPrevFrameNum)
            nPrevFrameNum = nBufFrameNum;
        pPrevFrames += (nBufFrameNum-nPrevFrameNum) * VAD_FRAME_SIZE;
        while (nPrevFrameNum) {
            SubVector<BaseFloat> wave_chunk(pPrevFrames, VAD_FRAME_SIZE);
            m_pPitchExtr->AcceptWaveform(VAD_SAMPLE_RATE, wave_chunk);
            pPrevFrames += VAD_FRAME_SIZE;
            nPrevFrameNum --;
        }
        if (m_pPitchExtr->NumFramesReady()) {
            // error
            int debug = 1;
        }
    }

    Matrix<BaseFloat> features;
    for (int i=0; i<nFrameNum; i++) {
        float *pFrame = (float*)(pWaveData+nFrameSize*i);
        float pOutFrame[nFrameSize];
        r2ssp_bpf_process(m_hBpf, pFrame, VAD_FRAME_SIZE, pOutFrame);

        SubVector<BaseFloat> wave_chunk(pOutFrame, VAD_FRAME_SIZE);
        m_pPitchExtr->AcceptWaveform(VAD_SAMPLE_RATE, wave_chunk);
        int32 num_frames = m_pPitchExtr->NumFramesReady();
        int32 new_frame  = num_frames - cur_frame;
        //printf("pitch num: %d\n", new_frame);
        if( new_frame > 0){
            features.Resize(new_frame, 2);
            for(int32 ii=0; ii < new_frame; ii++){
                SubVector<BaseFloat> row(features,ii);
                m_pPitchExtr->GetFrame(ii+cur_frame, &row);
                BaseFloat prob = NccfToPov(row(0));
                //printf("prob: %f\n", prob);
                if (m_nCurPitchProbFrameNum < VAD_MAX_FRAME_NUM)
                    m_pitchprobs[m_nCurPitchProbFrameNum++] = prob;
                else {
                    // error
                    int debug = 1;
                }
            }
        }
        cur_frame = num_frames;
    }
}

bool VoiceDetector::JudgePitchVoiceBegin(int nCurFrameIdx) {
    if (nCurFrameIdx < m_nMinVocFrameNum) {
        // error
        return false;
    }
    if (nCurFrameIdx >= m_nCurPitchProbFrameNum)
        nCurFrameIdx = m_nCurPitchProbFrameNum-1;

    int nPitchFrameNum = 0;
    for (int i=0; i<m_nMinVocFrameNum; i++) {
        if (m_pitchprobs[nCurFrameIdx-i] >= m_fPitchProbThreshold)
            nPitchFrameNum ++;
        if (nPitchFrameNum >= m_nMinBeginPitchFrameNum)
            return true;
    }
    return false;
}

bool VoiceDetector::JudgePitchVoiceEnd(int nCurFrameIdx) {
    int nCheckFrameNum = m_nMinSilFrameNum + m_nMinVocFrameNum;
    if (nCurFrameIdx < nCheckFrameNum) {
        // error
        return false;
    }
    if (nCurFrameIdx >= m_nCurPitchProbFrameNum)
        nCurFrameIdx = m_nCurPitchProbFrameNum-1;

    int nPitchFrameNum = 0;
    for (int i=0; i<nCheckFrameNum; i++) {
        if (m_pitchprobs[nCurFrameIdx-i] >= m_fPitchProbThreshold)
            nPitchFrameNum ++;
        if (++nPitchFrameNum >= m_nMinEndPitchFrameNum)
            return false;
    }
    return true;
}


