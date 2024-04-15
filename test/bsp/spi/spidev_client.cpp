/*
 * SPI testing utility (using spidev driver)
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <time.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include <string.h>
#include <thread>
#include <algorithm>
#include "spidev_api.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define min(a,b) (((a) < (b)) ? (a) : (b))

static void pabort(const char *s)
{
	perror(s);
	abort();
}

typedef struct spidata_priv {
#define SPI_MSG_LEN 512
    uint8_t tx_buffer[SPI_MSG_LEN];
    uint8_t rx_buffer[SPI_MSG_LEN];
    std::mutex spi_mutex;
    sem_t read_sem;

    int fd;
    union {
        uint32_t timeout_sec;
        uint32_t delay_ms;
    };
} spidata_priv_t;

static const char *device = "/dev/spidev7.0";
static uint32_t mode;
static uint8_t bits = 8;
static char *input_file;
static char *output_file;
static uint32_t speed = 500000;
static uint16_t delay;
static int verbose;
static int transfer_size;
static int iterations;
static int interval = 5; /* interval in seconds for showing transfer rate */

uint8_t default_tx[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x40, 0x00, 0x00, 0x00, 0x00, 0x95,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xF0, 0x0D,
};

uint8_t default_rx[ARRAY_SIZE(default_tx)] = {0, };
char *input_tx;

static void hex_dump(const uint8_t *src, size_t length, size_t line_size,
		     char *prefix)
{
	int i = 0;
	const uint8_t *address = src;
	const uint8_t *line = address;
	unsigned char c;

	printf("%s | ", prefix);
	while (length-- > 0) {
		printf("%02X ", *address++);
		if (!(++i % line_size) || (length == 0 && i % line_size)) {
			if (length == 0) {
				while (i++ % line_size)
					printf("__ ");
			}
			printf(" | ");  /* right close */
			while (line < address) {
				c = *line++;
				printf("%c", (c < 33 || c == 255) ? 0x2E : c);
			}
			printf("\n");
			if (length > 0)
				printf("%s | ", prefix);
		}
	}
}

/*
 *  Unescape - process hexadecimal escape character
 *      converts shell input "\x23" -> 0x23
 */
static int unescape(char *_dst, char *_src, size_t len)
{
	int ret = 0;
	int match;
	char *src = _src;
	char *dst = _dst;
	unsigned int ch;

	while (*src) {
		if (*src == '\\' && *(src+1) == 'x') {
			match = sscanf(src + 2, "%2x", &ch);
			if (!match)
				pabort("malformed input string");

			src += 4;
			*dst++ = (unsigned char)ch;
		} else {
			*dst++ = *src++;
		}
		ret++;
	}
	return ret;
}

static void transfer(int fd, uint8_t const *tx, uint8_t const *rx, size_t len)
{
	int ret;
	int out_fd;
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = (uint32_t)len,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	if (mode & SPI_TX_QUAD)
		tr.tx_nbits = 4;
	else if (mode & SPI_TX_DUAL)
		tr.tx_nbits = 2;
	if (mode & SPI_RX_QUAD)
		tr.rx_nbits = 4;
	else if (mode & SPI_RX_DUAL)
		tr.rx_nbits = 2;
	if (!(mode & SPI_LOOP)) {
		if (mode & (SPI_TX_QUAD | SPI_TX_DUAL))
			tr.rx_buf = 0;
		else if (mode & (SPI_RX_QUAD | SPI_RX_DUAL))
			tr.tx_buf = 0;
	}

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");

	if (verbose)
		hex_dump(tx, len, 32, "TX");

	if (output_file) {
		out_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
		if (out_fd < 0)
			pabort("could not open output file");

		ret = write(out_fd, rx, len);
		if (ret != len)
			pabort("not all bytes written to output file");

		close(out_fd);
	}

	if (verbose)
		hex_dump(rx, len, 32, "RX");
}

