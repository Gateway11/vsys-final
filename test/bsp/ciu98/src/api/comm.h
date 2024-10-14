/**@file  comm.h
* @brief  comm interface declearation	 
* @author  liangww
* @date  2021-04-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/


#ifndef _COMM_H_
#define _COMM_H_

#include <stdint.h>
#include <string.h>
#include "config.h"
#include "types.h"
#include "error.h"
#include "log.h"
#include "peripheral.h"
#include "port_config.h"
#include "apdu.h"
//#include "port_util.h"

#include "ial_spi.h"
#include "proto_spi.h"
#include "port_spi.h"

#include "ial_i2c.h"
#include "proto_i2c.h"
#include "port_i2c.h"




/** @addtogroup SE_Service
  * @{
  */

/** @addtogroup API
  * @{
  */


/** @defgroup COMM COMM
  * @brief comm interface api.
  * @{
  */



/* Private constants --------------------------------------------------------*/
/** @defgroup Proto_Spi_Impl_Private_Constants Proto_Spi_Impl Private Constants
  * @{
  */

/** @defgroup BUS_DRIVER    MAX BUS DRIVER Define
  * @{
  */
#define MAX_BUS_DRIVER 4
/**
  * @}
  */

/** @defgroup COMM_DATA    COMM_DATA Limit Define
  * @{
  */
#define COMM_DATA_LEN_MIN        5   
#define COMM_DATA_LEN_MAX        1024  
/**
  * @}
  */

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @defgroup COMM_Exported_Functions COMM Exported Functions
  * @{
  */
// Complete the registration of all peripherals in the main task
#define api_register(periph_type, periph_id) _api_register((peripheral_bus_driver *)PERIPHERAL_BUS_DRIVER_GET(periph_type), (peripheral *)PERIPHERAL_GET(periph_type, periph_id))

se_error_t _api_register(peripheral_bus_driver *driver, peripheral *periph);

se_error_t api_select(peripheral_type periph_type, uint32_t periph_id);

se_error_t api_connect (uint8_t *out_buf, uint32_t *out_buf_len);

#ifdef CONNECT_NEED_AUTH
se_error_t api_connect_auth (sym_key_t *ekey,  uint8_t *out_buf, uint32_t *out_buf_len);
#endif
se_error_t api_disconnect (void);

se_error_t api_transceive(const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len);

se_error_t api_reset (uint8_t *out_buf, uint32_t *out_buf_len);

se_error_t _api_control(uint32_t ctrlcode, uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len);

se_error_t api_transceive_queue(uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len);

se_error_t api_delay (uint32_t us);

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


