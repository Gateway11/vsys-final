/**@file  peripheral.h
* @brief  abstraction layer interface declearation    
* @author  zhengwd
* @date  2021-04-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/


#ifndef _PERIPHERAL_H_
#define _PERIPHERAL_H_

/**************************************************************************
* Global Macro Definition
***************************************************************************/

/** @addtogroup SE_Driver
  * @{
  */

/** @addtogroup PROTO 
  * @brief link protocol layer.
  * @{
  */


/** @addtogroup PERIPHERAL 
  * @{
  */



/* peripheral define --------------------------------------------------------*/
/** @defgroup peripheral_bus_driver_declearation peripheral bus driver declearation
  * @{
  */


#define MAX_PERIPHERAL_DEVICE                   4     /*!< Maximum number of devices supported by each communication interface*/  
#define MAX_PERIPHERAL_BUS_DRIVER         4     /*!< Maximum number of communication interfaces supported*/  
//#define SPI_SUPPORT_GPIO_IRQ_RECEIVE                /*!< SPI IRQ mode receive data*/
//#define I2C_SUPPORT_GPIO_IRQ_RECEIVE                /*!< I2C IRQ mode receive data*/


/**
  * @brief  Peripheral Type Structure definition
  */
typedef enum _peripheral_type {	
	PERIPHERAL_SPI,   ///< Peripherals of SPI communication interface
	PERIPHERAL_I2C,   ///< Peripherals of I2C communication interface
	PERIPHERAL_UART, ///<  Peripherals of UART communication interface
	PERIPHERAL_USB  ///<   Peripherals of USB communication interface
}peripheral_type;


/**
  * @brief  Peripheral Structure definition
  */
typedef struct _peripheral {
	peripheral_type type; ///< Peripheral communication interface type
	uint8_t id;	 ///< Peripheral identification
}peripheral;

/** Define peripheral structure type name */
#define PERIPHERAL_STRUCT_DEFINE(periph_type) struct peripheral_##periph_type  

/** Define the name of the peripheral instance variable */
#define PERIPHERAL_NAME(periph_type, id) g_peripheral_device_##periph_type##id

/** Define pointers to peripheral instances*/
#define PERIPHERAL_POINTER(periph_type, id) p_peripheral_device_##periph_type##id

/** Define peripheral start */
#define PERIPHERAL_DEFINE_BEGIN(periph_type, id) PERIPHERAL_STRUCT_DEFINE(periph_type) PERIPHERAL_NAME(periph_type, id) = {	\
	{periph_type, id},

/** Define peripheral end */
#define PERIPHERAL_DEFINE_END()  };

/** Register peripherals*/
//#define PERIPHERAL_REGISTER(periph_type, id, periph) const PERIPHERAL_STRUCT_DEFINE(periph_type) *PERIPHERAL_NAME(periph_type, id)_ = &periph
#define PERIPHERAL_REGISTER(periph_type, id, periph) PERIPHERAL_STRUCT_DEFINE(periph_type) *PERIPHERAL_POINTER(periph_type, id)= &periph


/** Get peripherals*/
#define PERIPHERAL_GET(periph_type, id) PERIPHERAL_POINTER(periph_type, id)

/** Declare peripherals */
//#define PERIPHERAL_DECLARE(periph_type, id) extern const PERIPHERAL_STRUCT_DEFINE(periph_type) *PERIPHERAL_NAME(periph_type, id)_
#define PERIPHERAL_DECLARE(periph_type, id) extern PERIPHERAL_STRUCT_DEFINE(periph_type) *PERIPHERAL_POINTER(periph_type, id)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*Bus type */


/**
  * @brief Peripheral bus driver abstract interface
  */
typedef struct _peripheral_bus_driver {
	peripheral_type type;  ///< Peripheral type
	peripheral    *periph[MAX_PERIPHERAL_DEVICE];  ///< Bus peripherals
	se_error_t (*init)		 (peripheral *periph);	  ///< Init 
	se_error_t (*deinit)	 (peripheral *periph);	  ///< Termination
	se_error_t (*open)      (peripheral *periph , uint8_t *rbuf, uint32_t *rlen);  ///< Open
	se_error_t (*close)     (peripheral *periph);	 ///< Close
	se_error_t (*transceive)(peripheral *periph ,  uint8_t *sbuf, uint32_t slen, uint8_t  *rbuf, uint32_t *rlen); ///< Data sending and receiving
	se_error_t (*reset)      (peripheral *periph , uint8_t *rbuf, uint32_t *rlen); ///< RESET
	se_error_t (*control)   (peripheral *periph , uint32_t ctrlcode, uint8_t *sbuf, uint32_t slen, uint8_t  *rbuf, uint32_t *rlen);	  ///< control commands
	se_error_t (*delay)   (peripheral *periph , uint32_t us);	  //subtle delay
	void *extra;  ///< RFU
}peripheral_bus_driver;

/** Register the bus driver */
//#define PERIPHERAL_BUS_DRIVER_REGISTER(periph_type, driver) const  peripheral_bus_driver *g_peripheral_bus_driver_##periph_type = &driver
#define PERIPHERAL_BUS_DRIVER_REGISTER(periph_type, driver) peripheral_bus_driver *g_peripheral_bus_driver_##periph_type = &driver


/** Get bus peripheral driver */
#define PERIPHERAL_BUS_DRIVER_GET(periph_type) g_peripheral_bus_driver_##periph_type

/** Declare bus peripheral driver*/
//#define PERIPHERAL_BUS_DRIVER_DECLARE(periph_type) extern const  peripheral_bus_driver *g_peripheral_bus_driver_##periph_type
#define PERIPHERAL_BUS_DRIVER_DECLARE(periph_type) extern  peripheral_bus_driver *g_peripheral_bus_driver_##periph_type

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


#endif // _PERIPHERAL_DRIVER_H_

