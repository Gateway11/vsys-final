#include <stdio.h>
#include <stdint.h>

#ifndef __SPIDEV_API_H__
#define __SPIDEV_API_H__

__BEGIN_DECLS

/*
 * Open the SPI device
 * Returns:
 * - File descriptor of the opened SPI device, or 0 on failure
 */
uint64_t spi_open();

/*
 * Read data from SPI device
 * Parameters:
 * - fd: File descriptor of the SPI device
 * - buffer: Buffer to store the read data
 * - len: Number of bytes to read
 * Returns:
 * - Number of bytes successfully read, or -1 on error
 */
ssize_t spi_read(uint64_t fd, void *buffer, size_t len);

/*
 * Write data to SPI device
 * Parameters:
 * - fd: File descriptor of the SPI device
 * - buffer: Data buffer to write
 * - len: Number of bytes to write
 * Returns:
 * - Number of bytes successfully written, or -1 on error
 */
ssize_t spi_write(uint64_t fd, const void *buffer, size_t len);

/*
 * Transfer data to and from SPI device simultaneously
 * Parameters:
 * - fd: File descriptor of the SPI device
 * - tx_buffer: Data buffer to write
 * - rx_buffer: Buffer to store the read data
 * - len: Number of bytes to transfer
 * Returns:
 * - Number of bytes successfully transferred, or -1 on error
 */
ssize_t spi_transfer(uint64_t fd, const void *tx_buffer, void *rx_buffer, size_t len);

/*
* Enumerated structure defining the keys for SPI control parameters.
*/
typedef enum {
    SPI_TIMEOUT_SEC = 0, // Timeout parameter (default value is 10 seconds).
    // Add more parameters here as needed
} spi_param_key_t;

/*
 * Control function to set various parameters for SPI communication.
 * Parameters are passed as key-value pairs, where the key is an enumerated
 * structure and the value is a void pointer.
 * Parameters:
 * - fd: File descriptor of the SPI device
 * - key: An enumerated structure specifying the parameter to be set.
 * - val: A void pointer to the value to be set for the specified parameter.
 * Returns:
 * - 0 if the operation is successful.
 * - -1 if an error occurs.
 */
int spi_control(uint64_t fd, spi_param_key_t key, void *val);

/*
 * Close the SPI device
 * Parameters:
 * - fd: File descriptor of the SPI device
 * Returns:
 * - 0 on success, -1 on error
 */
int spi_close(uint64_t fd);

__END_DECLS

#endif /*__SPIDEV_API_H__*/
