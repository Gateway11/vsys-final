#define LOG_TAG "audio_hw_sample"
// #define LOG_NDEBUG 0
#include <cutils/log.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <tinyalsa/asoundlib.h>

#include "pcm_plugin.h"
#define DEBUG_FUN_IN ALOGV("%s(), LINE:%d in --->", __func__, __LINE__);
#define DEBUG_FUN_OUT ALOGV("%s(), LINE:%d in --->", __func__, __LINE__);

typedef struct sample_plug {
    int32_t sound_index;
    struct pcm_config config;
    FILE *inject_fp;
} sample_plug_t;

int plug_open(struct pcm_plugin **plugin, unsigned int card, unsigned int device,
              unsigned int flags) {
    struct pcm_plugin *plugin_data = NULL;
    char *inject_file_path = "/data/semidrive_tts.pcm";
    DEBUG_FUN_IN

    sample_plug_t *priv = calloc(1, sizeof(sample_plug_t));
    if (!priv)
        return -ENOMEM;

    plugin_data = (struct pcm_plugin *)calloc(1, sizeof(struct pcm_plugin));
    if (!plugin_data) {
        if (priv)
            free(priv);
        return -ENOMEM;
    }

    plugin_data->constraints = calloc(1, sizeof(struct pcm_plugin_hw_constraints));
    if (!plugin_data->constraints) {
        if (priv)
            free(priv);
        if (plugin_data)
            free(plugin_data);
        return -ENOMEM;
    }

    plugin_data->card = card;
    plugin_data->device = device;
    priv->inject_fp = fopen(inject_file_path, "rb");
    if (!priv->inject_fp)
        ALOGW("%s : %s, inject will not work", inject_file_path, strerror(errno));
    priv->sound_index = 0;
    plugin_data->priv = priv;
    *plugin = plugin_data;

    return 0;
}

int plug_close(struct pcm_plugin *plugin) {
    sample_plug_t *priv = NULL;

    if (!plugin)
        return -EINVAL;

    priv = (sample_plug_t *)plugin->priv;
    DEBUG_FUN_IN
    if (plugin) {
        if (plugin->constraints)
            free(plugin->constraints);
        if (priv) {
            if (priv->inject_fp) {
                fclose(priv->inject_fp);
                priv->inject_fp = NULL;
            }
            free(priv);
        }
        free(plugin);
    }
    return 0;
}
static void dump_out_data(const void *buffer, size_t bytes, int *size, char *path) {
    static FILE *fd;
    static int offset = 0;
    if (!buffer || !path)
        return;
    ALOGV("dump file is in %s", path);
    if (fd == NULL) {
        fd = fopen(path, "wb+");
        if (fd == NULL) {
            ALOGV(
                "DEBUG open  error =%d "
                "%s, errno = (%s)%d",
                (int)fd, path, strerror(errno), errno);
            offset = 0;
            return;
        }
    }
    fwrite(buffer, bytes, 1, fd);
    offset += bytes;
    fflush(fd);
    /* Stop dump if over than 100M */
    if (offset >= (100 * 1024 * 1024)) {
        if (size)
            *size = 0;
        fclose(fd);
        offset = 0;
        ALOGD("pcm dump end");
    }
}
int plug_writei_frames(struct pcm_plugin *plugin, struct snd_xferi *x) {
    DEBUG_FUN_IN
    sample_plug_t *priv = (sample_plug_t *)plugin->priv;
    int32_t period_bytes = (pcm_format_to_bits(priv->config.format) / 8 /*format*/) *
                           x->frames /*period_size*/ * priv->config.channels /*ch*/;
    ALOGV("%s, writei: %lu frames", __func__, x->frames);
    char dump_path[128];  // TODO: Read from getprop
    snprintf(dump_path, sizeof(dump_path), "%sdump_pcm_c%dd%d.pcm", "/data/", plugin->card,
             plugin->device);
    dump_out_data(x->buf, period_bytes, NULL, dump_path);
    x->result = x->frames;
    return 0;
}

int plug_readi_frames(struct pcm_plugin *plugin, struct snd_xferi *x) {
    DEBUG_FUN_IN
    sample_plug_t *priv = (sample_plug_t *)plugin->priv;
    int32_t period_bytes = (pcm_format_to_bits(priv->config.format) / 8 /*format*/) * x->frames *
                           priv->config.channels /*ch*/;
    int32_t ret = 0;
    if (!priv->inject_fp) {
        x->result = x->frames;
        return 0;
    }

    ret = fread(x->buf, 1, period_bytes, priv->inject_fp);
    if (ret < period_bytes)
        rewind(priv->inject_fp);
    x->result =
        ret / (pcm_format_to_bits(priv->config.format) / 8 /*format*/) / priv->config.channels;
    return 0;
}

int plug_prepare(struct pcm_plugin *plugin) {
    DEBUG_FUN_IN
    return 0;
}
static struct snd_mask *hw_param_mask(struct snd_pcm_hw_params *params, snd_pcm_hw_param_t var) {
    return &params->masks[var - SNDRV_PCM_HW_PARAM_FIRST_MASK];
}
static struct snd_interval *param_get_interval(const struct snd_pcm_hw_params *p, int n) {
    return (struct snd_interval *)&(p->intervals[n - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL]);
}
static int param_is_interval(int p) {
    return (p >= SNDRV_PCM_HW_PARAM_FIRST_INTERVAL) && (p <= SNDRV_PCM_HW_PARAM_LAST_INTERVAL);
}
static struct snd_interval *param_to_interval(struct snd_pcm_hw_params *p, int n) {
    return &(p->intervals[n - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL]);
}
static unsigned int param_get_min(const struct snd_pcm_hw_params *p, int n) {
    if (param_is_interval(n)) {
        const struct snd_interval *i = param_get_interval(p, n);
        return i->min;
    }
    return 0;
}
static unsigned int param_get_int(struct snd_pcm_hw_params *p, int n) {
    if (param_is_interval(n)) {
        struct snd_interval *i = param_to_interval(p, n);
        if (i->integer)
            return i->max;
    }
    return 0;
}
int plug_hw_params(struct pcm_plugin *plugin, struct snd_pcm_hw_params *params) {
    sample_plug_t *priv = (sample_plug_t *)plugin->priv;
    DEBUG_FUN_IN
    struct snd_mask *fmt = hw_param_mask(params, SNDRV_PCM_HW_PARAM_FORMAT);
    priv->config.format = fmt->bits[0];
    priv->config.period_size =
        param_get_min((const struct snd_pcm_hw_params *)params, SNDRV_PCM_HW_PARAM_PERIOD_SIZE);
    priv->config.channels =
        param_get_int((struct snd_pcm_hw_params *)params, SNDRV_PCM_HW_PARAM_CHANNELS);
    priv->config.rate = param_get_int((struct snd_pcm_hw_params *)params, SNDRV_PCM_HW_PARAM_RATE);
    return 0;
}
int plug_start(struct pcm_plugin *plugin) {
    DEBUG_FUN_IN
    return 0;
}

struct pcm_plugin_ops pcm_plugin_ops = {
    .open = plug_open,
    .close = plug_close,
    .writei_frames = plug_writei_frames,
    .readi_frames = plug_readi_frames,
    .prepare = plug_prepare,
    .hw_params = plug_hw_params,
    .start = plug_start,
};