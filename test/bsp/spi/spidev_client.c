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

#include <pthread.h>
#include "spidev_api.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define READ_ONLY_ENABLE

static void pabort(const char *s)
{
	perror(s);
	abort();
}

typedef struct spidata_priv {
    int fd;
#ifdef READ_ONLY_ENABLE
    int read_fd;
#endif
    uint32_t timeout_sec;
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

static void hex_dump(const void *src, size_t length, size_t line_size,
		     char *prefix)
{
	int i = 0;
	const unsigned char *address = src;
	const unsigned char *line = address;
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
		.len = len,
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

	if (verbose && tx)
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

	if (verbose && rx)
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

static void transfer_escaped_string(int fd, char *str)
{
	size_t size = strlen(str);
	uint8_t *tx;
	uint8_t *rx;

	tx = malloc(size);
	if (!tx)
		pabort("can't allocate tx buffer");

	rx = malloc(size);
	if (!rx)
		pabort("can't allocate rx buffer");

	size = unescape((char *)tx, str, size);
	transfer(fd, tx, rx, size);
	free(rx);
	free(tx);
}

static void transfer_file(int fd, char *filename)
{
	ssize_t bytes;
	struct stat sb;
	int tx_fd;
	uint8_t *tx;
	uint8_t *rx;

	if (stat(filename, &sb) == -1)
		pabort("can't stat input file");

	tx_fd = open(filename, O_RDONLY);
	if (tx_fd < 0)
		pabort("can't open input file");

	tx = malloc(sb.st_size);
	if (!tx)
		pabort("can't allocate tx buffer");

	rx = malloc(sb.st_size);
	if (!rx)
		pabort("can't allocate rx buffer");

	bytes = read(tx_fd, tx, sb.st_size);
	if (bytes != sb.st_size)
		pabort("failed to read input file");

	transfer(fd, tx, rx, sb.st_size);
	free(rx);
	free(tx);
	close(tx_fd);
}

static uint64_t _read_count;
static uint64_t _write_count;

static void show_transfer_rate(void)
{
	static uint64_t prev_read_count, prev_write_count;
	double rx_rate, tx_rate;

	rx_rate = ((_read_count - prev_read_count) * 8) / (interval*1000.0);
	tx_rate = ((_write_count - prev_write_count) * 8) / (interval*1000.0);

	printf("rate: tx %.1fkbps, rx %.1fkbps\n", rx_rate, tx_rate);

	prev_read_count = _read_count;
	prev_write_count = _write_count;
}

static void transfer_buf(int fd, int len)
{
	uint8_t *tx;
	uint8_t *rx;
	int i;

	tx = malloc(len);
	if (!tx)
		pabort("can't allocate tx buffer");
	for (i = 0; i < len; i++)
		tx[i] = random();

	rx = malloc(len);
	if (!rx)
		pabort("can't allocate rx buffer");

	transfer(fd, tx, rx, len);

	_write_count += len;
	_read_count += len;

	if (mode & SPI_LOOP) {
		if (memcmp(tx, rx, len)) {
			fprintf(stderr, "transfer error !\n");
			hex_dump(tx, len, 32, "TX");
			hex_dump(rx, len, 32, "RX");
			exit(1);
		}
	}

	free(rx);
	free(tx);
}

uint64_t spi_open() {
    static spidata_priv_t spidata;
    spidata.timeout_sec = 10;

	spidata.fd = open(device, O_RDWR);
	if (spidata.fd < 0) {
		pabort("Failed to open the device in read-write mode");
        return 0;
    }
#ifdef READ_ONLY_ENABLE
	spidata.read_fd = open(device, O_RDONLY);
	if (spidata.read_fd < 0) {
		pabort("Failed to open the device in read-only mode");
        close(spidata.fd);
        return 0;
    }
    // Set the file descriptor to blocking mode
    int flags = fcntl(spidata.read_fd, F_GETFL, 0);
    if (flags == -1) {
        perror("Failed to get file descriptor flags");
        return -1;
    }

    // Clear the non-blocking flag
    flags &= ~O_NONBLOCK;

    // Set the file descriptor's attributes
    int ret = fcntl(spidata.read_fd, F_SETFL, flags);
    if (ret == -1) {
        perror("Failed to set file descriptor flags");
        return -1;
    }
#endif
    return (uint64_t)&spidata;
}

int spi_close(uint64_t fd) {
    int ret = -1;
    spidata_priv_t *spidata = (spidata_priv_t*)fd;

    if (spidata != NULL) {
        ret = close(spidata->fd);
#ifdef READ_ONLY_ENABLE
        ret = close(spidata->read_fd);
#endif
    }
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
            spidata->timeout_sec = *timeout;
            printf("Setting timeout to %d\n", *timeout);
            break;
        }
        default:
            // Handle unsupported or invalid key
            return -1;
    }
    return 0;
}

