/*******************************************************************************
Copyright (c) 2023 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensors.
******************************************************************************
 * @file:    cpeng.h
 * @brief:   This  header file contains structure definitions for CP commands and events
 * Developed by: Automotive Software and Systems team, Bangalore, India
*****************************************************************************/
/** \addtogroup CP_Engine
 *  @{
 */
#ifndef __CPENG_H__
#define __CPENG_H__


/*======================= D E F I N E S =======================*/
#define A2B_CP_SLAVE_SELECT 		0x01u

#define CP_SHADOW_REGISTER_SPACE 	0x81u
#define CP_REG_SRM					0x18u
/* SPI commands */
#define CP_SPI_WRITEREG          0x00u
#define CP_SPI_READREG           0x01u
#define CP_SPI_FIFOREAD          0x02u
#define CP_SPI_READSTATUS        0x04u
#define CP_SPI_ENTERBULK         0x05u
#define CP_SPI_EXITBULK          0x06u
#define CP_SPI_BULKWRITE         0x07u
#define CP_SPI_RESETOFFSET       0x08u
#define CP_SPI_ABORT             0x0Au

#define GET_REM_BYTES(_nRemByte_, _nVal_, _nCount_) {\
					(_nRemByte_) = (_nVal_) - ((_nCount_) + 2);\
}


/*======================= D A T A T Y P E S =======================*/

/*! \struct a2b_CpPortStat
    Structure to hold the port status parameters
*/

typedef struct {
	a2b_Byte authStat ;			/*!< Authentication status indication for a given port */
	a2b_Byte encStat ;			/*!< Encryption status indication for a given port */
	a2b_Byte muteStat ;			/*!< Mute status indication for a given port */
}a2b_CpPortStat;


/*! \struct A2B_CP_I2C_PARAM
    Structure to hold the CP Engine node parameters
*/
typedef struct
{
	a2b_Int16 nNodeAddr;		/*!< Node address of the CP Engine */
	a2b_UInt16 nI2cAddr;		/*!< I2C address of the CP Engine */
	a2b_UInt16 nRegAddIdx;		/*!< Address index of the registers */
	a2b_UInt8 nDataVal;			/*!< Value to be written into the register */
} A2B_CP_I2C_PARAM;


/*! \enum ADI_A2B_CP_CMD
Command to CP Engine
*/
typedef enum
{
	A2B_CP_START_AKE = 0,		/*!< Command to start AKE for the given port */
	A2B_CP_START_ENCRYPTION,	/*!< Command to start encryption for the given port */
	A2B_CP_STOP_ENCRYPTION,		/*!< Command to stop encryption for the given port */
	A2B_CP_MUTE,				/*!< Command to mute the given port */
	A2B_CP_PORT_UNMUTE			/*!< Command to unmute encryption for the given port */
}ADI_A2B_CP_CMD;

/*! \enum ADI_A2B_EXIT_LEN
Length to be used to exit from the Bulk Mode
*/
typedef enum
{
	SPI_DMA_EXIT_32_BYTES = 31,		/*!< SPI Bulk config - 0  */
	SPI_DMA_EXIT_64_BYTES = 63,		/*!< SPI Bulk config - 1  */
	SPI_DMA_EXIT_128_BYTES = 127,	/*!< SPI Bulk config - 2  */
	SPI_DMA_EXIT_256_BYTES = 255	/*!< SPI Bulk config - 3  */

}ADI_A2B_EXIT_LEN;


/*! \enum ADI_A2B_CP_RESULT
Enumeration for CP API return values
*/
typedef enum
{
    ADI_A2B_CP_SUCCESS,			/*!< API execution success	*/
    ADI_A2B_CP_FAILED			/*!< API execution failed	*/
}ADI_A2B_CP_RESULT;

/*! \enum SPI_STATUS
SPI Slave status
*/
typedef enum
{
	SPI_STATUS_OK,
	SPI_STATUS_BUSY,
	SPI_STATUS_ERROR,
	SPI_STATUS_BADCMD
}SPI_STATUS;

/*! \enum SPI_BULKPARAM
SPI Bulk mode parameter
*/
typedef enum
{
	SPI_DMA_32_BYTES = 0u,         /*!< SPI Bulk config - 0  */
	SPI_DMA_64_BYTES = 1u,         /*!< SPI Bulk config - 1  */
	SPI_DMA_128_BYTES = 2u,        /*!< SPI Bulk config - 2  */
	SPI_DMA_256_BYTES = 3u         /*!< SPI Bulk config - 3  */
}SPI_BULKPARAM;

a2b_HResult a2b_CPregWrite(struct a2b_StackContext* ctx, a2b_Int16   nNodeAddr, a2b_UInt16 nWrCount, a2b_Byte* wBuf);
a2b_HResult a2b_CPregWriteRead(struct a2b_StackContext* ctx, a2b_Int16   nNodeAddr, a2b_UInt16 nWrCount, a2b_Byte* wBuf,  a2b_UInt16 nRdCount, a2b_Byte* pRdBuf );
a2b_HResult a2b_getCPPortStat(struct a2b_StackContext* ctx, a2b_Int16   nNodeAddr, a2b_Byte nPort, a2b_CpPortStat* pCPPortStat);
a2b_HResult a2b_CPEventTrig(struct a2b_StackContext* ctx, a2b_Int16  nNode, ADI_A2B_CP_CMD eCommand, a2b_Byte nPort);
#endif	/*__CPENG_H__*/

/**@}*/
