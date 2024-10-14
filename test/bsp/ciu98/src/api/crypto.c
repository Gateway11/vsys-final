/**@file  crypto.c
* @brief  crypto interface definition
* @author liuch
* @date  2020-06-02
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/

/***************************************************************************
* Include Header Files
***************************************************************************/
#include "crypto.h"


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


/**
* @brief Symmetric encryption
* @param [in] key->alg  				ALG_AES128/ALG_DES128/ALG_SM4
* @param [in] key->id   				kid
* @param [in] sym_param->mode  			symmetric encryption mode
* @param [in] sym_param->iv  			IV
* @param [in] sym_param->iv_len  		IV length
* @param [in] sym_param->padding_type 	PADDING_NOPADDING/PADDING_PKCS7
* @param [in] in_buf  					Plaintext data 
* @param [in] in_buf_len  				Plaintext data length
* @param [out] out_buf  				Ciphertext data
* @param [out] out_buf_len  			Ciphertext data length
* @return  refer error.h
* @note no
* @see apdu_sym_enc_dec
*/
se_error_t api_sym_encrypt (sym_key_t *key, alg_sym_param_t *sym_param, const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len)
{

    se_error_t ret = 0;
	uint32_t block = DBLOCK;
	uint32_t count = 0;
	uint32_t padding_len = in_buf_len;
	uint32_t temp_len = in_buf_len;
	uint32_t seg_len = 0;
	uint32_t off = 0;
	uint32_t out_len = 0;
	bool if_last_block =false;
	bool if_first_block =true;
	//parameter check
	if(key==NULL||sym_param==NULL||in_buf==NULL||in_buf_len==0)
	{  
		LOGE("failed to api_sym_encrypt input params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if(out_buf==NULL||out_buf_len==NULL)
	{  
		LOGE("failed to api_sym_encrypt output params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(key->alg!=ALG_AES128&&key->alg!=ALG_DES128&&key->alg!=ALG_SM4)
	{  
		LOGE("failed to api_sym_encrypt alg params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(sym_param->mode!=SYM_MODE_CBC&&sym_param->mode!=SYM_MODE_ECB)
	{  
		LOGE("failed to api_sym_encrypt mode params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if(sym_param->mode==SYM_MODE_CBC)
	{
		if(key->alg==ALG_DES128)
		{
			if(sym_param->iv_len!=8)
            {
				LOGE("failed to api_sym_encrypt iv params!\n");
				return SE_ERR_PARAM_INVALID;
			}
		}
		else
		{
			if(sym_param->iv_len!=16)
            {
				LOGE("failed to api_sym_encrypt iv params!\n");
				return SE_ERR_PARAM_INVALID;
			}
		}
	}
	if(sym_param->padding_type!=PADDING_NOPADDING&&sym_param->padding_type!=PADDING_PKCS7)
	{  
		LOGE("failed to api_sym_encrypt padding params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(sym_param->padding_type==PADDING_NOPADDING)
	{
		if(key->alg==ALG_DES128)
		{
			if(in_buf_len%8!=0)
			{
				LOGE("failed to api_sym_encrypt input len params!\n");
				return SE_ERR_PARAM_INVALID;
			}
		}
		else
		{
			if(in_buf_len%16!=0)
			{
				LOGE("failed to api_sym_encrypt input len params!\n");
				return SE_ERR_PARAM_INVALID;
			}
		}
	}
	else//the length of the padding data after pkcs7
	{
		padding_len =  apdu_sym_padding_length(key->alg,PADDING_PKCS7,in_buf_len);
	}

	//at least 1 block
	count = temp_len/block+1;
	if(temp_len%block==0)
		count--;

	//Send data cyclically
	while(count>0)
	{			
		
		if(temp_len<=block)
		{
			seg_len = temp_len;
		}			
		else
			seg_len = block;
		
		if(count==1)
		{
		    //if the block is the last one, set the vlue of if_last_block true.  
			if_last_block = true;
			if(sym_param->padding_type==PADDING_PKCS7)
				out_len =  apdu_sym_padding_length(key->alg,PADDING_PKCS7,seg_len);
		}
		ret = apdu_sym_enc_dec (key,sym_param,in_buf+off, seg_len, out_buf+off, &out_len,if_first_block,if_last_block,true);
		if(ret!=SE_SUCCESS)
		{	
			LOGE("failed to call apdu_sym_enc_dec!\n");
			return ret;
		}
		if_first_block =false;
		off+=seg_len;
		count--;
		temp_len-=seg_len;
	}
	//Ciphertext length
	*out_buf_len = padding_len;
	return SE_SUCCESS;
}


/**
* @brief Symmetric decryption
* @param [in] key ->alg               ALG_AES128/ALG_DES128/ALG_SM4
* @param [in] key->id                 kid
* @param [in] sym_param ->mode        symmetric encryption mode
* @param [in] sym_param ->iv          IV
* @param [in] sym_param ->iv_len      IV length
* @param [in] sym_param ->padding_type  PADDING_NOPADDING/PADDING_PKCS7
* @param [in] in_buf                  Ciphertext data 
* @param [in] in_buf_len              Ciphertext data length
* @param [out] out_buf                Plaintext data
* @param [out] out_buf_len            Plaintext data length
* @return refer error.h
* @note no
* @see  apdu_sym_enc_dec
*/
se_error_t api_sym_decrypt (sym_key_t *key, alg_sym_param_t *sym_param, const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len)
{
	se_error_t ret = 0;
	uint32_t block = DBLOCK;
	uint32_t count = 0;
	uint32_t padding_len = 0;
	uint32_t temp_len = in_buf_len;
	uint32_t seg_len = 0;
	uint32_t off = 0;
	uint32_t out_len = 0;
	bool if_last_block =false;
	bool if_first_block =true;
	//parameter check
	if(key==NULL||sym_param==NULL||in_buf==NULL||in_buf_len==0)
	{  
		LOGE("failed to api_sym_decrypt input params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if(out_buf==NULL||out_buf_len==NULL)
	{  
		LOGE("failed to api_sym_decrypt output params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(key->alg!=ALG_AES128&&key->alg!=ALG_DES128&&key->alg!=ALG_SM4)
	{  
		LOGE("failed to api_sym_decrypt alg params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if(sym_param->mode!=SYM_MODE_CBC&&sym_param->mode!=SYM_MODE_ECB)
	{  
		LOGE("failed to api_sym_decrypt mode params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if(sym_param->mode==SYM_MODE_CBC)
	{
		if(key->alg==ALG_DES128)
		{
			if(sym_param->iv_len!=8)
            {
				LOGE("failed to api_sym_decrypt iv params!\n");
				return SE_ERR_PARAM_INVALID;
			}
		}
		else
		{
			if(sym_param->iv_len!=16)
            {
				LOGE("failed to api_sym_decrypt iv params!\n");
				return SE_ERR_PARAM_INVALID;
			}
		}
	}
	if(sym_param->padding_type!=PADDING_NOPADDING&&sym_param->padding_type!=PADDING_PKCS7)
	{  
		LOGE("failed to api_sym_decrypt padding params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(sym_param->padding_type==PADDING_NOPADDING)
	{
		if(key->alg==ALG_DES128)
		{
			if(in_buf_len%8!=0)
			{
				LOGE("failed to api_sym_decrypt input len params!\n");
				return SE_ERR_PARAM_INVALID;
			}
		}
		else
		{
			if(in_buf_len%16!=0)
			{
				LOGE("failed to api_sym_decrypt input len params!\n");
				return SE_ERR_PARAM_INVALID;
			}
		}
	}
	//at least 1 block
	count = temp_len/block+1;

	if(temp_len%block==0)
		count--;

	//send data cyclically
	while(count>0)
	{				
		if(temp_len<=block)
		{
			seg_len = temp_len;
		}			
		else
			seg_len = block;
		if(count==1)
		{
			//if the block is the last one, set the vlue of if_last_block true. 
			if_last_block = true;
		}
		ret = apdu_sym_enc_dec (key,sym_param,in_buf+off, seg_len, out_buf+off, &out_len,if_first_block,if_last_block,false);
		if(ret!=SE_SUCCESS)
		{	
			LOGE("failed to call apdu_sym_enc_dec!\n");
			return ret;
		}
		if_first_block =false;
		off+=seg_len;
		count--;
		temp_len-=seg_len;
		padding_len += out_len;
	}
	*out_buf_len = padding_len;

	return SE_SUCCESS;
}


/**
* @brief Calculate MAC 
* @param [in] key ->alg               ALG_AES128/ALG_DES128/ALG_SM4
* @param [in] key->id                 kid
* @param [in] sym_param ->iv          IV
* @param [in] sym_param ->iv_len      IV length
* @param [in] sym_param ->padding_type  PADDING_NOPADDING/PADDING_ISO9797_M1/PADDING_ISO9797_M2
* @param [in] in_buf                  MAC input data 
* @param [in] in_buf_len              MAC input data length
* @param [out] out_buf                MAC data
* @param [out] out_buf_len            MAC data length
* @return refer error.h
* @note no
* @see  apdu_mac
*/
se_error_t api_mac (sym_key_t *key,  alg_sym_param_t *sym_param,const uint8_t *in_buf,uint32_t in_buf_len, uint8_t *mac, uint32_t *mac_len)
{
	se_error_t ret = 0;
	uint32_t block = DBLOCK;
	uint32_t count = 0;
	uint32_t temp_len = in_buf_len;
	uint32_t seg_len = 0;
	uint32_t off = 0;
	//uint32_t iv_len = 0;
	bool if_last_block =false;
	bool if_first_block =true;

	//parameter check
	if(key==NULL||sym_param==NULL||in_buf==NULL||in_buf_len==0)
	{  
		LOGE("failed to api_mac input params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if(mac==NULL||mac_len==NULL)
	{  
		LOGE("failed to api_mac output params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(key->alg!=ALG_AES128&&key->alg!=ALG_DES128&&key->alg!=ALG_SM4)
	{  
		LOGE("failed to api_mac alg params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(key->alg==ALG_DES128)
	{
		if(sym_param->iv_len!=8)
        {
			LOGE("failed to api_mac iv params!\n");
			return SE_ERR_PARAM_INVALID;
		}
	}
	else
	{
		if(sym_param->iv_len!=16)
        {
			LOGE("failed to api_mac iv params!\n");
			return SE_ERR_PARAM_INVALID;
		}
	}

	if(sym_param->padding_type!=PADDING_NOPADDING&&sym_param->padding_type!=PADDING_ISO9797_M1&&sym_param->padding_type!=PADDING_ISO9797_M2)
	{  
		LOGE("failed to api_mac padding params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(sym_param->padding_type==PADDING_NOPADDING)
	{
		if(key->alg==ALG_DES128)
		{
			if(in_buf_len%8!=0)
			{
				LOGE("failed to api_mac input len params!\n");
				return SE_ERR_PARAM_INVALID;
			}
		}
		else
		{
			if(in_buf_len%16!=0)
			{
				LOGE("failed to api_mac input len params!\n");
				return SE_ERR_PARAM_INVALID;
			}
		}
	}
	

	//at least 1 block
	count = temp_len/block+1;

	if(temp_len%block==0)
		count--;

	//Send data cyclically
	while(count>0)
	{			
		
		if(temp_len<=block)
		{
			seg_len = temp_len;
		}			
		else
			seg_len = block;
		
		if(count==1)
		{
			//if the block is the last one, set the vlue of if_last_block true. 
			if_last_block = true;
		}
		
		ret = apdu_mac (key,sym_param,in_buf+off, seg_len, mac, mac_len,if_first_block,if_last_block,true);
		if(ret!=SE_SUCCESS)
		{	
			LOGE("failed to call apdu_mac!\n");
			return ret;
		}
		if_first_block =false;
		off+=seg_len;
		count--;
		temp_len-=seg_len;
	}

	return SE_SUCCESS;	

}




/**
* @brief  MAC verify 
* @param [in] key ->alg               ALG_AES128/ALG_DES128/ALG_SM4
* @param [in] key->id                 kid
* @param [in] sym_param ->iv          IV
* @param [in] sym_param ->iv_len      IV length
* @param [in] sym_param ->padding_type  PADDING_NOPADDING/PADDING_ISO9797_M1/PADDING_ISO9797_M2
* @param [in] in_buf                  MAC input data 
* @param [in] in_buf_len              MAC input data length
* @param [in] mac                     MAC data
* @param [in] mac_len                 MAC data length
* @return refer error.h
* @note no
* @see  apdu_mac
*/
se_error_t  api_mac_verify (sym_key_t *key, alg_sym_param_t *sym_param,const uint8_t *in_buf,uint32_t in_buf_len,const uint8_t *mac, uint32_t mac_len)
{
	se_error_t ret = 0;
	uint32_t block = DBLOCK-16;
	int32_t count = 0;
	uint32_t temp_len = in_buf_len;
	uint32_t seg_len = 0;
	uint32_t off = 0;
	//uint32_t iv_len = 0;
	bool if_last_block =false;
	bool if_first_block =true;

	//parameter check
	if(key==NULL||sym_param==NULL||in_buf==NULL||in_buf_len==0)
	{  
		LOGE("failed to api_mac_verify input params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if(mac==NULL||mac_len==0)
	{  
		LOGE("failed to api_mac_verify output params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(key->alg!=ALG_AES128&&key->alg!=ALG_DES128&&key->alg!=ALG_SM4)
	{  
		LOGE("failed to api_mac_verify alg params!\n");
		return SE_ERR_PARAM_INVALID;
	}


	if(key->alg==ALG_DES128)
	{
		if(sym_param->iv_len!=8||mac_len!=8)
        {
			LOGE("failed to api_mac_verify iv params!\n");
			return SE_ERR_PARAM_INVALID;
		}
	}
	else
	{
		if(sym_param->iv_len!=16||mac_len!=16)
        {
			LOGE("failed to api_mac_verify iv params!\n");
			return SE_ERR_PARAM_INVALID;
		}
	}

	if(sym_param->padding_type!=PADDING_NOPADDING&&sym_param->padding_type!=PADDING_ISO9797_M1&&sym_param->padding_type!=PADDING_ISO9797_M2)
	{  
		LOGE("failed to api_mac_verify padding params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(sym_param->padding_type==PADDING_NOPADDING)
	{
		if(key->alg==ALG_DES128)
		{
			if(in_buf_len%8!=0)
			{
				LOGE("failed to api_mac_verify input len params!\n");
				return SE_ERR_PARAM_INVALID;
			}
		}
		else
		{
			if(in_buf_len%16!=0)
			{
				LOGE("failed to api_mac input len params!\n");
				return SE_ERR_PARAM_INVALID;
			}
		}
	}
	

	//at least 1 block
	count = temp_len/block+1;

	if(temp_len%block==0)
		count--;

	//Send data cyclically
	while(count>0)
	{			
		
		if(temp_len<=block)
		{
			seg_len = temp_len;
		}			
		else
			seg_len = block;
		//if the block is the last one, set the vlue of if_last_block true. 
		if(count==1)
		{
			if_last_block = true;
		}
		ret = apdu_mac (key,sym_param,in_buf+off, seg_len, (uint8_t*)mac, &mac_len,if_first_block,if_last_block,false);
		if(ret!=SE_SUCCESS)
		{	
			LOGE("failed to call apdu_mac!\n");
			return ret;
		}
		if_first_block =false;
		off+=seg_len;
		count--;
		if(temp_len!=0)
		{
			temp_len-=seg_len;
		}
	}

	return SE_SUCCESS;	
}


/**
* @brief Asymmetric encryption
* @param [in] key->alg  		ALG_RSA1024_CRT/ALG_RSA2048_CRT/ALG_ECC256/ALG_SM2
* @param [in] key->id   		public kid
* @param [in] asym_param-> 		padding_type  rsa padding mode(PADDING_NOPADDING/PADDING_PKCS1)(only valid for RSA algorithm)
* @param [in] in_buf 			plaintext data 
* @param [in] in_buf_len  		plaintext data length
* @param [out] out_buf  		ciphertext data
* @param [out] out_buf_len 		ciphertext data length
* @return refer error.h
* @note no
* @see apdu_asym_enc_dec
*/
se_error_t api_asym_encrypt(pub_key_t *key, alg_asym_param_t *asym_param, const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len)
{
 	se_error_t ret = 0;
	uint32_t block = DBLOCK+16;
	uint32_t count = 0;
	uint32_t temp_len = in_buf_len;
	uint32_t seg_len = 0;
	uint32_t off = 0;
	bool if_last_block =false;


	//parameter check
	if(key==NULL||in_buf==NULL||in_buf_len==0)
	{  
		LOGE("failed to api_asym_encrypt input params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(out_buf==NULL||out_buf_len==NULL)
	{  
		LOGE("failed to api_asym_encrypt output params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	
	if(key->alg!=ALG_ECC256_NIST&&key->alg!=ALG_SM2&&key->alg!=ALG_RSA1024_CRT&&key->alg!=ALG_RSA2048_CRT)
	{  
		LOGE("failed to api_asym_encrypt alg params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	//PADDING_NOPADDING-mold length PADDING_PKCS1-mold length-11 at most
	if(key->alg==ALG_RSA1024_CRT||key->alg==ALG_RSA2048_CRT)
	{
		if(asym_param->padding_type!=PADDING_NOPADDING&&asym_param->padding_type!=PADDING_PKCS1)
		{
			LOGE("failed to api_asym_encrypt padding params!\n");
			return SE_ERR_PARAM_INVALID;
		}
	}

	if(key->alg==ALG_ECC256_NIST||key->alg==ALG_SM2)
	{
		//SM2 and ECC,in_buf_len<=4096 bytes
		if(in_buf_len>4096)
		{
			LOGE("failed to api_asym_encrypt in_buf_len params!\n");
			return SE_ERR_PARAM_INVALID;
		}
	}
	else 
	{
		if(asym_param->padding_type==PADDING_NOPADDING)
		{
			//RSA,in_buf_len = mold length
			if(in_buf_len!=128&&in_buf_len!=256)
			{
				LOGE("failed to api_asym_encrypt in_buf_len params!\n");
				return SE_ERR_PARAM_INVALID;
			}
		}
		else
		{
			//RSA,in_buf_len <= (mold length-11)
			if((key->alg==ALG_RSA2048_CRT&&asym_param->padding_type==PADDING_PKCS1&&in_buf_len>245)||(key->alg==ALG_RSA1024_CRT&&asym_param->padding_type==PADDING_PKCS1&&in_buf_len>117))
			{
				LOGE("failed to api_asym_encrypt in_buf_len params!\n");
				return SE_ERR_PARAM_INVALID;
			}
		}
	}


	//at least 1 block
	count = temp_len/block+1;

	if(temp_len%block==0)
		count--;

	//send data cyclically
	while(count>0)
	{			
			
		if(temp_len<=block)
		{
			seg_len = temp_len;
		}			
		else
			seg_len = block;
	   //if the block is the last one, set the vlue of if_last_block true. 
		if(count==1)
		{
			if_last_block = true;
		}
		ret = apdu_asym_enc (key,asym_param, in_buf+off, seg_len, out_buf, out_buf_len,if_last_block);
		if(ret!=SE_SUCCESS)
		{	
			LOGE("failed to call api_asym_encrypt!\n");
			return ret;
		}
		off+=seg_len;
		count--;
		temp_len-=seg_len;
	}
	
	return SE_SUCCESS;	

}


/**
* @brief Asymmetric decryption
* @param [in] key->alg  				ALG_RSA1024_CRT/ALG_RSA2048_CRT/ALG_ECC256/ALG_SM2
* @param [in] key->id   				private kid
* @param [in] asym_param->padding_type  rsa padding mode(PADDING_NOPADDING/PADDING_PKCS1)(only valid for RSA algorithm)
* @param [in] in_buf  					ciphertext data  
* @param [in] in_buf_len  				ciphertext data length
* @param [out] out_buf  				plaintext data
* @param [out] out_buf_len 				plaintext data length
* @return refer error.h
* @note no
* @see apdu_asym_enc_dec
*/
se_error_t api_asym_decrypt(pri_key_t *key, alg_asym_param_t *asym_param ,const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len)
{
	se_error_t ret = 0;
	uint32_t block = DBLOCK+16;
	uint32_t count = 0;
	uint32_t temp_len = in_buf_len;
	uint32_t seg_len = 0;
	uint32_t off = 0;
	bool if_last_block =false;


	//parameter check
	if(key==NULL||in_buf==NULL||in_buf_len==0)
	{  
		LOGE("failed to api_asym_decrypt input params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(out_buf==NULL||out_buf_len==NULL)
	{  
		LOGE("failed to api_asym_decrypt output params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	
	if(key->alg!=ALG_ECC256_NIST&&key->alg!=ALG_SM2&&key->alg!=ALG_RSA1024_CRT&&key->alg!=ALG_RSA2048_CRT)
	{  
		LOGE("failed to api_asym_decrypt alg params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	//PADDING_NOPADDING-mold length PADDING_PKCS1-mold length-11 at most
	if(key->alg==ALG_RSA1024_CRT||key->alg==ALG_RSA2048_CRT)
	{
		if(asym_param->padding_type!=PADDING_NOPADDING&&asym_param->padding_type!=PADDING_PKCS1)
		{
			LOGE("failed to api_asym_decrypt padding params!\n");
			return SE_ERR_PARAM_INVALID;
		}
	}

	if(key->alg==ALG_ECC256_NIST||key->alg==ALG_SM2)
	{
		//SM2 and ECC,in_buf_len<=4096+96 bytes
		if(in_buf_len>4096+96)
		{
			LOGE("failed to api_asym_decrypt in_buf_len params!\n");
			return SE_ERR_PARAM_INVALID;
		}
	}
	else 
	{
		//RSA,in_buf_len = mold length
		if(in_buf_len!=128&&in_buf_len!=256)
		{
			LOGE("failed to api_asym_decrypt in_buf_len params!\n");
			return SE_ERR_PARAM_INVALID;
		}

	}

	//at least 1 block
	count = temp_len/block+1;

	if(temp_len%block==0)
		count--;

	//send data cyclically
	while(count>0)
	{			
		
		if(temp_len<=block)
		{
			seg_len = temp_len;
		}			
		else
			seg_len = block;
		 //if the block is the last one, set the vlue of if_last_block true.
		if(count==1)
		{
			if_last_block = true;
		}
		
		ret = apdu_asym_dec (key,asym_param, in_buf+off, seg_len, out_buf, out_buf_len,if_last_block);
		if(ret!=SE_SUCCESS)
		{	
			LOGE("failed to call api_asym_decrypt!\n");
			return ret;
		}
		off+=seg_len;
		count--;
		temp_len-=seg_len;
	}

	
	return SE_SUCCESS;

}

/**
* @brief Asymmetric signature
* @param [in] key->alg  			ALG_RSA1024_CRT/ALG_RSA2048_CRT/ALG_ECC256/ALG_SM2
* @param [in] key->id   			private kid
* @param [in] asym_param->hash_type ALG_SHA1/ALG_SHA256/ALG_SHA384/ALG_SHA512/ALG_SM3/ALG_NONE
* @param [in] in_buf  				input data 
* @param [in] in_buf_len  			input data length
* @param [out] sign_buf  			signature 
* @param [out] sign_buf_len 		signature length
* @return refer error.h
* @note 1.If the input data is not hash data, the has type needs to be assigned. 2.If the input data is hash data, and the hash type is ALG_NONE, caculating the RSA signature needs to pad the hash data by PKCS1.      
* @see apdu_asym_sign
*/
se_error_t api_asym_sign (pri_key_t *key, alg_asym_param_t *asym_param,const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *sign_buf, uint32_t *sign_buf_len)
{
	se_error_t ret = 0;
	uint32_t block = DBLOCK+16;
	uint32_t count = 0;
	uint32_t temp_len = in_buf_len;
	uint32_t seg_len = 0;
	uint32_t off = 0;
	bool if_last_block =false;


	//parameter check
	if(key==NULL||asym_param==NULL||in_buf==NULL||in_buf_len==0)
	{  
		LOGE("failed to api_asym_sign input params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(sign_buf==NULL||sign_buf_len==NULL)
	{  
		LOGE("failed to api_asym_sign output params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	
	if(key->alg!=ALG_ECC256_NIST&&key->alg!=ALG_SM2&&key->alg!=ALG_RSA1024_CRT&&key->alg!=ALG_RSA2048_CRT)
	{  
		LOGE("failed to api_asym_sign alg params!\n");
		return SE_ERR_PARAM_INVALID;
	}


	if(asym_param->hash_type!=ALG_NONE&&asym_param->hash_type!=ALG_SHA1&&asym_param->hash_type!=ALG_SHA256\
	&&asym_param->hash_type!=ALG_SHA384&&asym_param->hash_type!=ALG_SHA512&&asym_param->hash_type!=ALG_SM3&&asym_param->hash_type!=ALG_SHA224)
	{
		LOGE("failed to api_asym_sign hash_type params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if((key->alg==ALG_SM2||key->alg==ALG_ECC256_NIST)&&(asym_param->hash_type==ALG_NONE))
	{
		if(in_buf_len!=32)
		{
			LOGE("failed to api_asym_sign in_buf_len params!\n");
			return SE_ERR_PARAM_INVALID;
		}

	}
	if(key->alg==ALG_RSA1024_CRT&&asym_param->hash_type==ALG_NONE)
		{
			if(in_buf_len!=128)
			{
				LOGE("failed to api_asym_sign in_buf_len params!\n");
				return SE_ERR_PARAM_INVALID;
			}
	
		}
	if(key->alg==ALG_RSA2048_CRT&&asym_param->hash_type==ALG_NONE)
			{
				if(in_buf_len!=256)
				{
					LOGE("failed to api_asym_sign in_buf_len params!\n");
					return SE_ERR_PARAM_INVALID;
				}
		
			}

	//at least 1 block
	count = temp_len/block+1;

	if(temp_len%block==0)
		count--;

	//send data cyclically
	while(count>0)
	{			
		if(temp_len<=block)
		{
			seg_len = temp_len;
		}			
		else
			seg_len = block;
		
		//if the block is the last one, set the vlue of if_last_block true.
		if(count==1)
		{
			if_last_block = true;
		}
		ret = apdu_asym_sign (key,asym_param,in_buf+off, seg_len, sign_buf, sign_buf_len,if_last_block);
		if(ret!=SE_SUCCESS)
		{	
			LOGE("failed to call api_asym_sign!\n");
			return ret;
		}
		off+=seg_len;
		count--;
		temp_len-=seg_len;
	}
	
	return SE_SUCCESS;	

}

/**
* @brief Verify signature 
* @param [in] key->alg  ALG_RSA1024_CRT/ALG_RSA2048_CRT/ALG_ECC256/ALG_SM2
* @param [in] key->id   public kid
* @param [in] asym_param->hash_type  ALG_SHA1/ALG_SHA256/ALG_SHA384/ALG_SHA512/ALG_SM3/ALG_NONE
* @param [in] in_buf   input data
* @param [in] in_buf_len  input data length
* @param [in] sign_buf  signature 
* @param [in] sign_buf_len  signature length
* @return refer error.h
* @note 1.If the input data is not hash data, the has type needs to be assigned. 2.If the input data is hash data, and the hash type is ALG_NONE, verifying the RSA signature needs to pad the hash data by PKCS1. 
* @see api_asym_verify
*/
se_error_t api_asym_verify (pub_key_t *key, alg_asym_param_t *asym_param,const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *sign_buf, uint32_t *sign_buf_len)
{
	se_error_t ret = 0;
	uint32_t block = DBLOCK+16;
	uint32_t count = 0;
	uint32_t temp_len = in_buf_len;
	uint32_t seg_len = 0;
	uint32_t off = 0;
	bool if_last_block =false;
	bool if_first_block =true;


	//parameter check
	if(key==NULL||asym_param==NULL||in_buf==NULL||in_buf_len==0)
	{  
		LOGE("failed to api_asym_sign input params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(sign_buf==NULL||sign_buf_len==NULL)
	{  
		LOGE("failed to api_asym_sign output params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	
	if(key->alg!=ALG_ECC256_NIST&&key->alg!=ALG_SM2&&key->alg!=ALG_RSA1024_CRT&&key->alg!=ALG_RSA2048_CRT)
	{  
		LOGE("failed to api_asym_sign alg params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(asym_param->hash_type!=ALG_NONE&&asym_param->hash_type!=ALG_SHA1&&asym_param->hash_type!=ALG_SHA256\
	&&asym_param->hash_type!=ALG_SHA384&&asym_param->hash_type!=ALG_SHA512&&asym_param->hash_type!=ALG_SM3&&asym_param->hash_type!=ALG_SHA224)
	{
		LOGE("failed to api_asym_sign hash_type params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if((key->alg==ALG_SM2||key->alg==ALG_ECC256_NIST)&&(asym_param->hash_type==ALG_NONE))
	{
		if(in_buf_len!=32)
		{
			LOGE("failed to api_asym_sign in_buf_len params!\n");
			return SE_ERR_PARAM_INVALID;
		}

	}
	if(key->alg==ALG_RSA1024_CRT)
		{
			if(asym_param->hash_type==ALG_NONE&&in_buf_len!=128)
			{
				LOGE("failed to api_asym_sign in_buf_len params!\n");
				return SE_ERR_PARAM_INVALID;
			}
			
	        if(*sign_buf_len!=128)
			{
				LOGE("failed to api_asym_sign in_buf_len params!\n");
				return SE_ERR_PARAM_INVALID;
			}
		}
	if(key->alg==ALG_RSA2048_CRT)
		{
			if(asym_param->hash_type==ALG_NONE&&in_buf_len!=256)
			{
				LOGE("failed to api_asym_sign in_buf_len params!\n");
				return SE_ERR_PARAM_INVALID;
			}
			
	        if(*sign_buf_len!=256)
			{
				LOGE("failed to api_asym_sign in_buf_len params!\n");
				return SE_ERR_PARAM_INVALID;
			}
		}

	//at least 1 block
	count = (temp_len+(*sign_buf_len))/block+1;

	if((temp_len+(*sign_buf_len))%block==0)
		count--;

	//send data cyclically
	while(count>0)
	{			
		if(if_first_block==true)
		{
			if((temp_len+(*sign_buf_len))<=block)
			{
				seg_len = temp_len;
			}			
			else 
			{
				seg_len = block-(*sign_buf_len);
			}

		}
		else
		{
			if(temp_len<=block)
			{
				seg_len = temp_len;
			}			
			else
				seg_len = block;
		}
		
		//if the block is the last one, set the vlue of if_last_block true.
		if(count==1)
		{
			if_last_block = true;
		}
		ret = apdu_asym_verify (key,asym_param, in_buf+off, seg_len, sign_buf, *sign_buf_len,if_first_block,if_last_block);
		if(ret!=SE_SUCCESS)
		{	
			LOGE("failed to call api_asym_sign!\n");
			return ret;
		}
		if_first_block=false;
		off+=seg_len;
		count--;
		temp_len-=seg_len;
	}
	
	return SE_SUCCESS;	

}

/**
* @brief Calculate ZA
* @param [in] uid->val           user id
* @param [in] uid->val_len       user id length
* @param [in] pub_key->val       SM2 public key
* @param [in] pub_key->val_len   SM2 public key length
* @param [out] za                za,32 bytes
* @return refer error.h
* @note no
* @see apdu_sm2_get_za
*/
se_error_t api_sm2_get_za (user_id_t* uid, pub_key_t *pub_key , uint8_t *za )
{
	se_error_t ret = 0;
	//parameter check
	if(uid==NULL||pub_key==NULL||za==NULL)
	{
		LOGE("failed to api_sm2_get_za pointer params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if(uid->val_len==0||uid->val_len>32||pub_key->val_len!=64)
	{
		LOGE("failed to api_sm2_get_za input params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	//call apdu_sm2_get_za
	ret = apdu_sm2_get_za (uid, pub_key , za );
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call apdu_sm2_get_za!\n");
		return ret;
	}

	return SE_SUCCESS;
	
}

/**
* @brief Hash
* @param [in] alg  		  ALG_SHA1/ALG_SHA224/ALG_SHA256/ALG_SHA384/ALG_SHA512/ALG_SM3/ALG_NONE
* @param [in] in_buf  	  input data 
* @param [in] in_buf_len  input data length
* @param [out] out_buf    hash data
* @param [out] out_buf_len  hash data length
* @return  refer error.h
* @note no
* @see apdu_digest
*/
se_error_t api_digest (uint32_t alg, const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len)
{
	se_error_t ret = 0;
	uint32_t block = 254;
	uint32_t count = 0;
	uint32_t temp_len = in_buf_len;
	uint32_t seg_len = 0;
	uint32_t off = 0;
	bool if_last_block =false;

	//parameter check
	if(in_buf==NULL||in_buf_len==0)
	{  
		LOGE("failed to api_digest input params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(out_buf==NULL||out_buf_len==NULL)
	{  
		LOGE("failed to api_digest output params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if(alg!=ALG_SHA1&&alg!=ALG_SHA224&&alg!=ALG_SHA256&&alg!=ALG_SHA384&&alg!=ALG_SHA512&&alg!=ALG_SM3)
	{
		LOGE("failed to api_digest alg params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	

	//at least 1 block
	count = temp_len/block+1;

	if(temp_len%block==0)
		count--;

	//send data cyclically
	while(count>0)
	{			
		
		if(temp_len<=block)
		{
			seg_len = temp_len;
		}			
		else
			seg_len = block;
		//if the block is the last one, set the vlue of if_last_block true.
		if(count==1)
		{
			if_last_block = true;
		}
		ret = apdu_digest (alg,in_buf+off, seg_len, out_buf, out_buf_len,if_last_block);
		if(ret!=SE_SUCCESS)
		{	
			LOGE("failed to call api_asym_encrypt!\n");
			return ret;
		}
		off+=seg_len;
		count--;
		temp_len-=seg_len;
	}
	
	return SE_SUCCESS;	

}


/**
* @brief Get random
* @param [in] expected_len  random length
* @param [out]random  random data
* @return refer error.h
* @note no
* @see apdu_get_random
*/
se_error_t  api_get_random  (uint32_t expected_len, uint8_t *random)
{
	se_error_t ret = 0;
	//parameter check
	if(random==NULL)
	{
		LOGE("failed to api_get_random pointer params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	
	if(expected_len<4||expected_len>16)
	{
		LOGE("failed to api_get_random input params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	//call apdu_get_random
	ret = apdu_get_random  (expected_len,random);
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to apdu_get_random!\n");
		return ret;
	}


	return SE_SUCCESS;
}
/**
* @brief cascade sym encrypt
* @param [in] key->alg  sym alg type(ALG_AES128/ALG_DES128/ALG_SM4)
* @param [in] key->id  kid
* @param [in] sym_param->mode  sym mode
* @param [in] sym_param->iv  IV 
* @param [in] sym_param->iv_len  IV length
* @param [in] sym_param->padding_type paddin mode(PADDING_NOPADDING/PADDING_PKCS7)
* @param [in] in_buf  plaintext data
* @param [in] in_buf_len   plaintext data length
* @param [in]  if_first_block whether the first block
* @param [in]  if_last_block whether the last block
* @param [out] out_buf  cipher data
* @param [out] out_buf_len  cipher data length
* @return refer error.h
* @note no
* @see api_sym_encrypt_cascade
*/
se_error_t api_sym_encrypt_cascade (sym_key_t *key, alg_sym_param_t *sym_param, const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len, bool if_first_block, bool if_last_block)
{

    se_error_t ret = 0;
	uint32_t block = DBLOCK;
	uint32_t count = 0;
	uint32_t padding_len = in_buf_len;
	uint32_t temp_len = in_buf_len;
	uint32_t seg_len = 0;
	uint32_t off = 0;
	uint32_t out_len = 0;
	uint32_t tmp_out_buf_len = 0;
	bool if_last_block_internal =false;
	
	//parameter check
	if(key==NULL||sym_param==NULL||in_buf==NULL||in_buf_len==0)
	{  
		LOGE("failed to api_sym_encrypt_cascade input params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(((if_first_block != true)&&(if_first_block != false))||((if_last_block != true)&&(if_last_block != false)))
	{
		LOGE("failed to api_sym_encrypt_cascade mode params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	
	if(out_buf==NULL||out_buf_len==NULL)
	{  
		LOGE("failed to api_sym_encrypt_cascade output params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(key->alg!=ALG_AES128&&key->alg!=ALG_DES128&&key->alg!=ALG_SM4)
	{  
		LOGE("failed to api_sym_encrypt_cascade alg params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(sym_param->mode!=SYM_MODE_CBC&&sym_param->mode!=SYM_MODE_ECB)
	{  
		LOGE("failed to api_sym_encrypt_cascade mode params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	
	if(sym_param->mode==SYM_MODE_CBC)
	{
		if(if_first_block == true)
		{
			if(key->alg==ALG_DES128)
			{
				if(sym_param->iv_len!=8)
	            {
					LOGE("failed to api_sym_encrypt_cascade iv params!\n");
					return SE_ERR_PARAM_INVALID;
				}
			}
			else
			{
				if(sym_param->iv_len!=16)
	            {
					LOGE("failed to api_sym_encrypt_cascade iv params!\n");
					return SE_ERR_PARAM_INVALID;
				}
			}	
		}
	}
	
	if(sym_param->padding_type!=PADDING_NOPADDING&&sym_param->padding_type!=PADDING_PKCS7)
	{  
		LOGE("failed to api_sym_encrypt padding params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(sym_param->padding_type == PADDING_PKCS7)
	{
		if(if_last_block != true)
		{
			if(key->alg==ALG_DES128)
			{
				if(in_buf_len%8!=0)
				{
					LOGE("failed to api_sym_encrypt input len params!\n");
					return SE_ERR_PARAM_INVALID;
				}
			}
			else
			{
				if(in_buf_len%16!=0)
				{
					LOGE("failed to api_sym_encrypt input len params!\n");
					return SE_ERR_PARAM_INVALID;
				}
			}
		}
	}



	count = temp_len/block+1;
	if(temp_len%block==0)
		count--;

	//send data cyclically
	while(count>0)
	{			
		
		if(temp_len<=block)
		{
			seg_len = temp_len;
		}			
		else
			seg_len = block;
		
		//just one blocak or last one
		if(count==1)
		{
			if(if_last_block == true)
			{
			//if the block is the last one, set the vlue of if_last_block true.
				if_last_block_internal = true;	
				if(sym_param->padding_type == PADDING_PKCS7)
				{
					out_len =  apdu_sym_padding_length(key->alg,PADDING_PKCS7,seg_len);
				}	
			}

		}
		ret = apdu_sym_enc_dec (key,sym_param,in_buf+off, seg_len, out_buf+tmp_out_buf_len, &out_len,if_first_block,if_last_block_internal,true);
		if(ret!=SE_SUCCESS)
		{	
			LOGE("failed to call apdu_sym_enc_dec!\n");
			return ret;
		}
		if_first_block =false;
		off+=seg_len;
		count--;
		temp_len-=seg_len;
		tmp_out_buf_len = tmp_out_buf_len + out_len;
	}
	
	*out_buf_len = tmp_out_buf_len;
	return SE_SUCCESS;
}
/**
* @brief cascade sym decrypt
* @param [in] key->alg  sym alg type(ALG_AES128/ALG_DES128/ALG_SM4)
* @param [in] key->id  kid
* @param [in] sym_param->mode  sym mode
* @param [in] sym_param->iv  IV 
* @param [in] sym_param->iv_len  IV length
* @param [in] sym_param->padding_type paddin mode(PADDING_NOPADDING/PADDING_PKCS7)
* @param [in] in_buf  cipher data
* @param [in] in_buf_len   cipher data length
* @param [in]  if_first_block whether the first block
* @param [in]  if_last_block whether the last block
* @param [out] out_buf plaintext data
* @param [out] out_buf_len  plaintext data length
* @return refer error.h
* @note no
* @see api_sym_encrypt_cascade
*/
se_error_t api_sym_decrypt_cascade (sym_key_t *key, alg_sym_param_t *sym_param, const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len, bool if_first_block, bool if_last_block)
{
    se_error_t ret = 0;
	uint32_t block = DBLOCK;
	uint32_t count = 0;
	uint32_t padding_len = in_buf_len;
	uint32_t temp_len = in_buf_len;
	uint32_t seg_len = 0;
	uint32_t off = 0;
	uint32_t out_len = 0;
	uint32_t tmp_out_buf_len = 0;
	bool if_last_block_internal =false;
	
	//parameter check
	if(key==NULL||sym_param==NULL||in_buf==NULL||in_buf_len==0)
	{  
		LOGE("failed to api_sym_decrypt_cascade input params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(((if_first_block != true)&&(if_first_block != false))||((if_last_block != true)&&(if_last_block != false)))
	{
		LOGE("failed to api_sym_decrypt_cascade mode params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	
	if(out_buf==NULL||out_buf_len==NULL)
	{  
		LOGE("failed to api_sym_decrypt_cascade output params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(key->alg!=ALG_AES128&&key->alg!=ALG_DES128&&key->alg!=ALG_SM4)
	{  
		LOGE("failed to api_sym_decrypt_cascade alg params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(sym_param->mode!=SYM_MODE_CBC&&sym_param->mode!=SYM_MODE_ECB)
	{  
		LOGE("failed to api_sym_decrypt_cascade mode params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	
	if(sym_param->mode==SYM_MODE_CBC)
	{
		if(if_first_block == true)
		{
			if(key->alg==ALG_DES128)
			{
				if(sym_param->iv_len!=8)
	            {
					LOGE("failed to api_sym_decrypt_cascade iv params!\n");
					return SE_ERR_PARAM_INVALID;
				}
			}
			else
			{
				if(sym_param->iv_len!=16)
	            {
					LOGE("failed to api_sym_decrypt_cascade iv params!\n");
					return SE_ERR_PARAM_INVALID;
				}
			}	
		}
	}
	
	if(sym_param->padding_type!=PADDING_NOPADDING&&sym_param->padding_type!=PADDING_PKCS7)
	{  
		LOGE("failed to api_sym_encrypt padding params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(sym_param->padding_type == PADDING_PKCS7)
	{
		if(key->alg==ALG_DES128)
		{
			if(in_buf_len%8!=0)
			{
				LOGE("failed to api_sym_encrypt input len params!\n");
				return SE_ERR_PARAM_INVALID;
			}
		}
		else
		{
			if(in_buf_len%16!=0)
			{
				LOGE("failed to api_sym_encrypt input len params!\n");
				return SE_ERR_PARAM_INVALID;
			}
		}

	}


	count = temp_len/block+1;
	if(temp_len%block==0)
		count--;

	//send data cyclically
	while(count>0)
	{			
		
		if(temp_len<=block)
		{
			seg_len = temp_len;
		}			
		else
			seg_len = block;
		
		//just one blocak or last one
		if(count==1)
		{
			if(if_last_block == true)
			{
				//if the block is the last one, set the vlue of if_last_block true.
				if_last_block_internal = true;				
			}

		}
		ret = apdu_sym_enc_dec (key,sym_param,in_buf+off, seg_len, out_buf+tmp_out_buf_len, &out_len,if_first_block,if_last_block_internal,false);
		if(ret!=SE_SUCCESS)
		{	
			LOGE("failed to call apdu_sym_enc_dec!\n");
			return ret;
		}
		if_first_block =false;
		off+=seg_len;
		count--;
		temp_len-=seg_len;
		tmp_out_buf_len = tmp_out_buf_len + out_len;
	}

	*out_buf_len = tmp_out_buf_len;
	return SE_SUCCESS;

}



/**
* @brief cascade mac calculation
* @param [in] key->alg  alg type(ALG_AES128/ALG_DES128/ALG_SM4)
* @param [in] key->id  kid
* @param [in] sym_param->iv  IV value
* @param [in] sym_param->iv_len  IV length
* @param [in] sym_param->padding_type padding mode(PADDING_NOPADDING/PADDING_ISO9797_M1/PADDING_ISO9797_M2)
* @param [in] in_buf input data 
* @param [in] in_buf_len  input data length
* @param [in]  if_first_block whether the first block
* @param [in]  if_last_block whether the last block
* @param [out] mac  MAC
* @param [out] mac_len  MAC length
* @return refer error.h
* @note 
* @see apdu_mac
*/
se_error_t api_mac_cascade (sym_key_t *key,  alg_sym_param_t *sym_param, const uint8_t *in_buf,uint32_t in_buf_len, uint8_t *mac, uint32_t *mac_len, bool if_first_block, bool if_last_block)
{
	se_error_t ret = 0;
	uint32_t block = DBLOCK;
	uint32_t count = 0;
	uint32_t temp_len = in_buf_len;
	uint32_t seg_len = 0;
	uint32_t off = 0;
	//uint32_t iv_len = 0;
	bool if_last_block_internal =false;

//parameter check
	if(key==NULL||sym_param==NULL||in_buf==NULL||in_buf_len==0)
	{  
		LOGE("failed to api_mac input params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(((if_first_block != true)&&(if_first_block != false))||((if_last_block != true)&&(if_last_block != false)))
	{
		LOGE("failed to api_sym_decrypt_cascade mode params!\n");
		return SE_ERR_PARAM_INVALID;
	}
		
	if(mac==NULL||mac_len==NULL)
	{  
		LOGE("failed to api_mac output params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(key->alg!=ALG_AES128&&key->alg!=ALG_DES128&&key->alg!=ALG_SM4)
	{  
		LOGE("failed to api_mac alg params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(key->alg==ALG_DES128)
	{
		if(sym_param->iv_len!=8)
        {
			LOGE("failed to api_mac iv params!\n");
			return SE_ERR_PARAM_INVALID;
		}
	}
	else
	{
		if(sym_param->iv_len!=16)
        {
			LOGE("failed to api_mac iv params!\n");
			return SE_ERR_PARAM_INVALID;
		}
	}

	if(sym_param->padding_type!=PADDING_NOPADDING&&sym_param->padding_type!=PADDING_ISO9797_M1&&sym_param->padding_type!=PADDING_ISO9797_M2)
	{  
		LOGE("failed to api_mac padding params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	

	count = temp_len/block+1;

	if(temp_len%block==0)
		count--;

	//send data cyclically
	while(count>0)
	{			
		
		if(temp_len<=block)
		{
			seg_len = temp_len;
		}			
		else
			seg_len = block;
		//just one blocak or last one
		if(count==1)
		{
			if(if_last_block == true)
			{
				//if the block is the last one, set the vlue of if_last_block true.
				if_last_block_internal = true;				
			}
		}
		
		ret = apdu_mac (key,sym_param,in_buf+off, seg_len, mac, mac_len,if_first_block,if_last_block_internal,true);
		if(ret!=SE_SUCCESS)
		{	
			LOGE("failed to call apdu_mac!\n");
			return ret;
		}
		if_first_block =false;
		off+=seg_len;
		count--;
		temp_len-=seg_len;
	}

	return SE_SUCCESS;	

}


/**
* @brief cascade mac verufy
* @param [in] key->alg  alg type(ALG_AES128/ALG_DES128/ALG_SM4)
* @param [in] key->id  kid
* @param [in] sym_param->iv  IV value
* @param [in] sym_param->iv_len  IV length
* @param [in] sym_param->padding_type padding mode(PADDING_NOPADDING/PADDING_ISO9797_M1/PADDING_ISO9797_M2)
* @param [in] in_buf input data 
* @param [in] in_buf_len  input data length
* @param [in] mac  MAC
* @param [in] mac_len  MAC length
* @param [in]  if_first_block whether the first block
* @param [in]  if_last_block whether the last block
* @return refer error.h
* @note 
* @see apdu_mac
*/
se_error_t  api_mac_verify_cascade (sym_key_t *key, alg_sym_param_t *sym_param,const uint8_t *in_buf,uint32_t in_buf_len,const uint8_t *mac, uint32_t mac_len, bool if_first_block, bool if_last_block )
{
	se_error_t ret = 0;
	uint32_t block = DBLOCK-16;
	int32_t count = 0;
	uint32_t temp_len = in_buf_len;
	uint32_t seg_len = 0;
	uint32_t off = 0;
	//uint32_t iv_len = 0;
	bool if_last_block_internal =false;

	//parameter check
	if(key==NULL||sym_param==NULL||in_buf==NULL||in_buf_len==0)
	{  
		LOGE("failed to api_mac_verify input params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(((if_first_block != true)&&(if_first_block != false))||((if_last_block != true)&&(if_last_block != false)))
	{
		LOGE("failed to api_sym_decrypt_cascade mode params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	
	if((mac==NULL||mac_len==0) && if_last_block)
	{  
		LOGE("failed to api_mac_verify output params!\n");
		return SE_ERR_PARAM_INVALID;
	}


	if(key->alg!=ALG_AES128&&key->alg!=ALG_DES128&&key->alg!=ALG_SM4)
	{  
		LOGE("failed to api_mac_verify alg params!\n");
		return SE_ERR_PARAM_INVALID;
	}


	if(key->alg==ALG_DES128)
	{
		if(sym_param->iv_len!=8||mac_len!=8)
        {
			LOGE("failed to api_mac_verify iv params!\n");
			return SE_ERR_PARAM_INVALID;
		}
	}
	else
	{
		if(sym_param->iv_len!=16||mac_len!=16)
        {
			LOGE("failed to api_mac_verify iv params!\n");
			return SE_ERR_PARAM_INVALID;
		}
	}

	if(sym_param->padding_type!=PADDING_NOPADDING&&sym_param->padding_type!=PADDING_ISO9797_M1&&sym_param->padding_type!=PADDING_ISO9797_M2)
	{  
		LOGE("failed to api_mac_verify padding params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	

	count = temp_len/block+1;

	if(temp_len%block==0)
		count--;

	//send data cyclically
	while(count>0)
	{			
		
		if(temp_len<=block)
		{
			seg_len = temp_len;
		}			
		else
			seg_len = block;
		//just one blocak or last one
		if(count==1)
		{
			if(if_last_block == true)
			{
				//if the block is the last one, set the vlue of if_last_block true.
				if_last_block_internal = true;				
			}

		}
		ret = apdu_mac (key,sym_param,in_buf+off, seg_len, (uint8_t*)mac, &mac_len,if_first_block,if_last_block_internal,false);
		if(ret!=SE_SUCCESS)
		{	
			LOGE("failed to call apdu_mac!\n");
			return ret;
		}
		if_first_block =false;
		off+=seg_len;
		count--;
		if(temp_len!=0)
		{
			temp_len-=seg_len;
		}
	}

	return SE_SUCCESS;	
}


/**
* @brief cascade digest
* @param [in] alg  digest type(ALG_SHA1/ALG_SHA224/ALG_SHA256/ALG_SHA384/ALG_SHA512/ALG_SM3/ALG_NONE)
* @param [in] in_buf  input data 
* @param [in] in_buf_len  input data length
* @param [in]  if_last_block if last one block
* @param [out] out_buf  digest data
* @param [out] out_buf_len  digest length
* @return refer error.h
* @note no
* @see apdu_digest
*/
se_error_t api_digest_cascade (uint32_t alg, const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len ,bool if_last_block)
{
	se_error_t ret = 0;
	uint32_t block = DBLOCK;
	uint32_t count = 0;
	uint32_t temp_len = in_buf_len;
	uint32_t seg_len = 0;
	uint32_t off = 0;
	bool if_last_block_internal =false;

	//parameter check
	if(in_buf==NULL||in_buf_len==0)
	{  
		LOGE("failed to api_digest_cascade input params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(out_buf==NULL||out_buf_len==NULL)
	{  
		LOGE("failed to api_digest_cascade output params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if(alg!=ALG_SHA1&&alg!=ALG_SHA224&&alg!=ALG_SHA256&&alg!=ALG_SHA384&&alg!=ALG_SHA512&&alg!=ALG_SM3)
	{
		LOGE("failed to api_digest_cascade alg params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	

	count = temp_len/block+1;

	if(temp_len%block==0)
		count--;

	//send data cyclically
	while(count>0)
	{			
		
		if(temp_len<=block)
		{
			seg_len = temp_len;
		}			
		else
			seg_len = block;
		//just one block or last one
		if(count==1)
		{
			if(if_last_block == true)
			{
				//if the block is the last one, set the vlue of if_last_block true.
				if_last_block_internal = true;				
			}
		}
		ret = apdu_digest (alg,in_buf+off, seg_len, out_buf, out_buf_len,if_last_block_internal);
		if(ret!=SE_SUCCESS)
		{	
			LOGE("failed to call api_asym_encrypt!\n");
			return ret;
		}
		off+=seg_len;
		count--;
		temp_len-=seg_len;
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
