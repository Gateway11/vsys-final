/*
 * r2_beamformer.cc
 *
 *  Created on: 2015-3-19
 *      Author: gaopeng
 */

#include <math.h>
#include <string.h>
#include <vector>
#include "r2_beamformer.h"
#include "frame_delayer.h"

#include "webrtc/common_audio/fir_filter.h"
using namespace webrtc;

//#define LOG_TAG "r2_beamformer"
//#include <android/log.h>
//#undef LOGD
//#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
//#undef LOGE
//#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

/* 20c, 1pa, normal condition. */
const float kSoundSpeed = 343.3f;

const int kSincFilterLength = 11;

class R2Beamformer {
public:
	R2Beamformer(bf_point *pMics, int micNum);
	virtual ~R2Beamformer();

	int Init(int nFrameSizeMs, int nSampleRate, float azimuth, float elevation);

	int Process(const float *pInFrames, float *pOutFrame);

private:
	std::vector<bf_point> mics_;

	int frameSizeMs_;
	int sampleRate_;
	float azimuth_, elevation_;

	int frameSize_;
	std::vector<FIRFilter*> fractionalDelays_;
	std::vector<FrameDelayer> integralDelays_;
	int max_delay_idx_;
	float *frameBuffer_;
};

R2Beamformer::R2Beamformer(bf_point *pMics, int micNum) {
	frameBuffer_ = 0;

	if (!pMics || micNum <= 1)
		return;

	for (int i = 0; i < micNum; i++)
		mics_.push_back(pMics[i]);
}

R2Beamformer::~R2Beamformer() {
	for (int i = 0; i < fractionalDelays_.size(); i++)
		delete fractionalDelays_[i];
	fractionalDelays_.clear();
	integralDelays_.clear();
	if (frameBuffer_) {
		delete[] frameBuffer_;
		frameBuffer_ = 0;
	}
}

/**
 * delay: Fractional delay amount
 * filterLength: Number of FIR filter taps (should be odd)
 */
static void calc_sinc_filter_coeff(double delay, int filterLength,
		float *coeffs) {
	int centreTap = filterLength / 2; // Position of center FIR tap

	const double PI = 3.141592653589793;
	for (int t = 0; t < filterLength; t++) {
		// Calculated shifted x position
		double x = t - delay;

		// Calculate sinc function value
		double sinc = sin(PI * (x - centreTap)) / (PI * (x - centreTap));

		// Calculate (Hamming) windowing function value
		double window = 0.54 - 0.46 * cos(2.0 * PI * (x + 0.5) / filterLength);

		// Calculate tap weight
		coeffs[t] = window * sinc;
	}
}

int R2Beamformer::Init(int nFrameSizeMs, int nSampleRate, float azimuth,
		float elevation) {
	if (nFrameSizeMs <= 0 || nSampleRate <= 0)
		return -1;

	frameSizeMs_ = nFrameSizeMs;
	sampleRate_ = nSampleRate;
	azimuth_ = azimuth;
	elevation_ = elevation;

	frameSize_ = frameSizeMs_ * (sampleRate_ / 1000);
	if (frameBuffer_) {
		delete[] frameBuffer_;
		frameBuffer_ = 0;
	}
	frameBuffer_ = new float[frameSize_];

	for (int i = 0; i < fractionalDelays_.size(); i++)
		delete fractionalDelays_[i];
	fractionalDelays_.clear();
	integralDelays_.clear();

	/*
	 * calculate delays
	 * reference:
	 * http://www.labbookpages.co.uk/audio/beamforming/delayCalc.html
	 * http://www.labbookpages.co.uk/audio/beamforming/coordinates.html
	 */
	float cos_azimuth = cos(azimuth_);
	float cos_elevation = cos(elevation_);
	float sin_azimuth = sin(azimuth_);
	float sin_elevation = sin(elevation_);
	std::vector<double> delays;
	double max_delay = -1e100;
	for (int i = 0; i < mics_.size(); i++) {
		double wx = cos_elevation * cos_azimuth;
		double wy = sin_elevation;
		double wz = cos_elevation * sin_azimuth;
		bf_point& mic = mics_[i];
		double delay = (wx * mic.x + wy * mic.y + wz * mic.z) / kSoundSpeed;
		delay *= sampleRate_;
		delays.push_back(delay);
		if (delay > max_delay) {
			max_delay = delay;
			max_delay_idx_ = i;
		}
	}

	/**
	 * create delay filters
	 */
	integralDelays_.resize(delays.size());
	for (int i = 0; i < delays.size(); i++) {
		delays[i] = max_delay - delays[i]; // align all to maximal delay
		int integral_delay = floor(delays[i]);
		if (integral_delay > 0)
			integralDelays_[i].Init(frameSize_ * sizeof(float),
					floor(delays[i]) * sizeof(float));
		double fractional_delay = delays[i] - floor(delays[i]);
		if (fractional_delay >= 0.0001f) {
			float coeffs[kSincFilterLength];
			calc_sinc_filter_coeff(fractional_delay, kSincFilterLength, coeffs);
			fractionalDelays_.push_back(
					FIRFilter::Create(coeffs, kSincFilterLength, frameSize_));
		} else {
			fractionalDelays_.push_back(0);
		}
	}

	return 0;
}

