/**@file  key.h
* @brief  key interface declearation	 
* @author  liangww
* @date  2021-04-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/


#ifndef _KEY_H_
#define _KEY_H_

#include "apdu.h"

/** @addtogroup SE_Service
  * @{
  */

/** @addtogroup API
  * @{
  */


/** @defgroup KEY KEY
  * @brief key interface api.
  * @{
  */


/* Exported functions --------------------------------------------------------*/
/** @defgroup KEY_Exported_Functions KEY Exported Functions
  * @{
  */
se_error_t api_update_mkey (sym_key_t *mkey_new,sym_key_t *mkey, bool if_encrypt);

se_error_t  api_generate_keypair (pub_key_t *pub_key, pri_key_t *pri_key);

se_error_t  api_set_transkey (sym_key_t *key);

se_error_t  api_generate_symkey (sym_key_t * symkey);

se_error_t api_generate_shared_key (uint8_t *in_buf, uint32_t in_buf_len, unikey_t *shared_key, uint8_t *sm2_s, bool if_return_key, bool if_return_s);

se_error_t api_del_key(uint8_t id);

se_error_t  api_import_key(unikey_t *ukey, unikey_t *inkey ,bool if_cipher,bool if_trasns_key);

se_error_t  api_export_key(unikey_t *ukey, unikey_t *exkey, bool if_cipher,bool if_trasns_key);

se_error_t  api_get_key_info (bool     if_app_key, uint8_t *out_buf,uint32_t *out_buf_len);

#ifdef CONNECT_NEED_AUTH
se_error_t api_update_ekey (sym_key_t *ekey,sym_key_t *mkey, bool if_encrypt);
#endif


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
