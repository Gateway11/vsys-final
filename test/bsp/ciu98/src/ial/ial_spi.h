/**@file  hal_spi.h
* @brief  port hal spi interface declearation    
* @author  zhengwd
* @date  2021-04-24
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/

#ifndef _HAL_SPI_H_
#define _HAL_SPI_H_

#include "types.h"
#include "peripheral.h"
#include "util.h"

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


/** @addtogroup HAL_SPI 
  * @{
  */



/* peripheral define --------------------------------------------------------*/
/** @defgroup peripheral_spi_comm_declearation peripheral spi communication declearation
  * @{
  */


#define HAL_SPI_PERIPHERAL_STRUCT_POINTER PERIPHERAL_STRUCT_DEFINE(PERIPHERAL_SPI)*


//brief ����ͨ����������ӿ�

//struct peripheral_PERIPHERAL_SPI
PERIPHERAL_STRUCT_DEFINE(PERIPHERAL_SPI) {
    peripheral periph;  //Peripheral interface
    int32_t (*init)       (HAL_SPI_PERIPHERAL_STRUCT_POINTER periph);  //  Initialization
    int32_t (*deinit)	  (HAL_SPI_PERIPHERAL_STRUCT_POINTER periph);  //  Termination
    int32_t (*delay)(uint32_t us);  //microsecond delay
    int32_t (*timer_start)(util_timer_t *timerval);  //start timing
    int32_t (*timer_differ)(util_timer_t *timerval);  //check timeout
    int32_t (*chip_select)(HAL_SPI_PERIPHERAL_STRUCT_POINTER periph, bool enable);  //Chip Select
    int32_t (*transmit)   (HAL_SPI_PERIPHERAL_STRUCT_POINTER periph, uint8_t *data, uint32_t  data_len);   //Send data 
    int32_t (*receive)    (HAL_SPI_PERIPHERAL_STRUCT_POINTER periph, uint8_t *data, uint32_t *data_len);   // Receive data 
    int32_t (*control)    (HAL_SPI_PERIPHERAL_STRUCT_POINTER periph, uint32_t ctrlcode, uint8_t *data, uint32_t  *data_len);  //Send and receive control commands
#ifdef SPI_SUPPORT_GPIO_IRQ_RECEIVE
    int32_t (*gpio_irqwait_edge)(uint32_t wait_time);  //wait for gpio 
#endif
//spi_comm_param_pointer extra;
        void *extra;       //RFU 
};

/** Define SPI peripheral instance variable name */

#define SPI_PERIPHERAL_NAME(id) PERIPHERAL_NAME(PERIPHERAL_SPI, id)

/** Define SPI peripheral start */

#define SPI_PERIPHERAL_DEFINE_BEGIN(id) PERIPHERAL_DEFINE_BEGIN(PERIPHERAL_SPI, id)

/** Define SPI peripheral end */
#define SPI_PERIPHERAL_DEFINE_END() PERIPHERAL_DEFINE_END()

/** Register SPI peripheral*/
#define SPI_PERIPHERAL_REGISTER(id) PERIPHERAL_REGISTER(PERIPHERAL_SPI, id, PERIPHERAL_NAME(PERIPHERAL_SPI, id))

/** Declare SPI peripheral */
#define SPI_PERIPHERAL_DECLARE(id) PERIPHERAL_DECLARE(PERIPHERAL_SPI, id)  

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
