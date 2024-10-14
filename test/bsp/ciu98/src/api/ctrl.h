/**@file  ctrl.h
* @brief  ctrl interface declearation    
* @author  liangww
* @date  2021-04-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/

#ifndef _CTRL_H_
#define _CTRL_H_

#include "apdu.h"
#include "comm.h"




/** @addtogroup SE_Service
  * @{
  */

/** @addtogroup API
  * @{
  */


/** @defgroup CTRL CTRL
  * @brief ctrl interface api.
  * @{
  */


/* Exported functions --------------------------------------------------------*/
/** @defgroup CTRL_Exported_Functions CTRL Exported Functions
  * @{
  */




se_error_t api_switch_mode (work_mode  type);

se_error_t api_control(ctrl_type ctrlcode, const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len);

se_error_t api_power_on (void);

se_error_t api_power_off (void);


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
