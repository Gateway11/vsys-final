#ifndef TTS_OPUS_H
#define TTS_OPUS_H

#include "opus.h"
#include "opus_types.h"
#include "opus_multistream.h"
#include <stdio.h>
#include <utils/String8.h>

typedef enum {
				SR_8K = 8000,
				SR_16K = 16000,
				SR_24K = 24000,
				SR_48K = 48000,
}OpusSampleRate;
typedef enum {
	AUDIO = 2048,
	VOIP = 2049,
}OPUS_APPLICATION;
class TtsOpus {
	public:
		long encoder;
		long decoder;
		TtsOpus(int sample_rate, int channels, int bitrate, int application);
		long native_opus_encoder_create(int sample_rate, int channels,
												int bitrate, int application);
		long native_opus_decoder_create(int sample_rate, int channels, int bitrate);
		uint32_t native_opus_encode(long enc, const char* in, size_t length, unsigned char* &opus);
		uint32_t native_opus_decode(long dec, const char* in, size_t length, char* &pcm_out);
};

#endif
