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
 * \file:   spi.h
 * \author: Analog Devices, bangalore
 * \brief:  This defines the SPI interface used by A2B stack application and
 *          plugins.
 *
 *=============================================================================
 */

/*============================================================================*/
/**
 * \defgroup a2bstack_spi       SPI Module
 *  
 * The types and associated APIs providing public low-level SPI access.
 *  
 * The StackContext provides a mechanism for enforcing access control.
 * This means that only a master plugin can access the master, slave, or
 * peripheral SPI read/write routines. A slave plugin can only read/write to
 * peripherals attached to its node. An "application" context can also
 * read/write to peripherals attached via SPI to <em>any</em> slave node.
 *  
 * \{ */
/*============================================================================*/

#ifndef A2B_SPI_H_
#define A2B_SPI_H_

/*======================= I N C L U D E S =========================*/

#include "a2b/macros.h"
#include "a2b/ctypes.h"

/*----------------------------------------------------------------------------*/
/**
 * \defgroup a2bstack_spi_defs          Types/Defs
 *  
 * The various defines and data types used within the SPI modules.
 *
 * \{ */
/*----------------------------------------------------------------------------*/

/*======================= D E F I N E S ===========================*/

/*======================= D A T A T Y P E S =======================*/

A2B_BEGIN_DECLS

/* Forward declarations */
struct a2b_StackContext;

/** \} -- a2bstack_spi_defs */

#define A2B_SPI_SLV_SEL										(0x00u) /*!< SPI Slave Select of the A2B Transceiver */

#define A2B_REMOTEDEVCONFIG_SPITOSPI_MAX_TRANS_SZ_INBYTES 	(256U)	/*!< SPI to SPI peripheral maximum transaction size */
#define A2B_REMOTEDEVCONFIG_SPITOSPI_MAX_ADDR_WIDTH_INBYTES	(4U)	/*!< 4 bytes are sufficient to hold a 32-bit address. */
#define A2B_REMOTEDEVCONFIG_SPITOSPI_TIMEOUT_IN_MS			(10u)	/*!< Timeout for SPI to SPi peripheral configuration timeout specified in milliseconds */
#define A2B_MAX_LENINBYTES_SPI_TUNNEL_WRITE					(A2B_REMOTEDEVCONFIG_SPITOSPI_MAX_TRANS_SZ_INBYTES + 3U)

#if !defined(A2B_SPI_MAX_BUSY_CHECK_CNT)
/* max number of Busy Checks  */
#define A2B_SPI_MAX_BUSY_CHECK_CNT  (256u)
#endif
/*! \enum A2B_SPI_EVENT
SPI event type
*/
typedef enum A2B_SPI_EVENT
{
	A2B_SPI_EVENT_RX_MSG,        			/*!< Message received event indication */
	A2B_SPI_EVENT_TX_DONE,       			/*!< Transmission finished event indication	*/
	A2B_SPI_EVENT_FD_MSG,					/*!< Full duplex message finished event indication	*/
	A2B_SPI_EVENT_RX_TIMEOUT,    			/*!< Reception timeout event indication	*/
	A2B_SPI_EVENT_TX_TIMEOUT,    			/*!< Transmission timeout event indication	*/
	A2B_SPI_EVENT_FD_TIMEOUT,    			/*!< Full duplex timeout event indication	*/
	A2B_SPI_EVENT_REMOTE_REG_ACCESS_ERROR,	/*!< SPI remote register access error */
	A2B_SPI_EVENT_REMOTE_I2C_ACCESS_ERROR,  /*!< SPI remote I2C access error */
	A2B_SPI_EVENT_DATA_TUNNEL_SPIBUSY,		/*!< SPI Peripheral Busy */
	A2B_SPI_EVENT_DATA_TUNNEL_ACTIVE,		/*!< Data Tunnel Transaction Active */
	A2B_SPI_EVENT_DATA_TUNNEL_INVALID, 		/*!< Data Tunnel Invalid Configuration */
	A2B_SPI_EVENT_DATA_TUNNEL_ABORT,		/*!< Data Tunnel Transaction Aborted */
	A2B_SPI_EVENT_DATA_TUNNEL_BADPKT,		/*!< Data Tunnel Bad Packet Detected */
	A2B_SPI_EVENT_BAD_COMMAND,              /*!< SPI bad command */
	A2B_SPI_EVENT_FIFO_OVERFLOW,            /*!< SPI FIFO overflow */
	A2B_SPI_EVENT_FIFO_UNDERFLOW,           /*!< SPI FIFO underflow */
	A2B_SPI_EVENT_FAILURE        			/*!< Generic failure indication	*/
}A2B_SPI_EVENT;


