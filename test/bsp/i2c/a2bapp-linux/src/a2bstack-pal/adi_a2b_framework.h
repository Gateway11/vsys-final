/*******************************************************************************
Copyright (c) 2014 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensors.
******************************************************************************
* @file: adi_a2b_framework.h
* @brief: This file contains Typedefs,configurable macros and A2B framework structures
* @version: $Revision: 3888 $
* @date: $Date: 2016-01-17 18:46:38 +0530 (Sun, 17 Jan 2016) $
* Developed by: Automotive Software and Systems team, Bangalore, India
*****************************************************************************/
/*! \addtogroup PAL_BF PAL_BF
* @{
*/

/** @defgroup Configuration_Macros_Structures
 *
 * This module has structure definitions, enums and Macros used for framework configuration
 *
 */

/*! \addtogroup Configuration_Macros_Structures Configuration Macros & Structures
* @{
*/

#ifndef _ADI_A2B_FRAMEWORK_H_
#define _ADI_A2B_FRAMEWORK_H_

/*============= I N C L U D E S =============*/
#include "a2b/ctypes.h"
#include "a2b/i2c.h"
#include "a2bapp_defs.h"
/*============= D E F I N E S =============*/
/*
** Constants: First set of constants for module Y
** MYDEFINE - definitions My define
*/
/*! Core clock in MHz  */
#define A2B_CORE_CLOCK				(588000000u)

/*! SDP-b board crystal clock in MHz */
#define A2B_CRYSTAL_CLOCK			(24000000u)

/*! BF527 Max core clock in MHz */
#define A2B_MAX_CORECLOCK			(600000000u)

/*! BF527 max system clock in MHz */
#define A2B_MAX_SYSCLOCK			(125000000u)


/* Memory sections for App and PAL */
#define A2B_APP_L1_CODE   __attribute__ ((section ("L1_code")))
#define A2B_APP_L3_CODE   __attribute__ ((section ("L3_code")))
/*! External memory code sections */
#define A2B_PAL_L1_CODE   __attribute__ ((section ("L1_code")))
#define A2B_PAL_L3_CODE   __attribute__ ((section ("L3_code")))
/*! Internal memory data sections */
#define A2B_APP_L1_DATA   __attribute__ ((section ("L1_data")))
#define A2B_APP_L3_DATA   __attribute__ ((section ("L3_data")))
/*! External memory data sections */
#define A2B_PAL_L1_DATA   __attribute__ ((section ("L1_data")))
#define A2B_PAL_L3_DATA   __attribute__ ((section ("L3_data")))

/*! Total number of Log Channels */
#define A2B_TOTAL_LOG_CH                   (A2B_CONF_MAX_NUM_MASTER_NODES * A2B_CONF_LOG_NUM_CHANNELS)

/*! Abstracted timer identification number used for A2B transceiver related events (FIXED to 1) */
#define A2B_TIMER_NO                                        (1u) 
 
/*! Node discovery time-out period in micro-seconds */
#define A2B_NODE_DISCOVERY_TIMEOUT                          (50000u)

/*! System clock */
#define A2B_SCLK                                            (49152000u*2u)

/*! Abstracted TWI peripheral identification number used for A2B connection */
#define A2B_TWI_NO                                          (0u)


/*********************************************************************
 *
 * PLATFORM SPECIFIC MACROS - CONF
 *********************************************************************/
/** The length of the buffer to hold the path to the I2C device. This
 * is typically something like /dev/i2c-N where 'N' is the device number.
 */
#define A2B_PAL_I2C_DEVICE_PATH_LEN (32)

/** Define the default (7-bit) A2B master node I2C address */
#define A2B_CONF_DEFAULT_MASTER_NODE_I2C_ADDR   (0x68)

/** Define the name of the TRACE log channel in terms of a URL.
 * The URL is encoded as follows:
 *      \verbatim      <protocol>//:<resource>:<port>      \endverbatim
 *
 * With the following supported protocol options:
 *      \verbatim
        file://<filename>
        stdio://stderr
        stdio://stdout
        tcp://<host>:<port>
        udp://<host>:<port>
        syslog://<level>
            where level = emerg, alert, crit, err, warning, notice, info, debug
        \endverbatim
 *
 * Example:
 *      \verbatim       udp://localhost:18001       \endverbatim
 */

#define A2B_INVALID_FD          							(-1)
/*============================================== Configurable MACROs ===============================================*/

