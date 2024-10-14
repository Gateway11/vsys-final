/**@file  proto_spi.h
* @brief  proto_spi interface declearation 
* @author  liangww
* @date  2021-04-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/
#ifndef _PROTO_SPI_H_
#define _PROTO_SPI_H_

/***************************************************************************
* Include Header Files
***************************************************************************/
#include <stdint.h>
#include "types.h"
#include "config.h"
#include "peripheral.h"
#include "ial_spi.h"
#include "util.h"
	


/**************************************************************************
* Global Variable Declaration
***************************************************************************/
//extern peripheral_bus_driver g_proto_spi;
PERIPHERAL_BUS_DRIVER_DECLARE(PERIPHERAL_SPI); 



/** @addtogroup SE_Driver
  * @{
  */

/** @addtogroup PROTO 
  * @brief link protocol layer.
  * @{
  */


/** @addtogroup PROTO_SPI 
  * @{
  */


/* Private constants --------------------------------------------------------*/
/** @defgroup Proto_Spi_Impl_Private_Constants Proto_Spi_Impl Private Constants
  * @{
  */

/** @defgroup HED_SPI_TIME_PARAMS    HED SPI Communication Protocol Time Params
  * @{
  */
#define SPI_SEND_CS_WAKEUP_TIME                  210   /*!< Wake-up time:210us */  
//#define SPI_SEND_DATA_OVER_WAIT_TIME              200 /*!< Protocol parameter T4: 200us */ 
#define SPI_SEND_DATA_OVER_WAIT_TIME              200 /*!< Protocol parameter T3: 200us */ 
#define SPI_SEND_DATA_OVER_WAIT_TIME_FOR_SM2_SIGN 6500 /*!< Protocol parameter T3 for SM2 sign: 6.5ms */ 
#define SPI_RESET_POLL_SLAVE_INTERVAL_TIME        20  /*!< Protocol parameter T4:20us */ 
#define SPI_BEFORE_RECEIVE_DATA_WAIT_TIME         20/*!< Protocol parameter T5: 20us */ 
#define SPI_SEND_BGT_TIME                         200  /*!< Frame protection time:200us */ 
	
#define SPI_RECEVIE_FRAME_WAIT_TIME               700   /*!< Frame waiting time BWT:700ms */ 
#define SPI_COMM_MUTEX_WAIT_TIME                  200   /*!< Communication lock identification waiting timeout time:200ms */ 
#define SPI_PROTO_SE_RST_DELAY                 350000   /*!<Delay 350 ms to ensure the SE woke up */  
#ifdef SPI_SUPPORT_GPIO_IRQ_RECEIVE 
#define SPI_RECEVIE_FRAME_WAIT_GPIO_RISING_IRQ_TIME           700/*!< time limit of waiting for the IRQ rising signel 700ms */ 
#define SPI_RECEVIE_FRAME_WAIT_GPIO_FALLING_IRQ_TIME           700/*!< time limit of waiting for the IRQ falling signel 700ms */ 
#endif
/**
  * @}
  */

/** @defgroup HED_SPI_FRAME_DEFINE    HED SPI  Protocol Frame Define
  * @{
  */

//Frame type definition
#define PIB_ACTIVE_FRAME                0x03   /*!< Active frame*/        
#define PIB_INFORMATION_FRAME    0x0E  /*!< Information frame*/      
#define PIB_PROCESS_FRAME             0x09  /*!< Process frame*/

//Active frame type definition
#define PIB_ACTIVE_FRAME_RESET          0xD3  /*!<RESET Active frame*/        
#define PIB_ACTIVE_FRAME_RATR           0xE2  /*!<RATR Active frame*/   
#define PIB_ACTIVE_FRAME_RATR_RESPONSE  0x3B 

//Process frame type definition
#define PIB_PROCESS_FRAME_NAK_CRC_INFO      0x3C   /*!<NAK Process frame for crc error*/
#define PIB_PROCESS_FRAME_NAK_OTHER_INFO      0x3D   /*!<NAK Process frame for other error*/      

#define PIB_PROCESS_FRAME_WTX_INFO      0x60  /*!<WTX Process frame*/     


//The offset 
#define PIB_OFFSET        0
#define LEN_OFFSET        1
#define DATA_OFFSET      3
#define APDU_CLA_OFFSET 3
#define APDU_INS_OFFSET 4
#define APDU_P1_OFFSET  5

//Protocol field length definition
#define  FRAME_HEAD_LEN                3   /*!<frame Header length*/    
#define  ACTIVE_FRAME_DATA_EDC_LEN     4  /*!<total length of active frame data and EDC*/  
#define  ACTIVE_REQ_FRAME_LEN          7  /*!<total length of active frame*/  
#define  PROCESS_FRAME_LEN             6  /*!<total length of process frame*/  
#define  EDC_LEN                       2  /*!< EDC length*/  
#define  WAKEUP_DATA_LEN               3 /*!<Wake-up character length*/ 