/*! \enum A2B_SPI_MODE
SPI mode which indicates the type of SPI transaction to be used
*/
typedef enum A2B_SPI_MODE
{
	A2B_SPI_ATOMIC,        			/*!< Use Atomic SPI transactions allow a write or read initiated on any node in an A2B system to occur at a peripheral on a different node */
	A2B_SPI_BULK,        			/*!< Use Bulk SPI transactions */
	A2B_SPI_FD_CMD_BASED,       	/*!< Use Full duplex SPI transaction - slave select 0 (ADR1) selects the SPI slave from the host */
	A2B_SPI_FD_REG_BASED,    		/*!< Use Full duplex register-based SPI transaction – the A2B node is selected by either the ADR2 or SIO2 pin */
	A2B_SPI_BULK_EXTENDED,			/*!< Use Bulk Extended SPI transactions */
	A2B_SPI_FD_CMD_BASED_EXTENDED	/*!< Use Full duplex Extended SPI transaction - slave select 0 (ADR1) selects the SPI slave from the host */

}A2B_SPI_MODE;

/* Status call back  to indicate events to higher layers */
typedef void (*pfStatusCb)(void* pCbParam, A2B_SPI_EVENT eEventType, a2b_Int8 nNodeAddr);

/*! \enum A2B_API_BLOCK_MODE
Enumeration for A2B API execution modes
*/
typedef enum A2B_API_BLOCK_MODE
{
	A2B_API_BLOCKING,      		/*!< API returns only after entire payload is Tx'ed/Rx'ed */
	A2B_API_NON_BLOCKING 		/*!< API returns immediately and does not wait for entire payload to be Tx'ed/Rx'ed	*/
} A2B_API_BLOCK_MODE;

/*! \enum A2B_DT_OPTIMIZATION_MODE
Enumeration for A2B SPI DT Optimization modes
*/
typedef enum A2B_DT_OPTIMIZATION_MODE
{
	A2B_SPI_DT_NO_OPTIMIZE_FOR_SPEED,  	/*!< API returns only after entire payload is Tx'ed/Rx'ed */
	A2B_SPI_DT_OPTIMIZE_FOR_SPEED 		/*!< API returns immediately and does not wait for entire payload to be Tx'ed/Rx'ed	*/
} A2B_DT_OPTIMIZATION_MODE;

/*! \struct a2b_SpiConfig
    Strcuture holding the various configurable parameters of SPI
*/
typedef struct a2b_SpiConfig
{
	/*!< Pointer to status call back function used for indicating events to application */
	pfStatusCb			pfStatCb;

	/*!< Pointer to callback parameter passed during the callback */
	void*				pCbParam;

	/*!< SPI mode of operation */
	A2B_SPI_MODE		eSpiMode;

	/*!< API mode of operation */
	A2B_API_BLOCK_MODE	eApiMode;

	/*!< Data Tunnel Optimization flag */
	A2B_DT_OPTIMIZATION_MODE	eDTOptimizationMode;
}a2b_SpiConfig;




