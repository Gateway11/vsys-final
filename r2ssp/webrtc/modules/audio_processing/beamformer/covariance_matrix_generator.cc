/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#define _USE_MATH_DEFINES

#include "webrtc/modules/audio_processing/beamformer/covariance_matrix_generator.h"

#include <cmath>

namespace {

float BesselJ0(float x) {
#if WEBRTC_WIN
	return _j0(x);
#else
	return j0(x);
#endif
}

}  // namespace

namespace webrtc {

void CovarianceMatrixGenerator::UniformCovarianceMatrix(float wave_number,
		const std::vector<Point>& geometry, ComplexMatrix<float>* mat) {
	CHECK_EQ(static_cast<int>(geometry.size()), mat->num_rows());
	CHECK_EQ(static_cast<int>(geometry.size()), mat->num_columns());

	complex<float>* const * mat_els = mat->elements();
	for (size_t i = 0; i < geometry.size(); ++i) {
		for (size_t j = 0; j < geometry.size(); ++j) {
			if (wave_number > 0.f) {
				mat_els[i][j] = BesselJ0(
						wave_number * Distance(geometry[i], geometry[j]));

				/*if (i == j)
				 mat_els[i][j] = 1.f;
				 else {
				 float ph = wave_number * Distance(geometry[i], geometry[j]);
				 if (ph > 0.f)
				 mat_els[i][j] = sin(ph) / ph;
				 else
				 mat_els[i][j] = 1.f;
				 }
				 */

				//mat_els[i][j] = cos(
				//		wave_number * Distance(geometry[i], geometry[j]));
			} else {
				mat_els[i][j] = i == j ? 1.f : 0.f;
			}
		}
	}
}

void CovarianceMatrixGenerator::AngledCovarianceMatrix(float sound_speed,
		float angle, float angle2, int frequency_bin, int fft_size,
		int num_freq_bins, int sample_rate, const std::vector<Point>& geometry,
		const std::vector<float>& delays, ComplexMatrix<float>* mat) {
	CHECK_EQ(static_cast<int>(geometry.size()), mat->num_rows());
	CHECK_EQ(static_cast<int>(geometry.size()), mat->num_columns());

	ComplexMatrix<float> interf_cov_vector(1, geometry.size());
	ComplexMatrix<float> interf_cov_vector_transposed(geometry.size(), 1);
	PhaseAlignmentMasks(frequency_bin, fft_size, sample_rate, sound_speed,
			geometry, delays, angle, angle2, &interf_cov_vector);
	interf_cov_vector_transposed.Transpose(interf_cov_vector);
	interf_cov_vector.PointwiseConjugate();
	mat->Multiply(interf_cov_vector_transposed, interf_cov_vector);
}

void CovarianceMatrixGenerator::PhaseAlignmentMasks(int frequency_bin,
		int fft_size, int sample_rate, float sound_speed,
		const std::vector<Point>& geometry, const std::vector<float>& delays,
        float angle, float angle2, ComplexMatrix<float>* mat) {
        CHECK_EQ(1, mat->num_rows());
        CHECK_EQ(static_cast<int>(geometry.size()), mat->num_columns());
    
        float freq_in_hertz = (static_cast<float>(frequency_bin) / fft_size) * sample_rate;
        std::vector<float> delays_new;
        std::vector<float> distance_delays;//add by ygqin
        distance_delays.resize(geometry.size(),0.0);//ygqin

        if(delays.size()==0){
            delays_new.resize(geometry.size(),0.0);
            distance_delays=delays_new;
        }else
            distance_delays=delays;
        complex<float>* const * mat_els = mat->elements();
        for (size_t c_ix = 0; c_ix < geometry.size(); ++c_ix) {
            //float distance = std::cos(angle) * geometry[c_ix].x() +
            //                 std::sin(angle) * geometry[c_ix].y();
            distance_delays[c_ix]=distance_delays[c_ix]*sound_speed;//delay time to distance_delay;
        
            float distance = std::cos(angle) * std::cos(angle2) * geometry[c_ix].x()
                            + std::sin(angle) * std::cos(angle2) * geometry[c_ix].y()
                            + std::sin(angle2) * geometry[c_ix].z()+distance_delays[c_ix];
            float phase_shift = -2.f * M_PI  * freq_in_hertz * distance/sound_speed;
            // Euler's formula for mat[0][c_ix] = e^(j * phase_shift).
            mat_els[0][c_ix] = complex<float>(cos(phase_shift), sin(phase_shift));
        }
    }

}  // namespace webrtc
