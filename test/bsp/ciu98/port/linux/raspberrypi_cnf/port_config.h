/**@file  port_config.h
* @brief  port_config interface declearation 
* @author  zhengwd
* @date 2021-04-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/


#ifndef _PORT_CONFIG_H_
#define _PORT_CONFIG_H_
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include <fcntl.h> 
#include <stdio.h> 
#include <string.h>
#include <unistd.h>


#include "config.h"


#define IN  0
#define OUT 1

#define LOW  0
#define HIGH 1

#define RISING 0
#define FALLING 1
#define BOTH 2
#define NONE 3

#define POUT            26  
#define BUFFER_MAX      5
#define DIRECTION_MAX   48

 
#define HED_SPI_SE0_ENABLE 1    //SPI ENABLE

#define HED_I2C_SE0_ENABLE 1    //I2C ENABLE



/**************************************************************************
* Global Type Definition
***************************************************************************/
/*
typedef struct
{
	struct timeval	start;	
	uint32_t  interval; 	
}util_timer_t;
 */


//#define PORT_POWER_CTRL_IO      201    //nano PI
#define PORT_POWER_CTRL_IO      20     //raspberry PI


#define PORT_POWER_CTRL_ON()       port_gpio_write(PORT_POWER_CTRL_IO, LOW) 
#define PORT_POWER_CTRL_OFF()      port_gpio_write(PORT_POWER_CTRL_IO, HIGH)


/** @addtogroup SE_Driver
  * @{
  */

/** @addtogroup PORT 
  * @brief hardware  portable layer .
  * @{
  */


/** @addtogroup PORT_CONFIG 
  * @{
  */

/** @defgroup PORT_Config_Exported_Constants PORT_Config Exported Constants
  * @{
  */

#if HED_SPI_SE0_ENABLE
#define  SPI_CHANNEL_SPI0                   0	/*!< SPI0 channel*/  
#define  SPI_CHANNEL_SPI1                   1	/*!< SPI1 channel*/  
#define  SPI_SPEED                   		   15000000 	/*!< communication speed: 15MHz*/  
#define  SPI_BPW                   		   8 	                /*!< communication bits: 8bits*/  
#endif

#if HED_I2C_SE0_ENABLE
#define  I2C_CHANNEL_I2C0                   0	/*!< I2C0 channel*/  
#define  I2C_CHANNEL_I2C1                   1	/*!< I2C1 channel*/  
#define  I2C_SPEED_DEFAULT                  100000 	        /*!< communication speed: 100Kbps*/  
#define  I2C_SLAVE_ADRR                       0x2A 	                /*!< slave address: 2A*/  
#define  I2C_TIMEOUT_DEFAULT             700 	        /*!< timeout: 700ms*/  
#define  I2C_RETRY_DEFAULT                 3 	                /*!< retry times: 3 */ 
#endif 


/**
  * @}
  */


/* Exported functions --------------------------------------------------------*/
/** @defgroup Port_Config_Functions Port_Config Exported Functions
  * @{
  */
  
extern int port_gpio_export(int pin);
extern int port_gpio_unexport(int pin);
extern int port_gpio_direction(int pin, int dir);
extern int port_gpio_write(int pin, int value);
extern int port_gpio_read(int pin);
extern int port_gpio_edge_set(int pin, int edge_mode);
extern int port_gpio_edge_init(int pin, int edge_mode);
extern void port_gpio_init(void);
extern void port_mcu_init(void);

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


#endif/*_PORT_CONFIG_H_*/