/*! \struct a2b_SpiWrRdParams
    Strcuture holding the various write/read parameters of SPI.
	Set nReadBytes: 0 & *pRBuf: NULL if write operation is required.
*/
typedef struct a2b_SpiWrRdParams
{
	/*!< A2B node number, Master: -1 , Slave 0: 0, Slave 1: 1 etc */
	a2b_Int16 		nNode;

	/*!< Slave select of the peripheral connected to A2B node */
	a2b_UInt8		nSlaveSel;

	/*!< Number of bytes to be written */
	a2b_UInt16		nWriteBytes;

	/*!< Pointer to write buffer. This buffer shall be managed by the application and should be kept alive until the entire transaction is complete */
	a2b_UInt8     	*pWBuf;

	/*!< Number of bytes to be read */
	a2b_UInt16		nReadBytes;

	/*!< Pointer to read buffer. This buffer shall be managed by the application and should be kept alive until the entire transaction is complete */
	a2b_UInt8     	*pRBuf;

	/*!< Indicates the  address increment (in bytes) in the write buffer. This field is MUST for SPI transaction */
	a2b_UInt8		nAddrIncrement;

	/*!< Indicates the address width (in bytes) in the write buffer. This field is MUST for SPI transaction */
	a2b_UInt8		nAddrWidth;

	/*!< Indicates the offset of the address (in bytes) in the write buffer. This field is MUST for SPI transaction */
	a2b_UInt8		nAddrOffset;
}a2b_SpiWrRdParams;


/*----------------------------------------------------------------------------*/
/**
* \ingroup         a2bstack_spi
*
* The fundamental SPI commands supported by the A2B stack.
*/
/*----------------------------------------------------------------------------*/
typedef enum
{
	A2B_CMD_SPI_LOCAL_REG_WRITE 						= 0x00,
	A2B_CMD_SPI_LOCAL_REG_READ 							= 0x01,
	A2B_CMD_SPI_SLAVE_REG_WRITE 						= 0x02,
	A2B_CMD_SPI_DATA_TUNNEL_ATOMIC_WRITE 				= 0x03,
	A2B_CMD_SPI_STATUS_READ 							= 0x04,
	A2B_CMD_SPI_BUS_FIFO_READ 							= 0x05,
	A2B_CMD_SPI_DATA_TUNNEL_BULK_WRITE 					= 0x06,
	A2B_CMD_SPI_REMOTE_I2C_WRITE 						= 0x07,
	A2B_CMD_SPI_REMOTE_I2C_READ_REQUEST 				= 0x08,
	A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_CMD_BASED		= 0x09,
	A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_REG_BASED		= 0x99,
	A2B_CMD_SPI_ABORT 									= 0x0A,
	A2B_CMD_SPI_DATA_TUNNEL_FIFO_READ 					= 0x0B,
	A2B_CMD_SPI_DATA_TUNNEL_ATOMIC_LARGE_WRITE 			= 0x0C,
	A2B_CMD_SPI_DATA_TUNNEL_ATOMIC_LARGE_READ_REQUEST 	= 0x0D,
	A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_EXTENDED		= 0x0E,
	A2B_CMD_SPI_DATA_TUNNEL_BULK_WRITE_EXTENDED			= 0x0F,
	A2B_CMD_SPI_SLAVE_REG_READ_REQUEST		 			= 0xC0,			/* 0xC0 - 0xDF */
	A2B_CMD_SPI_DATA_TUNNEL_ATOMIC_READ_REQUEST 		= 0xE0,			/* 0xE0 - 0xFF */
} a2b_SpiCmd;

/*===========================================================================
 *
 * Define the *synchronous* SPI API
 *
 * ========================================================================*/
A2B_DSO_LOCAL a2b_SpiCmd  a2b_getSpiCmd(struct a2b_StackContext *pCtx, A2B_SPI_MODE eSpiMode, a2b_Bool bWrRd);

/*----------------------------------------------------------------------------*/
/**
 * \defgroup a2bstack_spi_master        Master Node SPI Synchronous Access
 *  
 * SPI API to synchronously read/write to/from the A2B master node.
 *
 * \{ */
/*----------------------------------------------------------------------------*/

A2B_DSO_PUBLIC a2b_HResult a2b_spiMasterWrite(struct a2b_StackContext* ctx, a2b_UInt16 nWrite, void* wBuf);
A2B_DSO_PUBLIC a2b_HResult a2b_spiMasterWriteRead(struct a2b_StackContext* ctx, a2b_UInt16 nWrite, void* wBuf, a2b_UInt16 nRead, void* rBuf);


/** \} -- a2bstack_spi_master */

/*----------------------------------------------------------------------------*/
/**
 * \defgroup a2bstack_spi_slave         Slave Node SPI Synchronous Access
 *  
 * SPI API to synchronously read/write to/from the A2B slave node.
 *
 * \{ */
