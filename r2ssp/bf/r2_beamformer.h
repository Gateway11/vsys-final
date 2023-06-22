/*
 * r2_beamformer.h
 *
 *  Created on: 2015-3-18
 *      Author: gaopeng
 */

#ifndef R2_BEAMFORMER_H_
#define R2_BEAMFORMER_H_

/**
 * user-defined geometry microphone array beam-forming API
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef long long bf_handle;

typedef struct {
	float x;
	float y;
	float z;
} bf_point;

bf_handle r2_bf_create(bf_point *pMics, int micNum);

int r2_bf_free(bf_handle hBf);

/*
 * azimuth: angle between x and z
 * elevation: angle between x and y
 */
int r2_bf_init(bf_handle hBf, int nFrameSizeMs, int nSampleRate,
		float azimuth, float elevation);

int r2_bf_process(bf_handle hBf, const float *pInFrames, float *pOutFrame);


bf_handle r2_dl_create();

int r2_dl_free(bf_handle hDl);

int r2_dl_init(bf_handle hDl, int frameSize, float delay);

int r2_dl_process(bf_handle hDl, const float *pInFrame, float *pOutFrame);

#ifdef __cplusplus
}
#endif

#endif /* R2_BEAMFORMER_H_ */
