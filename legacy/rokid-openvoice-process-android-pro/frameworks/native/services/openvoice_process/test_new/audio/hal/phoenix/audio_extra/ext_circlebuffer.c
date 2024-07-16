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

#define LOG_TAG "audio_hw_extra"
// #define LOG_NDEBUG 0
#include <log/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "audio_hw.h"
#include "ext_circlebuffer.h"
#include "audio_extra.h"

#define AUDIO_CIRCLEBUF_DEFAULT_SIZE 4096

struct circlebuf_s {
    u_char *data;
    int32_t size;
    int32_t used;
    int32_t read_offset;
    int32_t write_offset;
    bool over_write;
    int32_t chann;
    enum pcm_format fmt;
    uint32_t rate;
    pthread_mutex_t lock;
};

audio_circlebuf_t *audio_circlebuf_create(int32_t size) {
    audio_circlebuf_t *new_rb;
    new_rb = (audio_circlebuf_t *)calloc(1, sizeof(audio_circlebuf_t));
    if (!new_rb) {
        AALOGE("can not alloc mem for rbuf failed!!");
        return NULL;
    }
    if (size == 0)
        new_rb->size = AUDIO_CIRCLEBUF_DEFAULT_SIZE;
    else
        new_rb->size = size;
    new_rb->data = (u_char *)malloc(new_rb->size);
    if (!new_rb->data) {
        AALOGE("can not alloc mem for buf failed!!");
        free(new_rb);
        return NULL;
    }
    pthread_mutex_init(&new_rb->lock, NULL);
    AALOGI("new rb: %p, size: %d", new_rb, size);
    return new_rb;
}
audio_circlebuf_t *audio_circlebuf_create_pcm(uint32_t channel, enum pcm_format format,
                                              uint32_t rate, int32_t len) {
    audio_circlebuf_t *new_rb;
    int32_t size = MIX_THRESHOLD_BYTES_BY_RATE(channel, format, rate) * len;
    new_rb = (audio_circlebuf_t *)calloc(1, sizeof(audio_circlebuf_t));
    if (!new_rb) {
        AALOGE("can not alloc mem for rbuf failed!!");
        return NULL;
    }

    if (size == 0)
        new_rb->size = AUDIO_CIRCLEBUF_DEFAULT_SIZE;
    else
        new_rb->size = size;
    new_rb->data = (u_char *)malloc(new_rb->size);
    if (!new_rb->data) {
        AALOGE("can not alloc mem for buf failed!!");
        free(new_rb);
        return NULL;
    }
    new_rb->chann = channel;
    new_rb->fmt = format;
    new_rb->rate = rate;
    pthread_mutex_init(&new_rb->lock, NULL);
    AALOGI("new rb: %p, size: %d(ch: %d, fmt: %s, rate: %d)", new_rb, size, new_rb->chann,
           audio_extra_get_alsa_format_name(new_rb->fmt), rate);
    return new_rb;
}

void audio_circlebuf_set_over_write(audio_circlebuf_t *rbuf, bool ov) {
    rbuf->over_write = ov;
}

bool audio_circlebuf_is_over_write(audio_circlebuf_t *rbuf) {
    return rbuf->over_write;
}

void audio_circlebuf_skip(audio_circlebuf_t *rb, int32_t size) {
    if (size >= rb->used) {  // just empty the ringbuffer
        rb->read_offset = rb->write_offset;
        rb->used = 0;
    } else {
        rb->used -= size;
        if (size > rb->size - rb->read_offset) {
            size -= rb->size - rb->read_offset;
            rb->read_offset = size;
        } else {
            rb->read_offset += size;
        }
    }
}

int32_t audio_circlebuf_read(audio_circlebuf_t *rb, u_char *out, int32_t size) {
    if (!rb)
        return -EINVAL;
    pthread_mutex_lock(&rb->lock);
    int32_t read_size = size > rb->used ? rb->used : size;
    int32_t to_end = rb->size - rb->read_offset;
    AALOGV("%p size: %d", rb, size);
    if (read_size > to_end) {  // check if we need to wrap around
        memcpy(out, &rb->data[rb->read_offset], to_end);
        int32_t start_size = read_size - to_end;
        memcpy(out + to_end, &rb->data[0], start_size);
        rb->read_offset = start_size;
        AALOGV("to end");
    } else {
        memcpy(out, &rb->data[rb->read_offset], read_size);
        rb->read_offset += read_size;
    }
    rb->used -= read_size;
    pthread_mutex_unlock(&rb->lock);
    return read_size;
}

