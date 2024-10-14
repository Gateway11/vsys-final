/**@file  fs.h
* @brief  fs interface declearation	 
* @author  liangww
* @date  2021-05-12
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/

#ifndef _FS_H_
#define _FS_H_

#include "apdu.h"

/** @addtogroup SE_Service
  * @{
  */

/** @addtogroup API
  * @{
  */


/** @defgroup FS FS
  * @brief fs interface api.
  * @{
  */


/* Exported functions --------------------------------------------------------*/
/** @defgroup FS_Exported_Functions FS Exported Functions
  * @{
  */
se_error_t  api_select_file(uint32_t  fid);


se_error_t api_write_file (uint32_t offset, const uint8_t *in_buf, uint32_t in_buf_len, bool if_encrypt);


se_error_t api_read_file  (uint32_t offset, uint32_t expect_len , bool if_encrypt,uint8_t *out_buf, uint32_t *out_buf_len);

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
