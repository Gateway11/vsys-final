/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef AUDIO_HW_RINGBUF_H
#define AUDIO_HW_RINGBUF_H

#define CAP_THRESHOLD_BYTES_BY_RATE(ch, format, rate, period_ms) \
    (ch * ((pcm_format_to_bits(format) >> 3)) * (rate * period_ms) / 1000)
#define CALCULATE_BYTES(ch, format, rate, period_ms) \
    (ch * ((pcm_format_to_bits(format) >> 3)) * (rate * period_ms) / 1000)
#define MIX_THRESHOLD_BYTES(ch, format) \
    (ch * ((pcm_format_to_bits(format) >> 3)) * (HIFI_SAMPLING_RATE * OUT_PERIOD_MS) / 1000)
#define MIX_THRESHOLD_BYTES_BY_RATE(ch, format, rate) \
    (ch * ((pcm_format_to_bits(format) >> 3)) * (rate * OUT_PERIOD_MS) / 1000)
#define MIX_THRESHOLD_FRAMES(buf_size, format) (buf_size / ((pcm_format_to_bits(format) >> 3)))

typedef struct circlebuf_s audio_circlebuf_t;

audio_circlebuf_t *audio_circlebuf_create(int32_t size);
audio_circlebuf_t *audio_circlebuf_create_pcm(uint32_t channel, enum pcm_format format,
                                              uint32_t rate, int32_t len);
void audio_circlebuf_set_over_write(audio_circlebuf_t *rbuf, bool ov);
bool audio_circlebuf_is_over_write(audio_circlebuf_t *rbuf);
void audio_circlebuf_skip(audio_circlebuf_t *rbuf, int32_t size);
int32_t audio_circlebuf_read(audio_circlebuf_t *rbuf, u_char *out, int32_t size);
int32_t audio_circlebuf_write(audio_circlebuf_t *rbuf, u_char *in, int32_t size);
int32_t audio_circlebuf_size(audio_circlebuf_t *rbuf);
/**
available for reading
 */
int32_t audio_circlebuf_used(audio_circlebuf_t *rbuf);
/**
space left in the ringbuffer for writing
 */
int32_t audio_circlebuf_available(audio_circlebuf_t *rbuf);
int32_t audio_circlebuf_move(audio_circlebuf_t *src, audio_circlebuf_t *dst, int32_t len);
int32_t audio_circlebuf_copy(audio_circlebuf_t *src, audio_circlebuf_t *dst, int32_t len);
void audio_circlebuf_clear(audio_circlebuf_t *rbuf);
void audio_circlebuf_destroy(audio_circlebuf_t *rbuf);
int32_t audio_circlebuf_get_ch(audio_circlebuf_t *rb);
enum pcm_format audio_circlebuf_get_fmt(audio_circlebuf_t *rb);

#endif
