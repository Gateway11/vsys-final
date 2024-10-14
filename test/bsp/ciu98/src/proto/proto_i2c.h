/******************************************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd.
 File name: 		proto_i2c.h
 Author:			zhengwd 
 Version:			V1.0	
 Date:			2021-04-26	
 Description:	        
 History:		

******************************************************************************/
#ifndef _PROTO_I2C_H_
#define _PROTO_I2C_H_ 

/***************************************************************************
* Include Header Files
***************************************************************************/
#include <stdint.h>
#include "types.h"
#include "config.h"
#include "peripheral.h"
#include "ial_i2c.h"
#include "util.h"


/**************************************************************************
* Global Variable Declaration
***************************************************************************/
//extern peripheral_bus_driver g_proto_i2c;
PERIPHERAL_BUS_DRIVER_DECLARE(PERIPHERAL_I2C); 

/** @addtogroup SE_Driver
  * @{
  */

/** @addtogroup PROTO 
  * @brief link protocol layer.
  * @{
  */


/** @addtogroup PROTO_I2C 
  * @{
  */


/* Private constants --------------------------------------------------------*/
/** @defgroup Proto_I2c_Private_Constants Proto_I2c Private Constants
  * @{
  */

/** @defgroup HED_I2C_TIME_PARAMS	 HED I2C Communication Protocol Time Params
  * @{
  */
#define PROTO_I2C_RECEVIE_FRAME_WAIT_TIME     700    /*!< Frame waiting time FWT:700ms*/ 
#define PROTO_I2C_COMM_MUTEX_WAIT_TIME         200   /*!< Communication lock identification waiting timeout time:200ms */ 
	
#define PROTO_I2C_RECEVIE_POLL_TIME                          1000 /*!< Frame polling time:1000us */ 
#define PROTO_I2C_RECEIVE_TO_SEND_BGT                     200  /*!<  Frame protection time:200us */ 
#define PROTO_I2C_SE_RST_DELAY                  350000   /*!< Delay 350 ms to ensure the SE woke up*/   

#ifdef I2C_SUPPORT_GPIO_IRQ_RECEIVE 
#define PROTO_I2C_IRQ_FIRST_RECEVIE_WAIT_TIME                          20 /*!< between find the falling and first read data interval time 20us */ 
#define I2C_RECEVIE_FRAME_WAIT_GPIO_RISING_IRQ_TIME           700  /*!<limit time for waiting IRQ rising signel 700ms */ 
#define I2C_RECEVIE_FRAME_WAIT_GPIO_FALLING_IRQ_TIME           700  /*!< limit time for waiting IRQ falling signel 700ms */ 
#endif
/**
  * @}
  */

/** @defgroup HED_I2C_FRAME_DEFINE    HED I2C Protocol Frame Define
  * @{
  */
  

//Protocol field length definition
#define PROTO_I2C_PIB_SIZE                       1   /*!< PIB length of the frame*/    
#define PROTO_I2C_LEN_SIZE                       2  /*!< LEN length of the frame*/       
#define PROTO_I2C_EDC_SIZE                       2 /*!< EDC length of the frame*/   
#define PROTO_I2C_PIB_LEN_SIZE               3 /*!< total length of PIB, LEN of the frame*/  
#define PROTO_I2C_PIB_LEN_EDC_SIZE      5 /*!< total length of PIB, LEN and EDC of the frame*/  
#define PROTO_I2C_RS_FRAME_SIZE           5 /*!< length of R or S frame*/             
#define PROTO_I2C_RESET_RSP_SIZE          5 /*!< length of response to reset request frame*/  
#define PROTO_I2C_ATR_RSP_SIZE             32 /*!< length of response to atr request frame*/  
#define PROTO_I2C_FRONT_FRAME_SIZE     7  /*!<length of frame header buffer*/            
#define PROTO_I2C_FRAME_MAX_SIZE        0x4000 /*!< Maximum length of the frame*/  
	
#define PROTO_I2C_TIMEOUT_RETRY_MAX_NUM               1    /*!< If timeout , Maximum of retransmissions */ 
#define PROTO_I2C_CRC_ERROR_RETRY_MAX_NUM          3   /*!<If EDC error , Maximum of retransmissions */ 
	

