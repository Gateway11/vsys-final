/**@file  v2x.c
* @brief  extern v2x interface definition
* @author  liangww
* @date  2021-10-21
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/

/***************************************************************************
* Include Header Files
***************************************************************************/
#include "v2x.h"


/** @addtogroup SE_Service
  * @{
  */

/** @addtogroup API 
  * @brief API layer.
  * @{
  */


/** @defgroup V2X V2X
  * @brief v2x interface api.
  * @{
  */



/* Exported functions --------------------------------------------------------*/

/** @defgroup V2X_Exported_Functions V2X Exported Functions
  * @{
  */

//V2x interfaces instruction
//1.Host device calls the v2x_gen_key_derive_seed interface to generate the derived seeds. (kS & kE & A & P) are sent to the PRA by EeRaCertRequest.   
//2.After receiving the SCTij, the host device verifies the signature by the public key of PCA certificate.  
//3.If the signature is verified successfully, the host device calls the v2x_reconsitution_key to obtain the private key sij stored in the SE and output the pseudonym certificate.  

/**
* @brief generate the key derived seeds
* @param [in]  derive_type    derive type:the default value is 0x00
* @param [out] out_buf        derive data information(can refer derive_seed_t)
* @return refer error.h
* @note Before calling v2x_gen_key_derive_seed api need obtain the administrator permission
* @see apdu_gen_key_derive_seed
*/
se_error_t v2x_gen_key_derive_seed (uint8_t derive_type,derive_seed_t* out_buf)
{
	se_error_t ret = 0;
	
	if(out_buf==NULL)
      return SE_ERR_PARAM_INVALID;

	//call the apdu_v2x_gen_key_derive_seed interface 
    ret =  apdu_v2x_gen_key_derive_seed(0x00, out_buf);
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call apdu_v2x_gen_key_derive_seed!\n");
		return ret;
	}

	return SE_SUCCESS;	

}


/**
* @brief reconstruct the private key
* @param [in]  in_buf      reconstruct data info(private KID-1byte|i-4byte|j-4B|CTij)
* @param [in]  in_buf_len  reconstruct data info length
* @param [out] out_buf     pseudonym certificate
* @param [out] out_buf_len pseudonym certificate length
* @return refer error.h
* @note must excute the v2x_gen_key_derive_seed and obtain the security file write permission before 
* @see apdu_reconsitution_key
*/
se_error_t v2x_reconsitution_key (uint8_t *in_buf, uint32_t *in_buf_len,uint8_t *out_buf, uint32_t *out_buf_len)
{
	se_error_t ret = 0;

	if(in_buf==NULL||*in_buf_len==0||out_buf==NULL||out_buf_len==NULL)
        return SE_ERR_PARAM_INVALID;

	
	//call the apdu_v2x_reconsitution_key interface 
    ret =  apdu_v2x_reconsitution_key(in_buf, in_buf_len, out_buf, out_buf_len);
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call apdu_v2x_reconsitution_key!\n");
		return ret;
	}

	return SE_SUCCESS;	
}


/**
* @brief get derive seed
* @param [out] out_buf    derive data information(can refer derive_seed_t)
* @return refer error.h
* @note guarantee the derive seed have been generated 
* @see apdu_v2x_get_derive_seed
*/
se_error_t v2x_get_derive_seed (derive_seed_t* out_buf)
{
	se_error_t ret = 0;
	
	if(out_buf==NULL)
      return SE_ERR_PARAM_INVALID;

	//call the apdu_v2x_get_derive_seed interface
    ret =  apdu_v2x_get_derive_seed(out_buf);
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call apdu_v2x_get_derive_seed!\n");
		return ret;
	}

	return SE_SUCCESS;	
	
}

/**
* @brief private key multiply-add interface
* @param [in]  input_blod_key_id         the private key id for k stored in SE: fixed key ID : 00-0xEF, can not be exported; temporary key ID: 0xF0-0xFE, can be exported
* @param [in]  output_blod_key_id        the private key id for k' to store into SE: fixed key ID : 00-0xEF, can not be exported; temporary key ID: 0xF0-0xFE, can be exported
* @param [in]  curve_type     curve type(curve type only support SM2 right now )    
* @param [in]  key_multiplier a
* @param [in]  key_addend     b
* @param [out] pubkey->value         the public key corresponding the k'
* @param [out] pubkey->value_len     the public key length corresponding the k'
* @return refer util_error.h
* @note  obtain the security file write permission before calling the api
* @see apdu_private_key_multiply_add
*/
se_error_t v2x_private_key_multiply_add (uint8_t input_blod_key_id, uint8_t output_blod_key_id, enum ecc_curve curve_type, key_multiplier_t key_multiplier, key_addend_t key_addend,  pub_key_t* pubkey)
{
	se_error_t ret = 0;
	
	if((input_blod_key_id > 0xff) || (output_blod_key_id > 0xff))
	{
		LOGE("failed to call v2x_private_key_multiply_add!\n");
		return SE_ERR_PARAM_INVALID;		
	}

	if(curve_type != SM2_CURVE)
	{
		LOGE("failed to call v2x_private_key_multiply_add!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(pubkey == NULL)
	{
		LOGE("failed to call v2x_private_key_multiply_add!\n");
		return SE_ERR_PARAM_INVALID;		
	}

	//call apdu_private_key_multiply_add interface
    ret =  apdu_private_key_multiply_add(input_blod_key_id, output_blod_key_id, curve_type, key_multiplier,key_addend, pubkey);
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call apdu_private_key_multiply_add!\n");
		return ret;
	}

	return SE_SUCCESS;	
}



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
 
