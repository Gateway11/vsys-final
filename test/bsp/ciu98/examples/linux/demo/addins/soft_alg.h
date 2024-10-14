/**@file  soft_alg.h
* @brief  soft_alg interface declearation	 
* @author liuch
* @date  2020-06-02
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/

#ifndef __SOFT_ALG_H__
#define __SOFT_ALG_H__

#include <stdint.h>
#include "sm4.h"
#include "sm3.h"
#include "string.h"


/** @addtogroup SE_APP_TEST
  * @{
  */


/** @addtogroup ADDINS
  * @{
  */


/** @defgroup SOFT_ALG SOFT_ALG
  * @brief soft_alg interface api.
  * @{
  */





/* Exported types ------------------------------------------------------------*/
/** @defgroup SOFT_ALG_Exported_Types SOFT_ALG Exported Types
  * @{
  */

/**
  * @brief ECB_CBC_mode definition
  */
//ECB/CBC mode
#define ECB 0
#define CBC 1
/**
  * @}
  */



/**
  * @brief ICC_ECB_CBC_mode definition
  */
#define ICC_ECB	0
#define ICC_CBC	1
/**
  * @}
  */


//ENCRYPT/DECRYPT mode
/**
  * @brief Encrypt_mode definition
  */
#define ENCRYPT 0
#define DECRYPT 1
/**
  * @}
  */

#define BUFFER_MAX_LEN 1024

/**
  * @brief Soft_Error definition
  */
#define EXCUTE_SUC		0x9000
#define CER_HEXSTR		0xff30
#define CER_DECSTR		0xff31
#define CER_BINSTR		0xff32
#define CER_LARGE		0xff33
#define CER_MODE		0xff34
#define CER_KEYLEN		0xff35
#define CER_DATALEN		0xff36
#define CER_CBC			0xff37
#define CER_RESULT		0xff38
#define CER_UNKNOWN     0xffff
/**
  * @}
  */



/* Exported functions --------------------------------------------------------*/
/** @defgroup SOFT_ALG_Exported_Functions SOFT_ALG Exported Functions
  * @{
  */
uint32_t ex_sm4_enc_dec(uint8_t* in_buf,uint32_t in_buf_len,uint8_t* key_buf,uint32_t key_buf_len,uint32_t ecb_cbc_mode,uint32_t enc_dec_mode,uint8_t* out_buf,uint32_t*out_buf_len);

uint32_t ex_sm4_mac(uint8_t* in_buf,uint32_t in_buf_len,uint8_t* iv,uint32_t iv_len,uint8_t* key_buf,uint32_t key_buf_len,uint8_t* out_buf,uint32_t*out_buf_len);

uint32_t ex_SM3_digest( uint8_t *in_buf, uint32_t in_buf_len,uint8_t *output);

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

/**
  * @}
  */

#endif
