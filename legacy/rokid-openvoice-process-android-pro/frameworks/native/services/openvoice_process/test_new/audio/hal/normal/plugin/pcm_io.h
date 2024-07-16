#ifndef AUDIO_HW_SRC_PCM_IO_H
#define AUDIO_HW_SRC_PCM_IO_H

#include <poll.h>
#include <sound/asound.h>

struct snd_node;

struct pcm_ops {
	int (*open)(unsigned int card, unsigned int device, unsigned int flags,
		    void **data, struct snd_node *node);
	void (*close)(void *data);
	int (*ioctl)(void *data, unsigned int cmd, ...);
	void *(*mmap)(void *data, void *addr, size_t length, int prot,
		      int flags, off_t offset);
	int (*munmap)(void *data, void *addr, size_t length);
	int (*poll)(void *data, struct pollfd *pfd, nfds_t nfds, int timeout);
};

extern const struct pcm_ops hw_ops;
extern const struct pcm_ops plug_ops;

#endif /* TINYALSA_SRC_PCM_IO_H */