ssize_t spi_read_blocking(int spi_fd, void *buffer, size_t len, int timeout_sec) {
    fd_set recv_fds;
    struct timeval tv;
    ssize_t bytes_read;

    tv.tv_sec = timeout_sec;
    tv.tv_usec = 0;

    FD_ZERO(&recv_fds);
    FD_SET(spi_fd, &recv_fds);

    // Call select to monitor if data is ready to read from the SPI device
    int fd_result = select(spi_fd + 1, &recv_fds, NULL, NULL, &tv);
    if (fd_result < 0) {
        perror("select error");
        return -1;
    } else if (fd_result == 0) {
        // No change in the SPI device's state within the specified time
        printf("select timeout\n");
        return -1;//continue;
    } else {
        // Data is ready to read from the SPI device
        if (FD_ISSET(spi_fd, &recv_fds)) {
            bytes_read = read(spi_fd, buffer, len);
            if (bytes_read < 0) {
                perror("read error");
                return -1;
            }
            return bytes_read;
        }
    }
    return -1;
}

ssize_t spi_read_nonblocking(int spi_fd, void *buffer, size_t len) {
    return read(spi_fd, buffer, len);
}

ssize_t spi_read(uint64_t fd, void *buffer, size_t len) {
    int ret = -1;
    spidata_priv_t *spidata = (spidata_priv_t*)fd;
    if (spidata != NULL)
#ifdef READ_ONLY_ENABLE
        ret = spi_read_blocking(spidata->read_fd, buffer, len, spidata->timeout_sec);
#else
        ret = spi_read_blocking(spidata->fd, buffer, len, spidata->timeout_sec);
#endif
    if (verbose && ret > 0)
		hex_dump(buffer, ret, 32, "RX");
    return ret;
}

ssize_t spi_write(uint64_t fd, const void *buffer, size_t len) {
    int ret = -1;
    spidata_priv_t *spidata = (spidata_priv_t*)fd;
    if (spidata != NULL)
        ret = write(spidata->fd, buffer, len);
    if (verbose && ret > 0)
	    hex_dump(buffer, ret, 32, "TX");
    return ret;
}

ssize_t spi_transfer(uint64_t fd, const void *tx_buffer, void *rx_buffer, size_t len) {
    spidata_priv_t *spidata = (spidata_priv_t*)fd;
    if (spidata != NULL)
        transfer(spidata->fd, tx_buffer, tx_buffer, len);
    return 0;
}

void* th_read_fun(void *arg) {
    uint64_t spi_fd = *(uint64_t *)arg;
    srand((unsigned)time(NULL));
    for(;;) {
        spi_read(spi_fd, default_rx, sizeof(default_rx));
        usleep((rand() % 1000) * 1000);
    }
    return NULL;
}

void* th_write_fun(void *arg) {
    uint64_t spi_fd = *(uint64_t *)arg;
    srand((unsigned)time(NULL));
    for(;;) {
        spi_write(spi_fd, input_tx, strlen(input_tx));
        usleep((rand() % 1000) * 1000);
    }
    return NULL;
}

void* th_transfer_fun(void *arg) {
    uint64_t spi_fd = *(uint64_t *)arg;
    spidata_priv_t *spidata = (spidata_priv_t*)spi_fd;

    srand((unsigned)time(NULL));
    for(;;) {
#define min(a,b) (((a) < (b)) ? (a) : (b))
        spi_transfer(spi_fd, input_tx, default_rx, min(strlen(input_tx), sizeof(default_rx)));
        usleep((rand() % 1000) * 1000);
    }
    return NULL;
}

#ifdef __ANDROID_NDK__
int main(int argc, char *argv[])
{
	int ret = 0;
	int fd;

	parse_opts(argc, argv);

#if 1
    uint64_t spi_fd = spi_open();
	if (input_tx && input_file)
		pabort("only one of -p and --input may be selected");

#if 1
    spi_control(spi_fd, SPI_TIMEOUT_SEC, (uint32_t []){15});

	size_t size = strlen(input_tx);
    ret = spi_write(spi_fd, input_tx, size);

    ret = spi_read(spi_fd, default_rx, sizeof(default_rx));

#define min(a,b) (((a) < (b)) ? (a) : (b))
    ret = spi_transfer(spi_fd, input_tx, default_rx, min(size, sizeof(default_rx)));

#else
    pthread_t th_read, th_write, th_transfer;
    pthread_create(&th_read, NULL, th_read_fun, (void *)&spi_fd);
    pthread_detach(th_read);

    pthread_create(&th_write, NULL, th_write_fun, (void *)&spi_fd);
    pthread_detach(th_write);

    pthread_create(&th_transfer, NULL, th_transfer_fun, (void *)&spi_fd);
    pthread_detach(th_transfer);
#endif

    ret = spi_close(spi_fd);
#else
	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE32, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE32, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	printf("spi mode: 0x%x\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

	if (input_tx && input_file)
		pabort("only one of -p and --input may be selected");

	if (input_tx)
		transfer_escaped_string(fd, input_tx);
	else if (input_file)
		transfer_file(fd, input_file);
	else if (transfer_size) {
		struct timespec last_stat;

		clock_gettime(CLOCK_MONOTONIC, &last_stat);

		while (iterations-- > 0) {
			struct timespec current;

			transfer_buf(fd, transfer_size);

			clock_gettime(CLOCK_MONOTONIC, &current);
			if (current.tv_sec - last_stat.tv_sec > interval) {
				show_transfer_rate();
				last_stat = current;
			}
		}
		printf("total: tx %.1fKB, rx %.1fKB\n",
		       _write_count/1024.0, _read_count/1024.0);
	} else
		transfer(fd, default_tx, default_rx, sizeof(default_tx));

	close(fd);
#endif
	return ret;
}
#endif
