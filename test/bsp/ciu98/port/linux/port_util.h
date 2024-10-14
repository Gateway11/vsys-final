/**@file  port_util.h
* @brief  port_util interface declearation 
* @author  zhengwd
* @date  2021-04-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/


#ifndef _PORT_UTIL_H_
#define _PORT_UTIL_H_
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include <fcntl.h> 
#include <stdio.h> 
#include <string.h>
#include <unistd.h>


#include "config.h"



/**************************************************************************
* Global Type Definition
***************************************************************************/

#define LOW  0
#define HIGH 1



#define LOCK_BUSY    0
#define LOCK_UNBUSY  1
 
/** @addtogroup SE_Driver
  * @{
  */

/** @addtogroup PORT 
  * @brief hardware  portable layer .
  * @{
  */


/** @addtogroup PORT_UTIL 
  * @{
  */

/* Exported functions --------------------------------------------------------*/
/** @defgroup Port_Uti_Exported_Functions Port_Util Exported Functions
  * @{
  */
  
extern int port_gpio_export(int pin);
extern int port_gpio_unexport(int pin);
extern int port_gpio_direction(int pin, int dir);
extern int port_gpio_write(int pin, int value);
extern int port_gpio_read(int pin);


extern void port_gpio_init(void);
extern void port_mcu_init(void);
extern void port_printf(const char *format, ...);
extern void port_power_on(void);
extern void port_power_off(void);
extern int  port_lock(volatile int *lock);
extern int  port_unlock(volatile int *lock);

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


#endif/*_PORT_UTIL_H_*/
