#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define DEFAULT_SAMPLE_RATE 48000
#define DEFAULT_BIT_DEPTH   16
#define DEFAULT_CHANNELS    32
#define DEFAULT_DURATION    300     /* 5 minutes */
#define TONE_FREQ           1000.0  /* 1 kHz */

#pragma pack(push, 1)
typedef struct {
    char     riff_id[4];      /* "RIFF" */
    uint32_t riff_size;
    char     wave_id[4];      /* "WAVE" */

    char     fmt_id[4];       /* "fmt " */
    uint32_t fmt_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;

    char     data_id[4];      /* "data" */
    uint32_t data_size;
} wav_header_t;
#pragma pack(pop)

static void print_usage(const char *prog)
{
    printf(
        "Usage:\n"
        "  %s [sample_rate] [bit_depth] [channels] [duration_sec]\n\n"
        "Arguments:\n"
        "  sample_rate     Sample rate in Hz (default: %d)\n"
        "  bit_depth       PCM bit depth: 16 or 32 (default: %d)\n"
        "  channels        Number of channels (default: %d)\n"
        "  duration_sec    Duration in seconds (default: %d = 5 minutes)\n\n"
        "Examples:\n"
        "  %s 48000 16 32 300\n"
        "  %s 48000 32 8 10\n",
        prog,
        DEFAULT_SAMPLE_RATE,
        DEFAULT_BIT_DEPTH,
        DEFAULT_CHANNELS,
        DEFAULT_DURATION,
        prog, prog 
    );
}

static void write_sample(FILE *fp, double sample, int bit_depth)
{
    if (bit_depth == 16) {
        int16_t v = (int16_t)(sample * 32767.0);
        fwrite(&v, sizeof(v), 1, fp);
    } else { /* 32-bit PCM */
        int32_t v = (int32_t)(sample * 2147483647.0);
        fwrite(&v, sizeof(v), 1, fp);
    }
}

int main(int argc, char *argv[])
{
    char filename[128];
    int sample_rate  = DEFAULT_SAMPLE_RATE;
    int bit_depth    = DEFAULT_BIT_DEPTH;
    int channels     = DEFAULT_CHANNELS;
    int duration_sec = DEFAULT_DURATION;

    if (argc < 2 ||
        strcmp(argv[1], "-h") == 0 ||
        strcmp(argv[1], "--help") == 0) {
        print_usage(argv[0]);
        return 0;
    }

    if (argc > 1) sample_rate  = atoi(argv[1]);
    if (argc > 2) bit_depth    = atoi(argv[2]);
    if (argc > 3) channels     = atoi(argv[3]);
    if (argc > 4) duration_sec = atoi(argv[4]);
    snprintf(filename, sizeof(filename), "gen_sine_%d_%dbit_%dch_%ds.wav",
         sample_rate, bit_depth, channels, duration_sec);

    if (bit_depth != 16 && bit_depth != 32) {
        fprintf(stderr, "Error: only 16-bit and 32-bit PCM are supported\n");
        print_usage(argv[0]);
        return -1;
    }

    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("fopen");
        return -1;
    }

    int bytes_per_sample = bit_depth / 8;
    uint64_t total_frames = (uint64_t)sample_rate * duration_sec;
    uint64_t data_size = total_frames * channels * bytes_per_sample;

    if (data_size > 0xFFFFFFFF) {
        fprintf(stderr, "Error: WAV file exceeds 4GB RIFF limit\n");
        fclose(fp);
        return -1;
    }

    wav_header_t header = {
        .riff_id = "RIFF",
        .riff_size = 36 + (uint32_t)data_size,
        .wave_id = "WAVE",
        .fmt_id  = "fmt ",
        .fmt_size = 16,
        .audio_format = 1,
        .num_channels = channels,
        .sample_rate = sample_rate,
        .byte_rate = sample_rate * channels * bytes_per_sample,
        .block_align = channels * bytes_per_sample,
        .bits_per_sample = bit_depth,
        .data_id = "data",
        .data_size = (uint32_t)data_size,
    };

    fwrite(&header, sizeof(header), 1, fp);

    for (uint64_t n = 0; n < total_frames; n++) {
        double s = sin(2.0 * M_PI * TONE_FREQ * n / sample_rate);
        for (int ch = 0; ch < channels; ch++)
            write_sample(fp, s, bit_depth);
    }

    fclose(fp);

    printf("Generated %s (%d Hz, %d-bit, %d ch, %d sec, 1 kHz)\n",
           filename, sample_rate, bit_depth, channels, duration_sec);

    return 0;
}
