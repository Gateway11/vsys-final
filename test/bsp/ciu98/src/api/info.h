/**@file  info.h
* @brief  info interface declearation	 
* @author liangww
* @date  2021-05-12
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/


#ifndef _INFO_H_
#define _INFO_H_

#define SDK_VERSION_NUM 0x01020702
#define SDK_VERSION_STRING "CIU98_B V2 Host SDK[V1.2.7b-release]"


#include "apdu.h"

/** @addtogroup SE_Service
  * @{
  */

/** @addtogroup API
  * @{
  */


/** @defgroup INFO INFO
  * @brief info interface api.
  * @{
  */


/* Exported functions --------------------------------------------------------*/
/** @defgroup INFO_Exported_Functions INFO Exported Functions
  * @{
  */
se_error_t  api_get_info (info_type type, se_info_t * info);


se_error_t  api_get_id (se_id_t *se_id );


se_error_t  api_sdk_version_number(uint32_t *num);


se_error_t  api_sdk_version_string (char *str);

se_error_t  api_write_id (se_id_t *se_id);

se_error_t  api_update_EM_flag (uint8_t pub_kid,  uint8_t *sig, uint32_t sig_len, uint8_t *random, uint32_t random_len, uint16_t EM_DID);

se_error_t  api_read_EM_flag (uint8_t *EM_flag);

se_error_t  api_get_IVD (IVD_type ivd_type ,uint8_t *IVD_value);

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
