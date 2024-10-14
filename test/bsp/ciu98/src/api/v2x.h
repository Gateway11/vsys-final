/**@file  v2x.h
* @brief  extern v2x interface declearation	 
* @author  liangww
* @date  2022-7-26
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/


#ifndef _V2X_H_
#define _V2X_H_

#include "apdu.h"

/** @addtogroup SE_Service
  * @{
  */

/** @addtogroup API
  * @{
  */


/** @defgroup V2X
  * @brief v2x interface api.
  * @{
  */




/* Exported functions --------------------------------------------------------*/
/** @defgroup V2X_Exported_Functions V2X Exported Functions
  * @{
  */

se_error_t v2x_gen_key_derive_seed (uint8_t derive_type,derive_seed_t* out_buf);

se_error_t v2x_reconsitution_key (uint8_t *in_buf, uint32_t *in_buf_len,uint8_t *out_buf, uint32_t *out_buf_len);

se_error_t v2x_get_derive_seed (derive_seed_t* out_buf);

se_error_t v2x_private_key_multiply_add (uint8_t input_blod_key_id, uint8_t output_blod_key_id,enum ecc_curve curve_type, key_multiplier_t key_multiplier, key_addend_t key_addend,  pub_key_t* pubkey);



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
