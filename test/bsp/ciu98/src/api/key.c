/**@file  key.c
* @brief  key interface definition
* @author  liangww
* @date  2021-04-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/

/***************************************************************************
* Include Header Files
***************************************************************************/

#include "key.h"
#include "sm4.h"
uint8_t trans_key[16] = {0x00};

/** @addtogroup SE_Service
  * @{
  */

/** @addtogroup API 
  * @brief API layer.
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


/**
* @brief Update master key
* @param [in] mkey->value  		 device master key value
* @param [in] mkey->value_len 	 device master key value length
* @param [in] if_encrypt  		 if protect the master key by device master key
* @return refer error.h
* @note need obtain the device authentication before updating the master key
* @see apdu_write_key
*/
se_error_t api_update_mkey (sym_key_t *mkey_new,sym_key_t *mkey, bool if_encrypt)
{
	se_error_t ret = 0;
	
	
	
	if(mkey_new==NULL)
	{  
		LOGE("failed to api_write_key mkey params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if(if_encrypt!=false&&if_encrypt!=true)
	{  
		LOGE("failed to api_update_mkey bool params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if((if_encrypt == true) && (mkey == NULL))
	{
		LOGE("failed to api_update_mkey params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if((if_encrypt == true) && (mkey ->val_len != 16))//just support SM4
	{
		LOGE("failed to api_update_mkey params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	
	
	//call apdu_write_key
	ret = apdu_write_key (mkey_new->val,mkey_new->val_len,mkey, if_encrypt,false,true,false);
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call apdu_write_key!\n");
		return ret;
	}



	return SE_SUCCESS;
}

#ifdef CONNECT_NEED_AUTH
/**
* @brief update the key for external auth
* @param [in] ekey_new->value  external auth key value
* @param [in] ekey_new->value_len  external auth key value length
* @param [in] mkey->value   device master key
* @param [in] mkey->value_len  device master key length
* @param [in] if_encrypt if protect the external auth key by device master key
* @return refer error.h
* @note 1.need obtain the device auth 2.if not use the device master key to protect ,the mkey can be ignored
* @see apdu_write_key
*/
se_error_t api_update_ekey (sym_key_t *ekey,sym_key_t *mkey, bool if_encrypt)
{
	se_error_t ret = 0;
	uint8_t inbuf[19]={0};
	uint32_t inbuf_len = 19;

	
	if(ekey==NULL)
	{  
		LOGE("failed to api_update_ekey ekey params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if(if_encrypt!=false&&if_encrypt!=true)
	{  
		LOGE("failed to api_update_ekey bool params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if((if_encrypt == true) && (mkey == NULL))
	{
		LOGE("failed to api_update_ekey params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if((if_encrypt == true) && (mkey ->val_len != 16))//just support SM4
	{
		LOGE("failed to api_update_ekey params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	
	//add key info
	inbuf[0]=0x03;//key usage
	inbuf[1]=0x01;//kid:0x01 for external auth key
	inbuf[2]=0x40;//algtype:SM4
	memcpy(inbuf+3,ekey->val,ekey->val_len);
    //call apdu_write_key
	ret = apdu_write_key (inbuf,inbuf_len,mkey, if_encrypt,false,false,true);
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call apdu_write_key!\n");
		return ret;
	}

	return SE_SUCCESS;
}
#endif


/**
* @brief Generate asymmetric fixed/temporary public-private key pairs
* @param [in] pub_key->alg  ALG_RSA1024_STANDAND/ALG_RSA1024_CRT/ALG_RSA2048_STANDAND/ALG_RSA2048_CRT/ALG_ECC256_NIST/ALG_SM2
* @param [in] pub_key->id   public key id (fixed public key ID : 00-0xEF, temporary public key ID: 0xF0-0xFE)
* @param [in] pri_key->id   private key id(fixed private key ID : 00-0xEF, temporary private key ID: 0xF0-0xFE)
* @param [out] pub_key->val  public key
* @param [out] pub_key->val_len  public key length
* @return refer error.h
* @note If the key generated is fixed, the wtite permission of security file must be obtained before. 
* @see apdu_generate_keypair
*/

se_error_t  api_generate_keypair (pub_key_t *pub_key, pri_key_t *pri_key)
{
	se_error_t ret = 0;
	//parameters check
	if(pub_key==NULL||pri_key==NULL)
	{  
		LOGE("failed to api_generate_keypair  pub_key/pri_key params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if(pub_key->alg!=ALG_ECC256_NIST&&pub_key->alg!=ALG_SM2&&pub_key->alg!=ALG_RSA1024_CRT&&pub_key->alg!=ALG_RSA2048_CRT&&pub_key->alg!=ALG_RSA1024_STANDAND&&pub_key->alg!=ALG_RSA2048_STANDAND)
	{  
		LOGE("failed to api_generate_keypair id  params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	//call the apdu_generate_keypair
	ret = apdu_generate_key (pub_key,pri_key,NULL);
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call apdu_generate_keypair!\n");
		return ret;
	}
	return SE_SUCCESS;
}



/**
* @brief Initialize the transport key
* @param [in] key->value  transport key
* @param [in] key->value_len  transport key length
* @param [in] key->id  transport key kid
* @return refer error.h
* @note transport key kid is 0x02
* @see apdu_write_key
*/

se_error_t  api_set_transkey (sym_key_t *key)
{
	se_error_t ret = 0;
	uint8_t inbuf[19]={0};
	uint32_t inbuf_len = 19;
	//parameters check
	if(key==NULL)
	{  
		LOGE("failed to api_set_transkey input pointer params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if(key->val==NULL||key->val_len!=16||key->id!=0x02)
		{  
			LOGE("failed to api_set_transkey input params!\n");
			return SE_ERR_PARAM_INVALID;
		}
	
    //the key infomation
	inbuf[0]=0x01;//key usage
	inbuf[1]=key->id;//the transport key kid is 0x02
	inbuf[2]=0x40;//the algorithm type is SM4 
	memcpy(inbuf+3,key->val,key->val_len);
	memcpy(trans_key,key->val,key->val_len );
	//call apdu_write_key
	ret = apdu_write_key (inbuf,inbuf_len,NULL,false,false,false,false);
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call api_set_transkey!\n");
		return ret;
	}
	return SE_SUCCESS;
}



/**
* @brief Generate symmetric key
* @param [in] symkey->alg ALG_AES128/ALG_DES128/ALG_SM4
* @param [in] symkey->id  key id (fixed key ID : 00-0xEF, temporary key ID: 0xF0-0xFE)
* @return refer error.h
* @note If the key generated is fixed, the wtite permission of security file must be obtained before. 
* @see apdu_generate_keypair
*/
se_error_t  api_generate_symkey (sym_key_t * symkey)
{
	se_error_t ret = 0;
	//parameters check
	if(symkey==NULL)
	{  
		LOGE("failed to api_generate_symkey input pointer params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	
	if(symkey->alg!=ALG_AES128&&symkey->alg!=ALG_DES128&symkey->alg!=ALG_SM4)
	{  
		LOGE("failed to api_generate_symkey input params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	//call the apdu_generate_key
	ret = apdu_generate_key (NULL,NULL,symkey);
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call apdu_generate_keypair!\n");
		return ret;
	}
	return SE_SUCCESS;
}


/**
* @brief ECC/SM2 ECDH
* @param [in] shared_key->alg ALG_ECDH_ECC256/ALG_ECDH_SM2/ALG_SM2_SM2
* @param [in] shared_key->id  if the algorithm type is ALG_ECDH_ECC256 or ALG_ECDH_SM2, the input value is private kid. 
* @param [in] in_buf          if the algorithm type is ALG_ECDH_ECC256 or ALG_ECDH_SM2, the in_buf vlue is public kid. if the algorithm type is ALG_SM2_SM2, please refer the "Product Manual" 
* @param [in] in_buf_len      calculated data length
* @param [in] if_return_key   whether to return shared key
* @param [in] if_return_s 	  whether to return sm2_s
* @param [out] shared_key     ECDH key
* @param [out] sm2_s 		  check value 64 bytes
* @return refer error.h
* @note no
* @see apdu_generate_shared_key
*/
se_error_t api_generate_shared_key (uint8_t *in_buf, uint32_t in_buf_len, unikey_t *shared_key, uint8_t *sm2_s, bool if_return_key, bool if_return_s)
{
	se_error_t ret = 0;
	//uint8_t key_info_tmp [2] = {0xF0,0x60};
	uint8_t int_buf_tmp[66] = {0xF0,0x60};
	uint32_t in_buf_len_tmp = 0;
	
	//parameters check
	if(shared_key->alg!=ALG_ECDH_ECC256 &&shared_key->alg!=ALG_ECDH_SM2&&shared_key->alg!=ALG_SM2_SM2)
	{  
		LOGE("failed to api_generate_shared_key params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(in_buf==NULL)
	{
		LOGE("failed to api_generate_shared_key input params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(shared_key->alg==ALG_ECDH_ECC256||shared_key->alg==ALG_ECDH_SM2)
	{
		if(in_buf_len>66)
		{
			LOGE("failed to api_generate_shared_key input len params!\n");
			return SE_ERR_PARAM_INVALID;
		}

	}
	else
	{
		if(in_buf_len<139||in_buf_len>201)
		{
			LOGE("failed to api_generate_shared_key input len params!\n");
			return SE_ERR_PARAM_INVALID;
		}
	}
  
	//call the apdu_generate_shared_key
	if(shared_key->alg==ALG_SM2_SM2)
	{
	  ret = apdu_generate_shared_key (in_buf, in_buf_len,shared_key,sm2_s, if_return_key, if_return_s);
	}
	else
	{
	  memcpy(int_buf_tmp+2, in_buf, in_buf_len);
		in_buf_len_tmp = in_buf_len+2 ;
		ret = apdu_generate_shared_key (int_buf_tmp, in_buf_len_tmp,shared_key,sm2_s, if_return_key, if_return_s);
	}
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call apdu_generate_shared_key!\n");
		return ret;
	}


	return SE_SUCCESS;
}


/**
* @brief Delete the key
* @param [in] id kid
* @return refer error.h
* @note If the key deleted is fixed, the wtite permission of security file must be obtained before. 
* @see apdu_del_key
*/
se_error_t api_del_key(uint8_t id)
{
	se_error_t ret = 0;
	
	//call the apdu_delete_key
	ret = apdu_delete_key(id);
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call api_del_key!\n");
		return ret;
	}

	return SE_SUCCESS;
}



/**
* @brief Import the application key
* @param [in] ukey->alg 	ALG_RSA1024_CRT/ALG_RSA2048_CRT/ALG_ECC256_NIST/ALG_SM2/ALG_AES128/ALG_DES128/ALG_SM4
* @param [in] ukey->id  	application kid(the private kid and symmetric kid are used to decrypt)
* @param [in] inkey->alg 	algorithm type(ALG_RSA1024_CRT/ALG_RSA2048_CRT/ALG_ECC256_NIST/ALG_SM2/ALG_AES128/ALG_DES128/ALG_SM4)
* @param [in] inkey->id   	import key id:0x00-0xFF  
* @param [in] inkey->type 	key type
* @param [in] inkey->val  	key value
* @param [in] inkey->val_len  application key length
* @param [in] if_cipher     whether to encrypt the input key 
* @param [in] if_trasns_key whether to use the transport key to encrypt
* @return refer error.h
* @note 1.If the value of ukey->alg is ALG_RSA_*, the padding type is PKCS1 2.If the value of ukey->alg is symmetric key, two bytes LD need to be add  before the data. The padding type is 9797_M2. The encryption mode is ECB. 3.Please obtain the write permission before importing the fixed key.4.knowing the key format and the key length refers the "user menu".5.If the key imported is plaintext or encrypted by transport key, the ukey is invalid.
* @see apdu_import_key
*/
se_error_t  api_import_key(unikey_t *ukey, unikey_t *inkey ,bool if_cipher,bool if_trasns_key)
{
	se_error_t ret = 0;
	//parameters check
	if(if_cipher==true)
	{ 	
         if(if_trasns_key == false)
         {
			 if(ukey->alg!=ALG_RSA1024_CRT &&ukey->alg!=ALG_RSA2048_CRT\
					  &&ukey->alg!=ALG_ECC256_NIST&&ukey->alg!=ALG_SM2&&ukey->alg!=ALG_AES128&&ukey->alg!=ALG_DES128&&ukey->alg!=ALG_SM4&&ukey->alg!=ALG_AES128&&ukey->alg!=ALG_RSA1024_STANDAND&&ukey->alg!=ALG_RSA2048_STANDAND)
			 {

				 LOGE("failed to api_import_key alg params!\n");
				 return SE_ERR_PARAM_INVALID;
			 }	 
		 }		 
	}
	else
	{
	   if(if_trasns_key == true)
		{
			LOGE("failed to api_import_key id params!\n");
			return SE_ERR_PARAM_INVALID;
		}
	   
	}
	//check the import key parameters
	if(inkey==NULL)
	{
		LOGE("failed to api_import_key inkey params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if(inkey->alg!=ALG_RSA1024_CRT &&inkey->alg!=ALG_RSA2048_CRT\
		 &&inkey->alg!=ALG_ECC256_NIST&&inkey->alg!=ALG_SM2&&inkey->alg!=ALG_AES128&&inkey->alg!=ALG_DES128&&inkey->alg!=ALG_SM4&&inkey->alg!=ALG_RSA1024_STANDAND&&inkey->alg!=ALG_RSA2048_STANDAND&&inkey->alg!=ALG_ECC_ED25519)
	{
		LOGE("failed to api_import_key alg params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if(inkey->val_len<16||inkey->val_len>912)
	{
		LOGE("failed to api_import_key val params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if(inkey->type!=PRI&&inkey->type!=PUB&&inkey->type!=PRI_PUB_PAIR&&inkey->type!=SYM)
	{
		LOGE("failed to api_import_key type params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if((inkey->alg == ALG_ECC_ED25519) && (inkey->type!=PUB))
	{
		LOGE("failed to api_import_key type params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	//call apdu_import_key
	ret = apdu_import_key (ukey,inkey,if_cipher,if_trasns_key);
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call apdu_import_key!\n");
		return ret;
	}


	return SE_SUCCESS;


}


/**
* @brief Export the temporary private key,temporary symmetric key and temporary/fixed public key.
* @param [in] ukey->alg 	ALG_RSA1024_CRT/ALG_RSA2048_CRT/ALG_ECC256_NIST/ALG_SM2/ALG_AES128/ALG_DES128/ALG_SM4
* @param [in] ukey->id  	application kid(the public kid and symmetric kid are used to encrypt)
* @param [in] exkey->alg 	algorithm type(ALG_RSA1024_CRT/ALG_RSA2048_CRT/ALG_ECC256_NIST/ALG_SM2/ALG_AES128/ALG_DES128/ALG_SM4)
* @param [in] exkey->id   	export key id:0x00-0xFF  
* @param [in] exkey->type 	key type
* @param [in] if_cipher  	whether to encrypt the private/symmetric key exported 
* @param [in] if_trasns_key whether to use the transport key to encrypt
* @param [out] exkey->val   export key value
* @param [out] exkey->val_len  export key length
* @return refer error.h
* @note 1.If the value of ukey->alg is ALG_RSA_*, the padding type is PKCS1. 2.If the value of ukey->alg is symmetric key, two bytes LD need to be add  before the data. The padding type is 9797_M2. The encryption mode is ECB. 3.Please obtain the write permission before exporting the temporary private key.4.knowing the key format and the key length refers the "user menu". 5.If the key imported is plaintext or encrypted by transport key, the ukey is invalid.
*/
se_error_t  api_export_key(unikey_t *ukey, unikey_t *exkey, bool if_cipher,bool if_trasns_key)
{
	se_error_t ret = 0;
	//parameters check
	if(if_cipher==true)
	{
		if(if_trasns_key == false)
		{
	          if(ukey->alg!=ALG_RSA1024_CRT &&ukey->alg!=ALG_RSA2048_CRT\
			 &&ukey->alg!=ALG_ECC256_NIST&&ukey->alg!=ALG_SM2&&ukey->alg!=ALG_AES128&&ukey->alg!=ALG_DES128&&ukey->alg!=ALG_SM4)
			{

				LOGE("failed to api_import_key alg params!\n");
				return SE_ERR_PARAM_INVALID;
			}
			if((exkey->type==PRI &&ukey->alg==ALG_RSA1024_CRT)||(exkey->type==PRI &&ukey->alg==ALG_RSA2048_CRT)||(exkey->type==PRI &&ukey->alg==ALG_ECC256_NIST)||(exkey->type==PRI &&ukey->alg==ALG_SM2)||(exkey->type==PRI &&ukey->alg==ALG_RSA1024_STANDAND)||(exkey->type==PRI &&ukey->alg==ALG_RSA2048_STANDAND))
			{

				LOGE("failed to api_export_key alg params!\n");
				return SE_ERR_PARAM_INVALID;
			}
		}
         
		if(exkey->type==PUB)
		{

			LOGE("failed to api_export_key type params!\n");
			return SE_ERR_PARAM_INVALID;
		}

	}
	else
	{
	    if(if_trasns_key == true)
		{
			LOGE("failed to api_import_key id params!\n");
			return SE_ERR_PARAM_INVALID;
		}
	}
	//check the export key parameters
	if(exkey==NULL)
	{
		LOGE("failed to api_import_session_key exkey params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	if(exkey->alg!=ALG_RSA1024_CRT &&exkey->alg!=ALG_RSA2048_CRT\
		 &&exkey->alg!=ALG_ECC256_NIST&&exkey->alg!=ALG_SM2&&exkey->alg!=ALG_AES128&&exkey->alg!=ALG_DES128&&exkey->alg!=ALG_SM4&&exkey->alg!=ALG_RSA1024_STANDAND &&exkey->alg!=ALG_RSA2048_STANDAND)
	{
		LOGE("failed to api_export_key alg params!\n");
		return SE_ERR_PARAM_INVALID;
	}
	
	if(exkey->type!=PRI&&exkey->type!=PUB&&exkey->type!=SYM)
	{
		LOGE("failed to api_export_key alg type!\n");
		return SE_ERR_PARAM_INVALID;
	}

	if(exkey->type==PRI||exkey->type==SYM)//Only export the temporary private key and temporary symmetric key.
	{
	    if(exkey->id<0xF0)
		{
			LOGE("failed to api_export_key alg type!\n");
			return SE_ERR_PARAM_INVALID;
		}
	}
	
	//call the apdu_export_key
	ret = apdu_export_key (ukey,exkey,if_cipher, if_trasns_key);
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call apdu_export_key!\n");
		return ret;
	}

	return SE_SUCCESS;


}

/**
* @brief Get the key information
* @param [in] if_app_key	whther to get the application key information
* @param [out] out_buf  output data
* @param [out] out_buf_len  output data length
* @return refe error.h
* @note no
* @see apdu_get_key_info
*/
se_error_t  api_get_key_info (bool     if_app_key, uint8_t *out_buf,uint32_t *out_buf_len)
{
	se_error_t ret = 0;
	
	//call the apdu_get_key_info
	ret = apdu_get_key_info (if_app_key, out_buf,out_buf_len);
	if(ret!=SE_SUCCESS)
	{	
		LOGE("failed to call apdu_get_key_info!\n");
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