int R2Beamformer::Process(const float *pInFrames, float *pOutFrame) {
	if (!pInFrames || !pOutFrame)
		return -1;

	int micNum = mics_.size();
	for (int i = 0; i < frameSize_; i++) {
		pOutFrame[i] = 0;
	}

	for (int i = 0; i < micNum; i++) {
		integralDelays_[i].ProcessFrame((char*) (pInFrames + i * frameSize_));

		if (fractionalDelays_[i])
			fractionalDelays_[i]->Filter(pInFrames + i * frameSize_, frameSize_,
					frameBuffer_);
		for (int j = 0; j < frameSize_; j++)
			pOutFrame[j] += frameBuffer_[j];
	}
	for (int i = 0; i < frameSize_; i++) {
		pOutFrame[i] /= micNum;
	}
	return 0;
}

class R2FrameDelay {
public:
	R2FrameDelay() {
		fractionalDelay_ = 0;
	}
	~R2FrameDelay() {
		if (fractionalDelay_)
			delete fractionalDelay_;
	}
	int Init(int frameSize, float delay) {
		if (frameSize <= 0 || delay > frameSize)
			return -1;
		int intDelay = floor(delay);
		if (integralDelay_.Init(frameSize * sizeof(float),
				intDelay * sizeof(float)) == 0) {
			float fDelay = delay - intDelay;
			if (fDelay > 0.001f) {
				float coeffs[kSincFilterLength];
				calc_sinc_filter_coeff(fDelay, kSincFilterLength, coeffs);
				fractionalDelay_ = FIRFilter::Create(coeffs, kSincFilterLength,
						frameSize);
			}
			frameSize_ = frameSize;
			return 0;
		} else
			return -1;
	}
	int Process(const float *pInFrame, float *pOutFrame) {
		if (!pInFrame || !pOutFrame)
			return -1;
		integralDelay_.ProcessFrame((char*) pInFrame);
		if (fractionalDelay_)
			fractionalDelay_->Filter(pInFrame, frameSize_, pOutFrame);
		return 0;
	}

private:
	FrameDelayer integralDelay_;
	FIRFilter *fractionalDelay_;
	int frameSize_;
};


#ifdef __cplusplus
extern "C" {
#endif

bf_handle r2_bf_create(bf_point *pMics, int micNum) {
	R2Beamformer *pBf = new R2Beamformer(pMics, micNum);
	return (bf_handle) pBf;
}

int r2_bf_free(bf_handle hBf) {
	R2Beamformer *pBf = (R2Beamformer*) hBf;
	if (!pBf)
		return -1;

	delete pBf;
	return 0;
}

int r2_bf_init(bf_handle hBf, int nFrameSizeMs, int nSampleRate, float azimuth,
		float elevation) {
	R2Beamformer *pBf = (R2Beamformer*) hBf;
	if (!pBf)
		return -1;

	return pBf->Init(nFrameSizeMs, nSampleRate, azimuth, elevation);
}

int r2_bf_process(bf_handle hBf, const float *pInFrames, float *pOutFrame) {
	R2Beamformer *pBf = (R2Beamformer*) hBf;
	if (!pBf)
		return -1;

	return pBf->Process(pInFrames, pOutFrame);
}

bf_handle r2_dl_create() {
	return (bf_handle) new R2FrameDelay();
}

int r2_dl_free(bf_handle hDl) {
	R2FrameDelay *pDelay = (R2FrameDelay*) hDl;
	delete pDelay;
	return 0;
}

int r2_dl_init(bf_handle hDl, int frameSize, float delay) {
	if (!hDl)
		return -1;
	R2FrameDelay *pDelay = (R2FrameDelay*) hDl;
	return pDelay->Init(frameSize, delay);
}

int r2_dl_process(bf_handle hDl, const float *pInFrame, float *pOutFrame) {
	if (!hDl)
		return -1;
	R2FrameDelay *pDelay = (R2FrameDelay*) hDl;
	return pDelay->Process(pInFrame, pOutFrame);
}

#ifdef __cplusplus
}
#endif
