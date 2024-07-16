#define LOG_TAG "audio_hw_wrapper"
// #define LOG_NDEBUG 0
#include "tinyalsa/asoundlib.h"
#include "pcm_wrapper.h"
#include "pcm_io.h"
#include "audio_hw.h"
#include <cutils/log.h>
#include <cutils/properties.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <sound/asound.h>
#define PCM_ERROR_MAX 32
#define plug_prop_enable "config.pcm.plug.enable"
#define plug_prop_type "config.pcm.plug.type"

struct pcm_wrapper {
	struct pcm *alsa_pcm;
	void *priv;
	/** The PCM's file descriptor */
	int fd;
	/** Flags that were passed to @ref pcm_open */
	unsigned int flags;
	/** The number of (under/over)runs that have occured */
	int xruns;
	/** Size of the buffer */
	unsigned int buffer_size;
	/** The boundary for ring buffer pointers */
	unsigned int boundary;
	/** Description of the last error that occured */
	char error[PCM_ERROR_MAX];
	/** Configuration that was passed to @ref pcm_open */
	struct pcm_config config;
	struct snd_pcm_mmap_status *mmap_status;
	/** The delay of the PCM, in terms of frames */
	long pcm_delay;
	/** The subdevice corresponding to the PCM */
	unsigned int subdevice;
	/** Pointer to the pcm ops, either hw or plugin */
	const struct pcm_ops *ops;
	struct listnode plug_list;
	/** Private data for pcm_hw or pcm_plugin */
	void *data;
	/** Pointer to the pcm node from snd card definition */
	struct snd_node *snd_node;
	int32_t plug_switch;
};

typedef enum plug_prop {
	SUPPORT_DUMP_PLUG,
	SUPPORT_INJECT_PLUG,
	// TODO: Customization plug type
}plug_prop_type_t;


static int pcm_ioctl_all(pcm_wrapper_t *pcm, int cmd, void *param)
{
	int result = 0;
	if (!pcm->ops)
	{
		return 0;
	}
	AALOGI("enable plug-in");
	if (NULL == param)
		result =
			pcm->ops->ioctl(pcm->data, cmd);
	else
		result = pcm->ops->ioctl(pcm->data,
							cmd, param);
	if (result < 0) {
		AALOGE("cmd %d failed", cmd);
	}

	return result;
}
static int pcm_rw_transfer(pcm_wrapper_t *pcm, void *data, unsigned int frames)
{
	int is_playback;

	struct snd_xferi transfer;
	int res;

	is_playback = !(pcm->flags & PCM_IN);
	transfer.buf = data;
	transfer.frames = frames;
	transfer.result = 0;

	res = pcm->ops->ioctl(pcm->data, is_playback
	                      ? SNDRV_PCM_IOCTL_WRITEI_FRAMES
	                      : SNDRV_PCM_IOCTL_READI_FRAMES, &transfer);
	return res == 0 ? (int)transfer.result : -1;
}
static int pcm_generic_transfer(pcm_wrapper_t *pcm, void *data,
				unsigned int frames)
{
	int res;
	res = pcm_rw_transfer(pcm, data, frames);

	if (res < 0) {
		ALOGW("pcm_rw_transfer err: %d", res);
		switch (errno) {
		case EPIPE:
			pcm->xruns++;
			break;
		}
	}
	return res;
}
static inline struct snd_interval *
param_to_interval(struct snd_pcm_hw_params *p, int n)
{
	return &(p->intervals[n - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL]);
}
static inline int param_is_interval(int p)
{
	return (p >= SNDRV_PCM_HW_PARAM_FIRST_INTERVAL) &&
	       (p <= SNDRV_PCM_HW_PARAM_LAST_INTERVAL);
}

