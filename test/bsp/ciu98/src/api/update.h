/**@file  update.h
* @brief  update interface declearation	 
* @author  ade
* @date  2020-06-02
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/

#ifndef _UPDATE_H_
#define _UPDATE_H_

#include "apdu.h"

/** @addtogroup SE_Service
  * @{
  */

/** @addtogroup API
  * @{
  */


/** @defgroup UPDATE UPDATE
  * @brief update interface api.
  * @{
  */


/* Exported functions --------------------------------------------------------*/
/** @defgroup UPDATE_Exported_Functions UPDATE Exported Functions
  * @{
  */

se_error_t api_loader_download(uint8_t *image_addr, uint32_t image_len);

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