static void print_usage(const char *prog)
{
	printf("Usage: %s [-DsbdlHOLC3vpNR24SI]\n", prog);
	puts("  -D --device   device to use (default /dev/spidev1.1)\n"
	     "  -s --speed    max speed (Hz)\n"
	     "  -d --delay    delay (usec)\n"
	     "  -b --bpw      bits per word\n"
	     "  -i --input    input data from a file (e.g. \"test.bin\")\n"
	     "  -o --output   output data to a file (e.g. \"results.bin\")\n"
	     "  -l --loop     loopback\n"
	     "  -H --cpha     clock phase\n"
	     "  -O --cpol     clock polarity\n"
	     "  -L --lsb      least significant bit first\n"
	     "  -C --cs-high  chip select active high\n"
	     "  -3 --3wire    SI/SO signals shared\n"
	     "  -v --verbose  Verbose (show tx buffer)\n"
	     "  -p            Send data (e.g. \"1234\\xde\\xad\")\n"
	     "  -N --no-cs    no chip select\n"
	     "  -R --ready    slave pulls low to pause\n"
	     "  -2 --dual     dual transfer\n"
	     "  -4 --quad     quad transfer\n"
	     "  -S --size     transfer size\n"
	     "  -I --iter     iterations\n");
	exit(1);
}

static void parse_opts(int argc, char *argv[])
{
	while (1) {
		static const struct option lopts[] = {
			{ "device",  1, 0, 'D' },
			{ "speed",   1, 0, 's' },
			{ "delay",   1, 0, 'd' },
			{ "bpw",     1, 0, 'b' },
			{ "input",   1, 0, 'i' },
			{ "output",  1, 0, 'o' },
			{ "loop",    0, 0, 'l' },
			{ "cpha",    0, 0, 'H' },
			{ "cpol",    0, 0, 'O' },
			{ "lsb",     0, 0, 'L' },
			{ "cs-high", 0, 0, 'C' },
			{ "3wire",   0, 0, '3' },
			{ "no-cs",   0, 0, 'N' },
			{ "ready",   0, 0, 'R' },
			{ "dual",    0, 0, '2' },
			{ "verbose", 0, 0, 'v' },
			{ "quad",    0, 0, '4' },
			{ "size",    1, 0, 'S' },
			{ "iter",    1, 0, 'I' },
			{ NULL, 0, 0, 0 },
		};
		int c;

		c = getopt_long(argc, argv, "D:s:d:b:i:o:lHOLC3NR24p:vS:I:",
				lopts, NULL);

		if (c == -1)
			break;

		switch (c) {
		case 'D':
			device = optarg;
			break;
		case 's':
			speed = atoi(optarg);
			break;
		case 'd':
			delay = atoi(optarg);
			break;
		case 'b':
			bits = atoi(optarg);
			break;
		case 'i':
			input_file = optarg;
			break;
		case 'o':
			output_file = optarg;
			break;
		case 'l':
			mode |= SPI_LOOP;
			break;
		case 'H':
			mode |= SPI_CPHA;
			break;
		case 'O':
			mode |= SPI_CPOL;
			break;
		case 'L':
			mode |= SPI_LSB_FIRST;
			break;
		case 'C':
			mode |= SPI_CS_HIGH;
			break;
		case '3':
			mode |= SPI_3WIRE;
			break;
		case 'N':
			mode |= SPI_NO_CS;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'R':
			mode |= SPI_READY;
			break;
		case 'p':
			input_tx = optarg;
			break;
		case '2':
			mode |= SPI_TX_DUAL;
			break;
		case '4':
			mode |= SPI_TX_QUAD;
			break;
		case 'S':
			transfer_size = atoi(optarg);
			break;
		case 'I':
			iterations = atoi(optarg);
			break;
		default:
			print_usage(argv[0]);
			break;
		}
	}
	if (mode & SPI_LOOP) {
		if (mode & SPI_TX_DUAL)
			mode |= SPI_RX_DUAL;
		if (mode & SPI_TX_QUAD)
			mode |= SPI_RX_QUAD;
	}
}