/*----------------------------------------------------------------------------*/

A2B_DSO_PUBLIC a2b_HResult a2b_spiSlaveWrite(struct a2b_StackContext* ctx, a2b_Int16 node, a2b_UInt16 nWrite, void* wBuf);
A2B_DSO_PUBLIC a2b_HResult a2b_spiSlaveWriteRead(struct a2b_StackContext* ctx, a2b_Int16 node, a2b_UInt16 nWrite, void* wBuf, a2b_UInt16 nRead, void* rBuf);
A2B_DSO_PUBLIC a2b_HResult a2b_spiSlaveBroadcastWrite(struct a2b_StackContext* ctx, a2b_UInt16 nWrite, void* wBuf);

/** \} -- a2bstack_spi_slave */

/*----------------------------------------------------------------------------*/
/**
 * \defgroup a2bstack_spi_periph        Peripheral Device Synchronous SPI Access
 *  
 * SPI API to synchronously read/write to/from the A2B node peripheral.
 *
 * \{ */
/*----------------------------------------------------------------------------*/

A2B_DSO_PUBLIC a2b_HResult a2b_spiPeriphWrite(struct a2b_StackContext* ctx, a2b_Int16 node, a2b_UInt16 spiCmd, a2b_UInt16 chipAddr, a2b_UInt16 spiSs, a2b_UInt16 nWrite, void* wBuf);
A2B_DSO_PUBLIC a2b_HResult a2b_spiPeriphWriteRead(struct a2b_StackContext* ctx, a2b_Int16 node, a2b_UInt16 spiCmd, a2b_UInt16 chipAddr, a2b_UInt16 spiSs, a2b_UInt16 nWrite, void* wBuf, a2b_UInt16 nRead, void* rBuf);

/** \} -- a2bstack_spi_periph */

A2B_DSO_PUBLIC a2b_HResult a2b_spiStatusRead(struct a2b_StackContext* ctx, a2b_UInt16 spiSs, a2b_UInt16 nRead, a2b_Byte *rbuf);
A2B_DSO_PUBLIC a2b_HResult a2b_spiDtFifoRead(struct a2b_StackContext* ctx, a2b_UInt16 nRead, a2b_Byte *rbuf);
A2B_DSO_PUBLIC a2b_UInt8   a2b_prepSpiSsNodeByte(a2b_Int16 nNodeaddr, a2b_UInt16 nTrgtMstrSlave, a2b_UInt16 nSpiSs);
A2B_DSO_PUBLIC a2b_UInt8   a2b_spiCheckBusyStat(struct a2b_StackContext* ctx, a2b_UInt16 spiSs, a2b_UInt32 maxTryCnt);
A2B_DSO_PUBLIC a2b_HResult a2b_spiAbort             (struct a2b_StackContext* ctx, a2b_UInt16 spiSs);

A2B_DSO_PUBLIC a2b_HResult adi_a2b_spiPeriCreate	(struct a2b_StackContext *pCtx);
A2B_DSO_PUBLIC a2b_HResult adi_a2b_spiPeriSetMode	(struct a2b_StackContext *pCtx, a2b_SpiConfig *pSpiConfig);
A2B_DSO_PUBLIC a2b_HResult adi_a2b_spiPeriGetMode	(struct a2b_StackContext *pCtx, A2B_SPI_MODE *peSpiMode, A2B_API_BLOCK_MODE *peApiMode);
A2B_DSO_PUBLIC a2b_HResult adi_a2b_spiPeriWrRd		(struct a2b_StackContext *pCtx, a2b_SpiWrRdParams *pSpiWrRdParams);

A2B_EXPORT A2B_DSO_LOCAL void	 	a2b_remoteDevConfigSpiToSpiStartTimer(struct a2b_StackContext *pCtx);
A2B_EXPORT A2B_DSO_LOCAL void 		a2b_spiPeriInterrupt	(struct a2b_StackContext* pCtx, a2b_Handle pHnd, a2b_UInt8 nIntrSrc, a2b_UInt8 nIntrType);


A2B_END_DECLS

/*======================= D A T A =================================*/

/** \} -- a2bstack_spi */

#endif /* A2B_SPI_H_ */