#define FRAME_LEN_MAX                 4101   /*!<Maximum length of the frame:3+4096+2 */ 
//#define FRAME_LEN_MAX                 265   /*!< Maximum length of the frame:3+5+255+1+1 */ 
#define FRAME_DATA_LEN_MAX  (FRAME_LEN_MAX - EDC_LEN-FRAME_HEAD_LEN)


#define PROTO_SPI_PFSM_DEFAULT          0   /*!<PFSM Defaults*/ 
#define PROTO_SPI_PFSS_DEFAULT          0     /*!<PFSS Defaults*/ 
#define PROTO_SPI_HBSM_DEFAULT          0   /*!<HBSM Defaults*/ 
#define PROTO_SPI_HBSS_DEFAULT          0     /*!<HBSS Defaults*/ 


#define PROTO_SPI_ATR_MAX_LEN         32   /*!<ATR maximum length value*/ 
#define PROTO_SPI_RERESET_MAX_LEN      7  /*!<maximum length value of response to reset frame*/ 
#define PROTO_SPI_RETRY_NUM           3    /*!<maximum number of EDC check error of retransmissions*/ 
#define PROTO_SPI_WTX_NUM               20   /*!<maximum number of WTX requests*/

/********************calculation type of CRC*******************/
#define	CRC_A 0
#define	CRC_B 1	

  
/**
  * @}
  */




/**
  * @}
  */



/* Exported types ------------------------------------------------------------*/
/** @defgroup Proto_Spi_Exported_Types Proto_Spi Exported Types
  * @{
  */

typedef struct
{
    uint32_t  start;	
    uint32_t  interval;		
}spi_timer_t;

/**
  * @brief  SPI Frame Structure definition
  */
typedef enum _SPI_FRMAE_TYPE
{
        SPI_WTX,              ///< WTX request frame
        SPI_NAK,              ///< NAK request frame
        SPI_INFO,            ///< information frame
        SPI_DEFAULT,     ///< RTU
        SPI_ACTIVE,           ///< active frame
}SPI_FRMAE_TYPE;

/**
  * @brief  SPI Param Structure definition
  */
typedef struct  {
	uint16_t pfsm;     ///< Max Protocol Frame Size For Master
	uint16_t pfss;      ///< Max Protocol Frame Size For Master
	uint16_t hbsm;     ///< Max Hardware Block Size For Master
	uint16_t hbss;      ///<Max Hardware Block Size For Slave
	SPI_FRMAE_TYPE type;  ///< frame type
} spi_param_t;

/**
  * @brief  Proto SPI  Control Structure definition
  */
enum  PROTO_SPI_CTRL
{
	PROTO_SPI_CTRL_RST =		0x00000010,    ///< RST reset control
	PROTO_SPI_CTRL_OTHER =		0x00000011    ///< other controls
} ;

enum HED20_SPI_RESET_TYPE
{
	 HED20_SPI_RESET_SE_RESET_NONE = 0x00,	 ///< no RESET request  while reset se
	 HED20_SPI_RESET_SE_RESET_REQ = 0x01,	///< RESET request while reset se
};

enum HED20_SPI_OPEN_TYPE
{
	 HED20_SPI_OPEN_SE_RESET_NONE	 = 0x00,	 ///< no RESET request  while open se
	 HED20_SPI_OPEN_SE_RESET_REQ = 0x01,	///< RESET request while open se
};

#define spi_time_get_diff(a, b)\
                ((a >= b) ? (a - b) : (0xffffffff -(b - a)))

/**
  * @}
  */

 #ifdef SPI_SUPPORT_GPIO_IRQ_RECEIVE
 /**
  * @brief  Port SPI  IRQ Edge Mode
  */
enum SPI_IRQ_EDGE_MODE
{
	SPI_IRQ_RISING    = 0x00,		
	SPI_IRQ_FALLING	= 0x01,
	SPI_IRQ_BOTH = 0x02,
	SPI_IRQ_NONE = 0x03,
};
/**
  * @}
  */
#endif

/* Exported functions --------------------------------------------------------*/
/** @defgroup Proto_Spi_Exported_Functions Proto_Spi Exported Functions
  * @{
  */

extern se_error_t proto_spi_init(peripheral *periph);
extern se_error_t proto_spi_deinit(peripheral *periph) ;
extern se_error_t proto_spi_open(peripheral *periph , uint8_t *rbuf, uint32_t *rlen) ;
extern se_error_t proto_spi_close(peripheral *periph) ;
//extern se_error_t proto_spi_power_on(peripheral *periph , uint8_t *rbuf, uint32_t *rlen) ;
extern se_error_t proto_spi_transceive(peripheral *periph, uint8_t *sbuf, uint32_t	slen, uint8_t *rbuf, uint32_t *rlen);
extern se_error_t proto_spi_reset(peripheral *periph , uint8_t *rbuf, uint32_t *rlen);
extern se_error_t proto_spi_control(peripheral *periph , uint32_t ctrlcode, uint8_t *sbuf, uint32_t slen, uint8_t  *rbuf, uint32_t *rlen);
extern se_error_t proto_spi_delay(peripheral *periph , uint32_t us) ;


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



#endif //_PROTO_SPI_H_