static inline struct snd_mask *param_to_mask(struct snd_pcm_hw_params *p, int n)
{
	return &(p->masks[n - SNDRV_PCM_HW_PARAM_FIRST_MASK]);
}
static inline int param_is_mask(int p)
{
	return (p >= SNDRV_PCM_HW_PARAM_FIRST_MASK) &&
	       (p <= SNDRV_PCM_HW_PARAM_LAST_MASK);
}
static void param_set_mask(struct snd_pcm_hw_params *p, int n, unsigned int bit)
{
	if (bit >= SNDRV_MASK_MAX)
		return;
	if (param_is_mask(n)) {
		struct snd_mask *m = param_to_mask(p, n);
		m->bits[0] = 0;
		m->bits[1] = 0;
		m->bits[bit >> 5] |= (1 << (bit & 31));
	}
}

static void param_set_min(struct snd_pcm_hw_params *p, int n, unsigned int val)
{
	if (param_is_interval(n)) {
		struct snd_interval *i = param_to_interval(p, n);
		i->min = val;
	}
}

static void param_set_int(struct snd_pcm_hw_params *p, int n, unsigned int val)
{
	if (param_is_interval(n)) {
		struct snd_interval *i = param_to_interval(p, n);
		i->min = val;
		i->max = val;
		i->integer = 1;
	}
}

static void param_init(struct snd_pcm_hw_params *p)
{
	int n;

	memset(p, 0, sizeof(*p));
	for (n = SNDRV_PCM_HW_PARAM_FIRST_MASK;
	     n <= SNDRV_PCM_HW_PARAM_LAST_MASK; n++) {
		struct snd_mask *m = param_to_mask(p, n);
		m->bits[0] = ~0;
		m->bits[1] = ~0;
	}
	for (n = SNDRV_PCM_HW_PARAM_FIRST_INTERVAL;
	     n <= SNDRV_PCM_HW_PARAM_LAST_INTERVAL; n++) {
		struct snd_interval *i = param_to_interval(p, n);
		i->min = 0;
		i->max = ~0;
	}
	p->rmask = ~0U;
	p->cmask = 0;
	p->info = ~0U;
}
static unsigned int pcm_format_to_alsa(enum pcm_format format)
{
	switch (format) {

	case PCM_FORMAT_S8:
		return SNDRV_PCM_FORMAT_S8;
	default:
	case PCM_FORMAT_S16_LE:
		return SNDRV_PCM_FORMAT_S16_LE;
	case PCM_FORMAT_S24_LE:
		return SNDRV_PCM_FORMAT_S24_LE;
	case PCM_FORMAT_S24_3LE:
		return SNDRV_PCM_FORMAT_S24_3LE;
	case PCM_FORMAT_S32_LE:
		return SNDRV_PCM_FORMAT_S32_LE;
	};
}

int pcm_wrapper_writei(pcm_wrapper_t *pcm, const void *data,
		       unsigned int frame_count)
{
	return pcm_generic_transfer(pcm, (void *)data, frame_count);
}

int pcm_wrapper_write(pcm_wrapper_t *pcm, const void *data, unsigned int count)
{
	unsigned int requested_frames = count /
					pcm->config.channels /*ch*/ /
					(pcm_format_to_bits(pcm->config.format) /* format */ /
					8 /* bits to Byte*/);
	int ret = 0;

	pcm_write(pcm->alsa_pcm, data, count);

	if (pcm->ops)
		ret = pcm_wrapper_writei(pcm, data, requested_frames);

	if (ret < 0)
		return ret;

	ret = ((unsigned int)ret == requested_frames) ? 0 : -EIO;
	return ret;
}

int pcm_wrapper_start(pcm_wrapper_t *pcm)
{
	int32_t ret;
	ret = pcm_ioctl_all(pcm, SNDRV_PCM_IOCTL_START, NULL);
	if (ret < 0)
		AALOGE("cannot start channel");

	return ret;
}

int pcm_wrapper_readi(pcm_wrapper_t *pcm, void *data, unsigned int frame_count)
{
	if (!(pcm->flags & PCM_IN))
		return -EINVAL;

	return pcm_generic_transfer(pcm, data, frame_count);
}