#ifdef A2B_PRINT_CONSOLE
/*! Debug macro - used only for printing PAL configuration status */
#define A2B_PRINT_FOR_DEBUG                                 (1u)

#else
/*! Debug macro - used only for printing PAL configuration status */
#define A2B_PRINT_FOR_DEBUG                                 (0u)
#endif

           
/*============================================== Debug MACROs ===============================================*/

/*! Enable I2C word store  */
#define A2B_DEBUG_STORE_CONFIG                               (0u)

/*! Enable cycle measurement */
#define DO_CYCLE_COUNTS

/*! Number of TWI words, increase according to network size */
#define A2B_TWI_DEBUG_NUM                                    (1000u)
/*============= D A T A T Y P E S=============*/

/*! \enum ADI_A2B_TIMER_CONTEXT
    Context of timer run 
 */
typedef enum
{
 /*!  During discovery   */
  ADI_A2B_SYS_CLK,
 /*!  For I2C event  */  
  ADI_A2B_I2C,

}ADI_A2B_TIMER_CONTEXT;


/*! Timer callback */
typedef void (*TIMER_CALL_BACK)(void* pHandle);


/*! \struct ADI_A2B_TIMER_HANDLER
    Configuration structure for GPT
*/
typedef struct ADI_A2B_TIMER_HANDLER
{
  /*! Flag to notify Timer expiry*/  
  volatile uint8_t bTimeout;
  
  /*! Application call back function pointer*/
  TIMER_CALL_BACK pCallbackhandle;
  
  /*! Timer Number */
  uint32_t nTimerNo;
  
  /*! Timer Expire value in milliSecond */
  uint32_t nTimerExpireVal;
  
  /*! A2B timer context */
  ADI_A2B_TIMER_CONTEXT eContext; 

}ADI_A2B_TIMER_HANDLER,*ADI_A2B_TIMER_HANDLER_PTR;


typedef void (*GPIO_CALL_BACK)(uint8_t nGPIONum, void* pHandle);

/*! \struct ADI_A2B_CONFIG_TABLE
    Structure to store config words
*/

typedef struct 
{
    /*! Register address */
    uint8_t nAddress;
    /*! Value  */
    uint8_t nValue;

}ADI_A2B_CONFIG_TABLE;

/*! \struct ADI_A2B_TWI_CONFIG
    TWI data configuration structure
*/
typedef struct ADI_A2B_TWI_CONFIG
{
    /*! Device Num  */
    uint32_t nTWIDeviceNo;

    /*! address Len */
    a2b_I2cAddrFmt a2b_fmt;

    /*! I2C speed  */
    a2b_I2cBusSpeed i2c_speed;

}ADI_A2B_TWI_CONFIG;

/*! \enum A2B_LOGGING_MODE
    Modes of Logging
*/
typedef enum
{
    PAL_LOG_UNKNOWN = 0,
    PAL_LOG_FILE
} A2B_LOGGING_MODE;

/*! \struct A2B_LOG_INFO
    Logging Information Structure
*/
typedef struct pal_LogConfig
{
    a2b_Bool inUse;
    a2b_Int32 fd;
    A2B_LOGGING_MODE elogmode;
} A2B_LOG_INFO;

/*! \struct ADI_A2B_TWI_DATA_DEBUG
    TWI data configuration structure
*/
typedef struct
{
    /*! Operation  */
    uint8_t eOp;

    /*! TWI address  */
    uint8_t nTwiAddr;

    /*! Register address  */
    uint8_t nRegAddr;

    /*! nValue  */
    uint8_t nDatVal;

}ADI_A2B_TWI_DATA_DEBUG;


/*! \struct ADI_A2B_TWI_DATA_DEBUG_CONFIG
    TWI data configuration structure
*/
typedef struct
{
    /*! Number of values  */
   ADI_A2B_TWI_DATA_DEBUG aStoreDataVal[A2B_TWI_DEBUG_NUM];

    /*! Total  */
    uint32_t nNum;

}ADI_A2B_TWI_DATA_DEBUG_CONFIG;


/** 
 @}
*/


/** 
 @}
*/

/*======= P U B L I C P R O T O T Y P E S ========*/

#endif /* _ADI_A2B_FRAMEWORK_H_ */

/** 
 @}
*/


/** 
 @}
*/

/*
**
** EOF: $URL$
**
*/