int32_t audio_circlebuf_write(audio_circlebuf_t *rb, u_char *in, int32_t size) {
    if (!rb || !in || !size)  // safety belt
        return 0;
    pthread_mutex_lock(&rb->lock);
    AALOGV("%p size: %d", rb, size);
    int32_t available_size = rb->size - rb->used;
    int32_t to_end = rb->size - rb->write_offset;
    int32_t write_size = (size > available_size) ? available_size : size;
    if (write_size < size && rb->over_write) {
        if (size > rb->size) {
            // the provided buffer is bigger than the
            // ringbuffer itself. Since we are in overwrite mode,
            // only the last chunk will be actually stored.
            write_size = rb->size;
            in = in + (size - write_size);
            rb->read_offset = 0;
            rb->write_offset = 0;
            memcpy(rb->data, in, write_size);
            rb->used = write_size;
            // NOTE: we still tell the caller we have written all
            // the data even if the initial part has been thrown
            // away
            return size;
        }
        // we are in overwrite mode, so let's make some space
        // for the new data by advancing the read offset
        int32_t diff = size - write_size;
        rb->read_offset += diff;
        write_size += diff;
        if (rb->read_offset >= rb->size)
            rb->read_offset -= rb->size;
        rb->used -= diff;
    }

    if (write_size > to_end) {
        memcpy(&rb->data[rb->write_offset], in, to_end);
        int32_t from_start = write_size - to_end;
        memcpy(&rb->data[0], in + to_end, from_start);
        rb->write_offset = from_start;
    } else {
        memcpy(&rb->data[rb->write_offset], in, write_size);
        rb->write_offset += write_size;
    }
    rb->used += write_size;
    pthread_mutex_unlock(&rb->lock);

    return write_size;
}

int32_t audio_circlebuf_used(audio_circlebuf_t *rb) {
    return rb->used;
}

int32_t audio_circlebuf_size(audio_circlebuf_t *rb) {
    return rb->size;
}

int32_t audio_circlebuf_available(audio_circlebuf_t *rb) {
    return rb->size - rb->used;
}

void audio_circlebuf_clear(audio_circlebuf_t *rb) {
    rb->read_offset = rb->write_offset = 0;
    rb->used = 0;
}

void audio_circlebuf_destroy(audio_circlebuf_t *rb) {
    AALOGI("rb : %p", rb);
    free(rb->data);
    free(rb);
}

static int32_t audio_circlebuf_copy_internal(audio_circlebuf_t *src, audio_circlebuf_t *dst,
                                             int32_t len, int32_t move) {
    if (!src || !dst || !len)
        return 0;

    int32_t to_copy = audio_circlebuf_available(dst);
    if (len < to_copy)
        to_copy = len;

    int32_t available = audio_circlebuf_used(src);
    if (available < to_copy)
        to_copy = available;

    int32_t contiguous = (dst->write_offset > dst->read_offset)
                             ? dst->size - dst->write_offset
                             : dst->read_offset - dst->write_offset;

    if (contiguous >= to_copy) {
        if (move) {
            audio_circlebuf_read(src, &dst->data[dst->write_offset], to_copy);
        } else {
            if (src->read_offset < src->write_offset) {
                memcpy(&dst->data[dst->write_offset], &src->data[src->read_offset], to_copy);
            } else {
                int32_t to_end = src->size - src->read_offset;
                memcpy(&dst->data[dst->write_offset], &src->data[src->read_offset], to_end);
                dst->write_offset += to_end;
                memcpy(&dst->data[dst->write_offset], &src->data[0], to_copy - to_end);
            }
        }
        dst->write_offset += to_copy;
    } else {
        int32_t remainder = to_copy - contiguous;
        if (move) {
            audio_circlebuf_read(src, &dst->data[dst->write_offset], contiguous);
            audio_circlebuf_read(src, &dst->data[0], remainder);
        } else {
            if (src->read_offset < src->write_offset) {
                memcpy(&dst->data[dst->write_offset], &src->data[src->read_offset], contiguous);
                memcpy(&dst->data[0], &src->data[src->read_offset + contiguous], remainder);
            } else {
                int32_t to_end = src->size - src->read_offset;
                if (to_end > contiguous) {
                    memcpy(&dst->data[dst->write_offset], &src->data[dst->read_offset], contiguous);
                    int32_t diff = to_end - contiguous;
                    if (diff > remainder) {
                        memcpy(&dst->data[0], &src->data[dst->read_offset + contiguous], remainder);
                    } else {
                        memcpy(&dst->data[0], &src->data[dst->read_offset + contiguous], diff);
                        memcpy(&dst->data[diff], &src->data[0], remainder - diff);
                    }
                } else {
                    memcpy(&dst->data[dst->write_offset], &src->data[dst->read_offset], to_end);
                    int32_t diff = contiguous - to_end;
                    if (diff) {
                        memcpy(&dst->data[dst->write_offset + to_end], &src->data[0], diff);
                        memcpy(&dst->data[0], &src->data[diff], remainder);
                    }
                }
            }
        }
        dst->write_offset = remainder;
    }
    dst->used = to_copy;
    return to_copy;
}

int32_t audio_circlebuf_move(audio_circlebuf_t *src, audio_circlebuf_t *dst, int32_t len) {
    return audio_circlebuf_copy_internal(src, dst, len, 1);
}

int32_t audio_circlebuf_copy(audio_circlebuf_t *src, audio_circlebuf_t *dst, int32_t len) {
    return audio_circlebuf_copy_internal(src, dst, len, 0);
}
int32_t audio_circlebuf_get_ch(audio_circlebuf_t *rb) {
    if (!rb)
        return -EINVAL;
    return rb->chann;
}
enum pcm_format audio_circlebuf_get_fmt(audio_circlebuf_t *rb) {
    if (!rb)
        return -EINVAL;
    return rb->fmt;
}