int pcm_wrapper_read(pcm_wrapper_t *pcm, void *data, unsigned int count)
{
	unsigned int requested_frames = count /
					pcm->config.channels /*ch*/ /
					(pcm_format_to_bits(pcm->config.format) /* format */ /
					8 /* bits to Byte*/);
	int ret = 0;
	ret = pcm_read(pcm->alsa_pcm, data, count);
	if (ret < 0)
		return ret;

	if (pcm->ops) {
		ret = pcm_wrapper_readi(pcm, data, requested_frames);
	}

	return ((unsigned int)ret == requested_frames) ? 0 : -EIO;
}

int pcm_wrapper_set_config(pcm_wrapper_t *pcm, const struct pcm_config *config)
{
	int ret = 0;
	if (pcm == NULL)
		return -EFAULT;
	else if (config == NULL) {
		config = &pcm->config;
		pcm->config.channels = 2;
		pcm->config.rate = 48000;
		pcm->config.period_size = 1024;
		pcm->config.period_count = 4;
		pcm->config.format = PCM_FORMAT_S16_LE;
		pcm->config.start_threshold =
		    config->period_count * config->period_size;
		pcm->config.stop_threshold =
		    config->period_count * config->period_size;
		pcm->config.silence_threshold = 0;
		pcm->config.silence_size = 0;
	} else
		pcm->config = *config;

	struct snd_pcm_hw_params params;
	param_init(&params);
	param_set_mask(&params, SNDRV_PCM_HW_PARAM_FORMAT,
		       pcm_format_to_alsa(config->format));
	param_set_min(&params, SNDRV_PCM_HW_PARAM_PERIOD_SIZE,
		      config->period_size);
	param_set_int(&params, SNDRV_PCM_HW_PARAM_CHANNELS, config->channels);
	param_set_int(&params, SNDRV_PCM_HW_PARAM_PERIODS,
		      config->period_count);
	param_set_int(&params, SNDRV_PCM_HW_PARAM_RATE, config->rate);
	AALOGI("Set plug: format: %d, ch: %d, rate: %d, period_size: %d",
			config->format,
			config->channels,
			config->rate,
			config->period_size);
	ret = pcm_ioctl_all(pcm, SNDRV_PCM_IOCTL_HW_PARAMS, &params);
	if (ret < 0) {
		AALOGE("cannot set hw params");
		return ret;
	}
	return 0;
}
pcm_wrapper_t *pcm_wrapper_open(unsigned int card, unsigned int device,
				unsigned int flags,
				const struct pcm_config *config)
{
	pcm_wrapper_t *pcm = NULL;
	char prop_value[PROPERTY_VALUE_MAX];
	pcm = calloc(1, sizeof(pcm_wrapper_t));
	if (!pcm)
		return NULL;

	pcm->flags = flags;

	property_get(plug_prop_enable, prop_value, "false");
	if (!strcmp(prop_value, "true"))
	{
		AALOGI("%s value: %s", plug_prop_enable, prop_value);
		pcm->ops = &plug_ops;
		pcm->fd = pcm->ops->open(card, device, flags, &pcm->data, pcm->snd_node);
		if (pcm_wrapper_set_config(pcm, config) != 0)
			AALOGE("pcm_wrapper_set_config failed");
	}
	pcm->alsa_pcm = pcm_open(card, device, flags,
		(struct pcm_config *)config);

	return pcm;
}

int pcm_wrapper_prepare(pcm_wrapper_t *pcm)
{
	int ret = 0;
	ret = pcm_ioctl_all(pcm, SNDRV_PCM_IOCTL_PREPARE, NULL);
	if (ret < 0) {
		AALOGE("cannot prepare channel");
		return -1;
	}
	return 0;
}

int pcm_wrapper_close(pcm_wrapper_t *pcm)
{
	int ret = -1;
	if(pcm->ops)
		pcm->ops->close(pcm->data);
	pcm->buffer_size = 0;
	pcm->fd = -1;
	if (pcm->alsa_pcm) ret = pcm_close(pcm->alsa_pcm);
	free(pcm);
	return ret;
}
struct pcm * pcm_wrapper_get_handler(pcm_wrapper_t *pcm)
{
	if(NULL == pcm)
		return NULL;
	return pcm->alsa_pcm;
}