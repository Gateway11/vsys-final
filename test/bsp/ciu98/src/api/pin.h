/**@file  pin.h
* @brief  pin interface declearation	 
* @author  liangww
* @date  2021-06-02
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/

#ifndef _PIN_H_
#define _PIN_H_

#include "apdu.h"

/** @addtogroup SE_Service
  * @{
  */

/** @addtogroup API
  * @{
  */


/** @defgroup PIN PIN
  * @brief pin interface api.
  * @{
  */


/* Exported functions --------------------------------------------------------*/
/** @defgroup PIN_Exported_Functions PIN  Exported Functions
  * @{
  */
se_error_t  api_write_pin(pin_t *pin, bool if_encrypt);


se_error_t api_change_pin (pin_t *pin, const uint8_t *in_buf, uint32_t in_buf_len);


se_error_t api_reload_pin  (pin_t *pin, const uint8_t *in_buf, uint32_t in_buf_len);

se_error_t api_verify_pin  (pin_t *pin);


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
