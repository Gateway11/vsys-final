/**@file  port_i2c.h
* @brief  port_i2c interface declearation 
* @author  zhengwd
* @date  2021-04-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/

#ifndef _PORT_I2C_H_
#define _PORT_I2C_H_

/***************************************************************************
* Include Header Files
***************************************************************************/
#include <stdint.h>
#include "types.h"
#include "config.h"
#include "peripheral.h"
#include "ial_i2c.h"



/**************************************************************************
* Global Macro Definition
***************************************************************************/

/** @addtogroup SE_Driver
  * @{
  */

/** @addtogroup PORT 
  * @brief hardware  portable layer .
  * @{
  */


/** @addtogroup PORT_I2C 
  * @{
  */


/* Private constants --------------------------------------------------------*/
/** @defgroup Port_I2c_Private_Constants Port_I2c Private Constants
  * @{
  */

/** @defgroup PORT_I2C_TIME_PARAMS    I2C Communication Time Params
  * @{
  */



#define             PORT_I2C_SE_RST_LOW_DELAY        1000  /*!< RST low level duration at reset T7:  1000us */  
#define		PORT_I2C_SE_RST_HIGH_DELAY       10000  /*!< RST high level duration after reset T6:  10000us */  

//#define		PORT_I2C_SE_PWR_OFF_DEALY       5000      //us	 5ms
//#define		PORT_I2C_SE_PWR_ON_DEALY        5000      //us	 5ms

/**
  * @}
  */






/******************** control gpio define *******************/
//SE0 RST control IO
//#define PORT_I2C_SE0_RST_IO      1       //nano PI
#define PORT_I2C_SE0_RST_IO     18      //raspberry PI

#ifdef I2C_SUPPORT_GPIO_IRQ_RECEIVE
#define PORT_I2C_SE0_GPIO_IRQ     17      //raspberry PI
#endif

#define PORT_I2C_SE0_RST_LOW()       port_gpio_write(PORT_I2C_SE0_RST_IO, LOW) 
#define PORT_I2C_SE0_RST_HIGH()      port_gpio_write(PORT_I2C_SE0_RST_IO, HIGH)



/**
  * @}
  */


/* Exported types ------------------------------------------------------------*/
/** @defgroup Port_I2c_Exported_Types Port_I2c Exported Types
  * @{
  */

/**
  * @brief  Port I2C  Control Structure definition
  */
enum  PORT_I2C_CTRL
{
	PORT_I2C_CTRL_RST =   0x00000010,	/*!< RST reset control*/  
	PORT_I2C_CTRL_OTHER = 0x00000011       /*!< other controls*/  
} ;

/**
  * @brief  I2C Set Structure definition
  */
typedef struct __I2C_SetDef
{ 
	int          aiI2cFds[2];  
	int          aiI2cSlave[2];  
} I2C_SetDef;


/**
  * @brief  Port I2C  Communication Params Structure definition
  */
typedef struct   _i2c_comm_param_t{
	I2C_SetDef *i2c_handle;  /*!< I2C communication handle */  
	int channel;/*!< I2C channel*/    
	int slave; /*!< I2C Slave device ID address*/    
	int speed; /*!< I2C communication speed*/    
	int timeout;  /*!< I2C timeout*/  
	int retry;  /*!< I2C retry times*/   
	uint8_t slave_id;  /*!< Slave device ID */  
} i2c_comm_param_t, *i2c_comm_param_pointer;

/**
  * @}
  */


/**************************************************************************
* Global Variable Declaration
***************************************************************************/
I2C_PERIPHERAL_DECLARE(I2C_PERIPHERAL_SE0);


/* Exported functions --------------------------------------------------------*/
/** @defgroup Port_I2c_Exported_Functions Port_I2c Exported Functions
  * @{
  */
extern se_error_t port_i2c_periph_delay(uint32_t us);
extern se_error_t port_i2c_periph_timer_start(util_timer_t *timer_start);
extern se_error_t port_i2c_periph_timer_differ(util_timer_t *timer_differ);
extern se_error_t port_i2c_periph_init (HAL_I2C_PERIPHERAL_STRUCT_POINTER periph) ;
extern se_error_t port_i2c_periph_deinit (HAL_I2C_PERIPHERAL_STRUCT_POINTER periph) ;
extern se_error_t port_i2c_periph_transmit(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph, uint8_t *inbuf, uint32_t  inbuf_len) ;
extern se_error_t port_i2c_periph_receive(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph, uint8_t *outbuf, uint32_t *outbuf_len) ;
extern se_error_t port_i2c_periph_control(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph, uint32_t ctrlcode, uint8_t *inbuf, uint32_t  inbuf_len) ;
#ifdef I2C_SUPPORT_GPIO_IRQ_RECEIVE
extern se_error_t port_i2c_periph_gpio_irqwait_edge(uint32_t wait_time);
// extern se_error_t port_i2c_periph_gpio_irq_edge_set(uint8_t edge_mode); 
//extern se_error_t port_i2c_periph_gpio_irqwait_high(uint32_t wait_time);
#endif
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


#endif /*_PORT_I2C_H_*/

