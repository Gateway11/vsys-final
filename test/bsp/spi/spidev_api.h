#include <stdio.h>
#include <stdint.h>

#ifdef __SPIDEV_API_H__
#define __SPIDEV_API_H__

#if 0
// Open the SPI device
// Parameters:
// - device: SPI device identifier or name
// - mode: Mode of SPI communication (e.g., read, write, read/write)
// Returns:
// - File descriptor of the opened SPI device
int spi_open(const char *device, int mode);

// Read data from SPI device
// Parameters:
// - fd: File descriptor of the SPI device
// - buffer: Buffer to store the read data
// - len: Number of bytes to read
// Returns:
// - Number of bytes successfully read, or -1 on error
ssize_t spi_read(int fd, void *buffer, size_t len);

// Write data to SPI device
// Parameters:
// - fd: File descriptor of the SPI device
// - buffer: Data buffer to write
// - len: Number of bytes to write
// Returns:
// - Number of bytes successfully written, or -1 on error
ssize_t spi_write(int fd, const void *buffer, size_t len);

// Transfer data to and from SPI device simultaneously
// Parameters:
// - fd: File descriptor of the SPI device
// - tx_buffer: Data buffer to write
// - rx_buffer: Buffer to store the read data
// - len: Number of bytes to transfer
// Returns:
// - Number of bytes successfully transferred, or -1 on error
ssize_t spi_transfer(int fd, const void *tx_buffer, void *rx_buffer, size_t len);

// Close the SPI device
// Parameters:
// - fd: File descriptor of the SPI device
// Returns:
// - 0 on success, -1 on error
int spi_close(int fd);

#else

// Open the SPI device
// Returns:
// - 0 on success, -1 on failure
int spi_open();

// Read data from SPI device
// Parameters:
// - buffer: Buffer to store the read data
// - len: Number of bytes to read
// Returns:
// - Number of bytes successfully read, or -1 on error
ssize_t spi_read(void *buffer, size_t len);

// Read data from the SPI device based on the blocking flag.
// Parameters:
// - buffer: Buffer to store the read data.
// - len: Number of bytes to read.
// - block_flag: Flag indicating whether the read operation should be blocking or non-blocking.
//               If block_flag is non-zero, the function will block until data is available.
//               If block_flag is zero, the function will return immediately if no data is available.
// Returns:
// - Number of bytes successfully read, or -1 on error.
ssize_t spi_read(void *buffer, size_t len, int block_flag);

// Read data from the SPI device based on the blocking flag with timeout.
// Parameters:
// - buffer: Buffer to store the read data.
// - len: Number of bytes to read.
// - block_flag: Flag indicating whether the read operation should be blocking or non-blocking.
//               If block_flag is non-zero, the function will block until data is available.
//               If block_flag is zero, the function will return immediately if no data is available.
// - timeout_sec: Timeout duration in seconds for blocking reads. This parameter is only effective
//                if block_flag is non-zero. If no data is available within this time, the function
//                returns with a timeout error.
// Returns:
// - Number of bytes successfully read, or -1 on error.
ssize_t spi_read(void *buffer, size_t len, int block_flag, int timeout_sec);

// Read data from the SPI device in a blocking manner with timeout.
// Parameters:
// - buffer: Buffer to store the read data.
// - len: Number of bytes to read.
// - timeout_sec: Timeout duration in seconds. If no data is available
//                within this time, the function returns with a timeout error.
// Returns:
// - Number of bytes successfully read, or -1 on error.
ssize_t spi_read_blocking(void *buffer, size_t len, int timeout_sec);

// Read data from the SPI device in a non-blocking manner.
// Parameters:
// - buffer: Buffer to store the read data.
// - len: Number of bytes to read.
// Returns:
// - Number of bytes successfully read, or -1 on error.
ssize_t spi_read_nonblocking(void *buffer, size_t len);

// Write data to SPI device
// Parameters:
// - buffer: Data buffer to write
// - len: Number of bytes to write
// Returns:
// - Number of bytes successfully written, or -1 on error
ssize_t spi_write(const void *buffer, size_t len);

// Transfer data to and from SPI device simultaneously
// Parameters:
// - tx_buffer: Data buffer to write
// - rx_buffer: Buffer to store the read data
// - len: Number of bytes to transfer
// Returns:
// - Number of bytes successfully transferred, or -1 on error
ssize_t spi_transfer(const void *tx_buffer, void *rx_buffer, size_t len);

// Close the SPI device
// Returns:
// - 0 on success, -1 on failure
int spi_close();
#endif

#endif //__SPIDEV_API_H__
