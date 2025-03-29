/*=============================================================================
*
* Project: a2bstack
*
* Copyright (c) 2023 - Analog Devices Inc. All Rights Reserved.
* This software is subject to the terms and conditions of the license set
* forth in the project LICENSE file. Downloading, reproducing, distributing or
* otherwise using the software constitutes acceptance of the license. The
* software may not be used except as expressly authorized under the license.
*
*=============================================================================
*
* \file:   spi_priv.h
* \author: Automotive Software Team, Bangalore
* \brief:  These are private definitions for the A2B public SPI API.
*
*=============================================================================
*/

#ifndef A2B_SPI_PRIV_H_
#define A2B_SPI_PRIV_H_

/*======================= I N C L U D E S =========================*/

#include "a2b/macros.h"
#include "a2b/ctypes.h"
#include "a2b/conf.h"
#include "a2b/spi.h"

/*======================= D E F I N E S ===========================*/

#define A2B_CMD_MIN_LENINBYTES_SPI_LOCAL_REG_WRITE							(1u+1u)	/* 1 Addr byte + Min 1 byte	*/
#define A2B_CMD_MIN_LENINBYTES_SPI_LOCAL_REG_READ							(1u)	/* Min 1 byte read */
#define A2B_CMD_MIN_LENINBYTES_SPI_SLAVE_REG_WRITE							(1u+1u)	/* 1 Addr byte + Min 1 byte	*/
#define A2B_CMD_MIN_LENINBYTES_SPI_DATA_TUNNEL_ATOMIC_WRITE					(3u)
#define A2B_CMD_MIN_LENINBYTES_SPI_STATUS_READ								(2u)
#define A2B_CMD_MIN_LENINBYTES_SPI_BUS_FIFO_READ							(2u)
#define A2B_CMD_MIN_LENINBYTES_SPI_TUNNEL_BULK_WRITE						(1u)	/* Min 1 byte write */
#define A2B_CMD_MIN_LENINBYTES_SPI_REMOTE_I2C_WRITE							(1u)	/* Min 1 byte write */
#define A2B_CMD_MIN_LENINBYTES_SPI_REMOTE_I2C_READ_REQUEST					(1u)	/* Min 1 byte read */
#define A2B_CMD_MIN_LENINBYTES_SPI_DATA_TUNNEL_FULL_DUPLEX_CMD_BASED		(1u)	/* Min 1 byte transaction size */
#define A2B_CMD_MIN_LENINBYTES_SPI_DATA_TUNNEL_FULL_DUPLEX_REG_BASED		(1u)	/* Min 1 byte transaction size */
#define A2B_CMD_MIN_LENINBYTES_SPI_ABORT									(1u)
#define A2B_CMD_MIN_LENINBYTES_SPI_DATA_TUNNEL_FIFO_READ					(2u)
#define A2B_CMD_MIN_LENINBYTES_SPI_DATA_TUNNEL_ATOMIC_LARGE_WRITE			(1u)	/* Min 1 byte write */
#define A2B_CMD_MIN_LENINBYTES_SPI_DATA_TUNNEL_ATOMIC_LARGE_READ_REQUEST	(1u)	/* Min 1 byte write */
#define A2B_CMD_MIN_LENINBYTES_SPI_SLAVE_REGISTER_READ_REQUEST				(1u)	/* Min 1 byte read */
#define A2B_CMD_MIN_LENINBYTES_SPI_DATA_TUNNEL_ATOMIC_READ_REQUEST			(3u)
					
#define A2B_CMD_MAX_LENINBYTES_SPI_LOCAL_REG_WRITE							(1u+32u)	/* 1 Addr byte + Max 32 bytes	*/
#define A2B_CMD_MAX_LENINBYTES_SPI_LOCAL_REG_READ							(32u)	/* Max 32 bytes read */
#define A2B_CMD_MAX_LENINBYTES_SPI_SLAVE_REG_WRITE							(1u+32u)	/* 1 Addr byte + Max 32 bytes	*/
#define A2B_CMD_MAX_LENINBYTES_SPI_DATA_TUNNEL_ATOMIC_WRITE					(34u)
#define A2B_CMD_MAX_LENINBYTES_SPI_STATUS_READ								(2u)
#define A2B_CMD_MAX_LENINBYTES_SPI_BUS_FIFO_READ							(33u)
#define A2B_CMD_MAX_LENINBYTES_SPI_TUNNEL_BULK_WRITE						(256u)
#define A2B_CMD_MAX_LENINBYTES_SPI_REMOTE_I2C_WRITE							(32u)	/* Max 32 bytes write */
#define A2B_CMD_MAX_LENINBYTES_SPI_REMOTE_I2C_READ_REQUEST					(32u)	/* Max 32 bytes read */
#define A2B_CMD_MAX_LENINBYTES_SPI_DATA_TUNNEL_FULL_DUPLEX_CMD_BASED		(256u)	/* Max 256 bytes transaction size */
#define A2B_CMD_MAX_LENINBYTES_SPI_DATA_TUNNEL_FULL_DUPLEX_REG_BASED		(256u)	/* Max 256 bytes transaction size */
#define A2B_CMD_MAX_LENINBYTES_SPI_ABORT									(1u)
#define A2B_CMD_MAX_LENINBYTES_SPI_DATA_TUNNEL_FIFO_READ					(257u)
#define A2B_CMD_MAX_LENINBYTES_SPI_DATA_TUNNEL_ATOMIC_LARGE_WRITE			(256u)	/* Max 256 bytes write */
#define A2B_CMD_MAX_LENINBYTES_SPI_DATA_TUNNEL_ATOMIC_LARGE_READ_REQUEST	(256u)	/* Max 256 bytes write */
#define A2B_CMD_MAX_LENINBYTES_SPI_SLAVE_REGISTER_READ_REQUEST				(32u)	/* Max 32 bytes read */
#define A2B_CMD_MAX_LENINBYTES_SPI_DATA_TUNNEL_ATOMIC_READ_REQUEST			(34u)

#define A2B_CMD_LEN_ATOMIC_WRITE_MODE										(2u)	/* SPI Command overhead */
#define A2B_CMD_LEN_BULK_MODE												(3u)	/* SPI Command overhead */
#define A2B_CMD_LEN_FULL_DUPLEX_MODE										(3u)	/* SPI Command overhead */
#define A2B_CMD_LEN_ATOMIC_READ_MODE										(3u)	/* SPI Command overhead */
/*======================= D A T A T Y P E S =======================*/

A2B_BEGIN_DECLS



A2B_END_DECLS

/*======================= D A T A =================================*/


#endif /* A2B_SPI_PRIV_H_ */
