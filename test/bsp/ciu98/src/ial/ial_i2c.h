
#ifndef _HAL_I2C_H_
#define _HAL_I2C_H_

#include "types.h"
#include "peripheral.h"
#include "util.h"



#define HAL_I2C_PERIPHERAL_STRUCT_POINTER PERIPHERAL_STRUCT_DEFINE(PERIPHERAL_I2C)*

PERIPHERAL_STRUCT_DEFINE(PERIPHERAL_I2C) {
    peripheral periph;  // Peripheral interface
    int32_t (*init)      (HAL_I2C_PERIPHERAL_STRUCT_POINTER periph);	//  Initialization
    int32_t (*deinit)	 (HAL_I2C_PERIPHERAL_STRUCT_POINTER periph);	//  Termination 
    int32_t (*delay)(uint32_t us);  //microsecond delay
    int32_t (*timer_start)(util_timer_t *timerval);  //start timing
    int32_t (*timer_differ)(util_timer_t *timerval);  //check timeout
    int32_t (*transmit)  (HAL_I2C_PERIPHERAL_STRUCT_POINTER periph, uint8_t *data, uint32_t  data_len);  //Send data 
    int32_t (*receive)   (HAL_I2C_PERIPHERAL_STRUCT_POINTER periph, uint8_t *data, uint32_t *data_len);         // Receive data 
    int32_t (*control)   (HAL_I2C_PERIPHERAL_STRUCT_POINTER periph, uint32_t ctrlcode, uint8_t *data, uint32_t  data_len);    //Send and receive control commands
#ifdef I2C_SUPPORT_GPIO_IRQ_RECEIVE
    int32_t (*gpio_irqwait_edge)(uint32_t wait_time);  //wait time for irq
#endif
    void *extra;                                               
};

/** Define I2C peripheral instance variable name */
#define I2C_PERIPHERAL_NAME(id) PERIPHERAL_NAME(PERIPHERAL_I2C, id)

/** Define I2C peripheral start */
#define I2C_PERIPHERAL_DEFINE_BEGIN(id) PERIPHERAL_DEFINE_BEGIN(PERIPHERAL_I2C, id)

/** Define I2C peripheral end */
#define I2C_PERIPHERAL_DEFINE_END() PERIPHERAL_DEFINE_END()

/** Register I2C peripheral*/
#define I2C_PERIPHERAL_REGISTER(id) PERIPHERAL_REGISTER(PERIPHERAL_I2C, id, PERIPHERAL_NAME(PERIPHERAL_I2C, id))

/** Declare I2C peripheral */
#define I2C_PERIPHERAL_DECLARE(id) PERIPHERAL_DECLARE(PERIPHERAL_I2C, id)  

#endif