uint64_t spi_open() {
    static spidata_priv_t spidata;
    spidata.delay_ms = 10;

	spidata.fd = open(device, O_RDWR);
	if (spidata.fd < 0) {
		pabort("Failed to open the device in read-write mode");
        return 0;
    }
    sem_init(&spidata.read_sem, 0, 0);
    std::thread thread([&]{
        for (;;) {
            {
                std::lock_guard<decltype(spidata.spi_mutex)> lg(spidata.spi_mutex);
                transfer(spidata.fd, spidata.tx_buffer, spidata.tx_buffer, SPI_MSG_LEN);
            }
            sem_post(&spidata.read_sem);

            std::this_thread::sleep_for(std::chrono::milliseconds(spidata.delay_ms));
        }
    });
    thread.detach();
    return (uint64_t)&spidata;
}

int spi_close(uint64_t fd) {
    int ret = -1;
    spidata_priv_t *spidata = (spidata_priv_t*)fd;

    if (spidata != NULL)
        ret = close(spidata->fd);
    return ret;
}

int spi_control(uint64_t fd, spi_param_key_t key, void *val) {
    spidata_priv_t *spidata = (spidata_priv_t*)fd;
    if (spidata == NULL)
        return -1;
    switch(key) {
        case SPI_TIMEOUT_SEC: {
            // Cast val to the appropriate type for the specified parameter
            uint32_t *timeout = (uint32_t *)val;
            spidata->delay_ms = *timeout;
            printf("Setting timeout to %d\n", *timeout);
            break;
        }
        default:
            // Handle unsupported or invalid key
            return -1;
    }
    return 0;
}

ssize_t spi_read(uint64_t fd, void *buffer, size_t len) {
    spidata_priv_t *spidata = (spidata_priv_t*)fd;
    sem_wait(&spidata->read_sem);
    if (spidata)
    {
        std::lock_guard<decltype(spidata->spi_mutex)> lg(spidata->spi_mutex);
        memcpy(buffer, spidata->rx_buffer, min(len, SPI_MSG_LEN));
    }
    return len;
}

ssize_t spi_write(uint64_t fd, const void *buffer, size_t len) {
    ssize_t ret = -1;
    spidata_priv_t *spidata = (spidata_priv_t*)fd;
    if (spidata) {
        std::lock_guard<decltype(spidata->spi_mutex)> lg(spidata->spi_mutex);
        ret = write(spidata->fd, buffer, len);
        //memcpy(spidata->tx_buffer, buffer, min(len, SPI_MSG_LEN));
    }
    if (verbose && ret > 0)
	    hex_dump((uint8_t *)buffer, ret, 32, "TX");
    return len;
}

ssize_t spi_transfer(uint64_t fd, const void *tx_buffer, void *rx_buffer, size_t len) {
    spidata_priv_t *spidata = (spidata_priv_t*)fd;
    if (spidata) {
        std::lock_guard<decltype(spidata->spi_mutex)> lg(spidata->spi_mutex);
        transfer(spidata->fd, (const uint8_t*)tx_buffer, (uint8_t*)tx_buffer, len);
    }
    return 0;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	int fd;

	parse_opts(argc, argv);

    uint64_t spi_fd = spi_open();
	if (input_tx && input_file)
		pabort("only one of -p and --input may be selected");

    spi_control(spi_fd, SPI_TIMEOUT_SEC, (uint32_t []){500});

	size_t size = strlen(input_tx);
    ret = spi_write(spi_fd, input_tx, size);

    ret = spi_read(spi_fd, default_rx, sizeof(default_rx));

#define min(a,b) (((a) < (b)) ? (a) : (b))
    ret = spi_transfer(spi_fd, input_tx, default_rx, min(size, sizeof(default_rx)));

    ret = spi_close(spi_fd);
	return ret;
}
