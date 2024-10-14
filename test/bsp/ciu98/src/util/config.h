/**@file  config.h
* @brief  Header file of config
* @author  liangww
* @date  2021-5-12
* @version  V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/



#ifndef _CONFIG_H_
#define _CONFIG_H_

//#include "main.h"

/**************************************************************************
* Global Macro Definition
***************************************************************************/



/** @addtogroup SE_Driver
  * @{
  */

/** @addtogroup UTIL
  * @{
  */


  
/* exported constants --------------------------------------------------------*/
/** @defgroup UTIL_Config_Exported_Constants UTIL_Config Exported Constants
  * @{
  */

#define  SPI_PERIPHERAL_SE0               0	/*!< Peripheral SE ID value*/ 
#define  I2C_PERIPHERAL_SE0               0	/*!< Peripheral SE ID value*/  

 
#define  _DEBUG                                            
#define  PORT_UART_PRINTF_ENABLE   0     /*!< When using the serial port to print out log, you need to set the macro to 1 */ 
//#define CONNECT_NEED_AUTH 			/*!<whether support the auth connect*/
/**
  * @}
  */

/**
  * @}
  */


/**
  * @}
  */


#endif  //_CONFIG_H_

