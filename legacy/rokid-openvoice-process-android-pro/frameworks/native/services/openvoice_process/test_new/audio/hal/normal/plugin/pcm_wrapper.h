#ifndef PCM_WRAPPER_H
#define PCM_WRAPPER_H
#include <cutils/list.h>
typedef struct pcm_wrapper pcm_wrapper_t;
struct plug_node {
	struct listnode list;
	void *plug_data;
};
int pcm_wrapper_start(pcm_wrapper_t *pcm);
int pcm_wrapper_write(pcm_wrapper_t *pcm, const void *data, unsigned int count);
int pcm_wrapper_read(pcm_wrapper_t *pcm, void *data, unsigned int count);
pcm_wrapper_t *pcm_wrapper_open(unsigned int card, unsigned int device,
				unsigned int flags,
				const struct pcm_config *config);
int pcm_wrapper_prepare(pcm_wrapper_t *pcm);
int pcm_wrapper_close(pcm_wrapper_t *pcm);
struct pcm * pcm_wrapper_get_handler(pcm_wrapper_t *pcm);
#endif