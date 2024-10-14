/**@file  crypto.h
* @brief  crypto interface declearation	 
* @author liangww
* @date  2021-05-12
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/


#ifndef _CRYPTO_H_
#define _CRYPTO_H_

#include "apdu.h"

/** @addtogroup SE_Service
  * @{
  */

/** @addtogroup API
  * @{
  */


/** @defgroup CPYPTO CPYPTO
  * @brief cpypto interface api.
  * @{
  */


/* Exported functions --------------------------------------------------------*/
/** @defgroup CPYPTO_Exported_Functions CPYPTO Exported Functions
  * @{
  */



se_error_t api_sym_encrypt (sym_key_t *key, alg_sym_param_t *sym_param, const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len);


se_error_t api_sym_decrypt (sym_key_t *key, alg_sym_param_t *sym_param, const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len);


se_error_t api_mac (sym_key_t *key,  alg_sym_param_t *sym_param,const uint8_t *in_buf,uint32_t in_buf_len, uint8_t *mac, uint32_t *mac_len);


se_error_t  api_mac_verify (sym_key_t *key, alg_sym_param_t *sym_param,const uint8_t *in_buf,uint32_t in_buf_len,const uint8_t *mac, uint32_t mac_len);


se_error_t api_asym_encrypt(pub_key_t *key, alg_asym_param_t *asym_param, const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len);


se_error_t api_asym_decrypt(pri_key_t *key, alg_asym_param_t *asym_param ,const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len);


se_error_t api_asym_sign (pri_key_t *key, alg_asym_param_t *asym_param,const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *sign_buf, uint32_t *sign_buf_len);


se_error_t api_asym_verify (pub_key_t *key, alg_asym_param_t *asym_param,const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *sign_buf, uint32_t *sign_buf_len);

se_error_t api_sm2_get_za (user_id_t* uid, pub_key_t *pub_key , uint8_t *za );


se_error_t api_digest (uint32_t alg, const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len);


se_error_t  api_get_random  (uint32_t expected_len, uint8_t *random);

se_error_t api_sym_encrypt_cascade (sym_key_t *key, alg_sym_param_t *sym_param, const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len, bool if_first_block, bool if_last_block);

se_error_t api_sym_decrypt_cascade (sym_key_t *key, alg_sym_param_t *sym_param, const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len, bool if_first_block, bool if_last_block);

se_error_t api_digest_cascade (uint32_t alg, const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len ,bool if_last_block);

se_error_t api_mac_cascade (sym_key_t *key,  alg_sym_param_t *sym_param, const uint8_t *in_buf,uint32_t in_buf_len, uint8_t *mac, uint32_t *mac_len, bool if_first_block, bool if_last_block);


se_error_t  api_mac_verify_cascade (sym_key_t *key, alg_sym_param_t *sym_param,const uint8_t *in_buf,uint32_t in_buf_len,const uint8_t *mac, uint32_t mac_len, bool if_first_block, bool if_last_block );

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
