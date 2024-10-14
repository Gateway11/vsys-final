/**@file  auth.h
* @brief  auth interface declearation	 
* @author  liangww
* @date  2021-04-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/


#ifndef _AUTH_H_
#define _AUTH_H_

#include "apdu.h"

/** @addtogroup SE_Service
  * @{
  */

/** @addtogroup API
  * @{
  */


/** @defgroup AUTH AUTH
  * @brief auth interface api.
  * @{
  */




/* Exported functions --------------------------------------------------------*/
/** @defgroup AUTH_Exported_Functions AUTH Exported Functions
  * @{
  */


se_error_t api_ext_auth (const uint8_t *in_buf, uint32_t in_buf_len);

se_error_t api_pair_auth (const uint8_t *in_buf, uint32_t in_buf_len);



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
