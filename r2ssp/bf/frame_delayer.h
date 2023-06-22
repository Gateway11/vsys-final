/*
 * frame_delayer.h
 *
 *  Created on: 2015-3-19
 *      Author: gaopeng
 */

#ifndef FRAME_DELAYER_H_
#define FRAME_DELAYER_H_

#include <string.h>

class FrameDelayer {
public:
	FrameDelayer() {
		frameSize_ = 0;
		delay_ = 0;
		pDelay_ = 0;
	}
	int Init(unsigned frameSize, unsigned delay) {
		frameSize_ = 0;
		delay_ = 0;
		if (pDelay_) {
			delete[] pDelay_;
			pDelay_ = 0;
		}

		if (frameSize < delay)
			return -1;
		frameSize_ = frameSize;
		delay_ = delay;
		if (delay > 0) {
			pDelay_ = new char[delay_ * 2];
			memset(pDelay_, 0, delay_ * 2);
		}
		return 0;
	}
	~FrameDelayer() {
		if (pDelay_)
			delete[] pDelay_;
	}

	void ProcessFrame(char *pFrame) {
		if (!pDelay_ || !pFrame || !delay_)
			return;
		memcpy(pDelay_ + delay_, pFrame + frameSize_ - delay_, delay_);
		memmove(pFrame + delay_, pFrame, frameSize_ - delay_);
		memcpy(pFrame, pDelay_, delay_);
		memcpy(pDelay_, pDelay_ + delay_, delay_);
	}

private:
	unsigned frameSize_;
	unsigned delay_;
	char *pDelay_;
};

#endif /* FRAME_DELAYER_H_ */