//Protocol field offset definition
#define PROTO_I2C_PIB_OFFSET                  (0)   /*!< PIB offset of the frame*/    
#define PROTO_I2C_LEN_OFFSET                  (1)   /*!< LEN offset of the frame*/    
#define PROTO_I2C_DATA_OFFSET               (3)   /*!< DATA offset of the frame*/    
		
//calculation type of CRC
#define	CRC_A 0  
#define	CRC_B 1	


/**
  * @}
  */

/**
  * @}
  */


/* Exported types ------------------------------------------------------------*/
/** @defgroup Proto_I2c_Exported_Types Proto_I2c Exported Types
  * @{
  */

/**
  * @brief  I2C Frame Structure definition
  */
enum PROTO_I2C_FRAME_TYPE
{
	PROTO_I2C_S_FRAME_WTX  = 0xC0,   ///< S frame (wtx request frame)
	PROTO_I2C_S_FRAME_RESET  = 0xE0,   ///< S frame (reset request frame)
	PROTO_I2C_I_FRAME_NO_LINK = 0x20,   ///< I frame (non chained frame)
	PROTO_I2C_I_FRAME_LINK = 0x00,   ///< I frame (chain frame)
	PROTO_I2C_I_FRAME_ATR	= 0x30,   ///< I frame (atr request frame)
	PROTO_I2C_R_FRAME_ACK = 0x80,   ///< R frame (ACK frame)
	PROTO_I2C_R_FRAME_NAK = 0x81,   ///< R frame (NAK frame)
};

/**
  * @brief  I2C Open SE Structure definition
  */
enum HED20_I2C_OPEN_TYPE
{
	HED20_I2C_OPEN_SE_RESET_NONE	= 0x00, 	 ///< no RESET request  while open se
	HED20_I2C_OPEN_SE_RESET_REQ = 0x01,    ///< RESET request while open se
};

/**
  * @brief  I2C Reset SE Structure definition
  */
enum HED20_I2C_RESET_TYPE
{
	HED20_I2C_RESET_SE_RESET_NONE = 0x00,	///< no RESET request  while reset se
	HED20_I2C_RESET_SE_RESET_REQ = 0x01,   ///< RESET request while reset se
};

/**
  * @brief  I2C Control Mode Structure definition
  */
enum  PROTO_I2C_CTRL
{
	PROTO_I2C_CTRL_RST =		0x00000010,     ///< RST reset control
	PROTO_I2C_CTRL_OTHER =		0x00000011	///< other controls
} ;


/**
  * @}
  */

#ifdef I2C_SUPPORT_GPIO_IRQ_RECEIVE
 /**
  * @brief  Port I2C  IRQ Edge Mode
  */
enum I2C_IRQ_EDGE_MODE
{
	I2C_IRQ_RISING    = 0x00,		
	I2C_IRQ_FALLING	= 0x01,
	I2C_IRQ_BOTH = 0x02,
	I2C_IRQ_NONE = 0x03,
};
/**
  * @}
  */
#endif

/* Exported functions --------------------------------------------------------*/
/** @defgroup Proto_I2c_Exported_Functions Proto_I2c Exported Functions
  * @{
  */
extern se_error_t proto_i2c_init(peripheral *periph);
extern se_error_t proto_i2c_deinit(peripheral *periph);
extern se_error_t proto_i2c_open(peripheral *periph, uint8_t *rbuf, uint32_t *rlen);
extern se_error_t proto_i2c_close(peripheral *periph) ;
extern se_error_t proto_i2c_transceive(peripheral *periph, uint8_t *sbuf,    uint32_t  slen, uint8_t *rbuf, uint32_t *rlen);
extern se_error_t proto_i2c_reset(peripheral *periph, uint8_t *rbuf, uint32_t *rlen);
extern se_error_t proto_i2c_control(peripheral *periph , uint32_t ctrlcode, uint8_t *sbuf, uint32_t slen, uint8_t  *rbuf, uint32_t *rlen);
extern se_error_t proto_i2c_delay(peripheral *periph , uint32_t us); 

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */


#endif

