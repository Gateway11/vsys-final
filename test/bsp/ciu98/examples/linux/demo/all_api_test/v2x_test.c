/**@file  v2x_test.h
* @brief  info_test interface declearation	 
* @author liangww
* @date  2022-07-25
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/


/***************************************************************************
* Include Header Files
***************************************************************************/
#include "v2x_test.h"

/** @addtogroup SE_APP_TEST
  * @{
  */


/** @defgroup V2X_TEST V2X_TEST
  * @brief info_test interface api.
  * @{
  */


/* Exported functions --------------------------------------------------------*/
/** @defgroup V2X_TEST_Exported_Functions V2X_TEST Exported Functions
  * @{
  */


/**
* @brief genrate derive key seed, get derive seed and private key multiply_add transform 
* @param no
* @return refer error.h
* @note no
* @see v2x_gen_key_derive_seed¡¢v2x_get_derive_seed¡¢v2x_private_key_multiply_add
*/
se_error_t v2x_test (void)
{
	se_error_t ret = 0;
	pin_t pin = {0};
	uint8_t admin_pin_buff[16] = {0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0,0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0};
	derive_seed_t gen_seed_out_buf = {0}; 
	derive_seed_t get_seed_out_buf = {0}; 
	char s_appkey[65]={0};
	uint32_t s_appkey_len = 0;
	bool if_cipher = false;
	bool if_trasns_key = false;
	unikey_t ukey = {0};
	uint8_t input_blod_key_id = 0x00;
	uint8_t output_blod_key_id = 0x00;
	enum ecc_curve curve_type;
	key_multiplier_t key_multiplier = {0};
	key_addend_t key_addend = {0};
    pub_key_t pubkey = {0};
	bool if_have_gen_seed = false;
	pri_key_t pri_key={0};
	alg_asym_param_t asym_param={0};
	uint8_t in_buf[64]={0};
	uint32_t in_buf_len = 32;
	uint8_t out_buf[64]={0};
	uint32_t out_buf_len = 32;

	/****************************************genrate derive key seed demo*****************************************/
  /*verify pin to obtain the administrator permission*/		   
	pin.owner = ADMIN_PIN;
	pin.pin_len = 0x10;
	memcpy(pin.pin_value, admin_pin_buff,pin.pin_len);
	ret = api_verify_pin(&pin);//verify pin
	if(ret!=SE_SUCCESS)
	{ 	  
		LOGE("failed to api_verify_pin\n");
		return ret;
	}

  /*Before calling v2x_gen_key_derive_seed api need obtain the administrator permission*/
	if(if_have_gen_seed == false)
	{
		ret = v2x_gen_key_derive_seed(0x00, &gen_seed_out_buf);
		if(ret!=SE_SUCCESS)
		{ 	  
			LOGE("failed to v2x_gen_key_derive_seed\n");
			return ret;
		}
		{
			if_have_gen_seed = true;
		}
	}

	/****************************************get derive seed demo*****************************************/
	if(if_have_gen_seed == true)
	{
		ret = v2x_get_derive_seed(&get_seed_out_buf);	
		if(ret!=SE_SUCCESS)
		{ 	  
			LOGE("failed to v2x_get_derive_seed\n");
			return ret;
		}
	}

	/****************************************private key multiply_add transform demo*****************************************/
	/*import the private key before demonstrating the private key multiply_add transform demo:BFDF0D1D7D993986DEED4DC3DBD969486B8982F7BDDACD7382B84AF3E9A7CBCC£¨k£©*/
	strcpy(s_appkey,"BFDF0D1D7D993986DEED4DC3DBD969486B8982F7BDDACD7382B84AF3E9A7CBCC");
	s_appkey_len = 64;
	StringToByte(s_appkey,pri_key.val,s_appkey_len); 
	pri_key.alg = ALG_SM2;
	pri_key.id = 0x75;
	pri_key.type = PRI;
	pri_key.val_len = 0x20;
//	ukey.alg = NULL;
	//ukey.id = NULL;
	if_cipher = false;
	if_trasns_key = false;
	ret =  api_import_key (&ukey,&pri_key, if_cipher, if_trasns_key);//import the private key : k£¨kid£º0x75£©
	if(ret!=SE_SUCCESS)
	{		 
		LOGE("failed to api_connect\n");
		return ret;
	}

	input_blod_key_id = 0x75;
	output_blod_key_id = 0x76;
	curve_type =  SM2_CURVE;
	strcpy(s_appkey,"0000000000000000000000000000000000000000000000000000000000000001");
	s_appkey_len = 64;
	StringToByte(s_appkey,key_multiplier.val,s_appkey_len);
	key_multiplier.val_len = 32;

	strcpy(s_appkey,"AF1AC0D06FC78B5B6E4C33B17623746DAC0225AF614FEA7C0778B19994F758C5");
	s_appkey_len = 64;
	StringToByte(s_appkey,key_addend.val,s_appkey_len);
	key_addend.val_len = 32;
  
  //if the output key (k') is stored in the fixed key region:0x00 ~ 0xEF, need obtain the security file write permission before calling the v2x_private_key_multiply_add 
  //if the security file write permission is user pin, need call the api_verify_pin to verify the user pin : pin.owner = USER_PIN
  //if the security file write permission is admin pin, need call the api_verify_pin to verify the admin pin : pin.owner = ADMIN_PIN
  //if the security file write permission is none, need not verify any pin
  
	/*verify pin to obtain the security file write permission*/	   
	pin.owner = ADMIN_PIN;
	pin.pin_len = 0x10;
	memcpy(pin.pin_value, admin_pin_buff,pin.pin_len);
	ret = api_verify_pin(&pin);//verify pin
	if(ret!=SE_SUCCESS)
	{ 	  
		LOGE("failed to api_verify_pin\n");
		return ret;
	}

	/*call v2x_private_key_multiply_add*/
	ret = v2x_private_key_multiply_add(input_blod_key_id, output_blod_key_id, curve_type, key_multiplier, key_addend, &pubkey);
	if(ret!=SE_SUCCESS)
	{		 
		LOGE("failed to v2x_private_key_multiply_add\n");
		return ret;
	}	

	/*print the output public key*/
	printf("\nthe output pubkey is:\n");
	for(int i  =0 ;i< 64;i++)
	{
		printf("%02x",pubkey.val[i]);

	}
	printf("\n");

  /*import the public key into se, use the private key (k') to sign some data and verify the signature by the public key to verify the private key and public key are pair */
	pubkey.alg = ALG_SM2;
	pubkey.id = 0x77;
	pubkey.type = PUB;
	pubkey.val_len = pubkey.val_len;
	memcpy(pubkey.val, pubkey.val, pubkey.val_len);
	if_cipher = false;
	if_trasns_key = false;
	ret =  api_import_key (&ukey,&pubkey, if_cipher, if_trasns_key);//import output public key
	if(ret!=SE_SUCCESS)
	{		 
		LOGE("failed to api_connect\n");
		return ret;
	}

	pri_key.alg = ALG_SM2;
	pri_key.id = 0x76;
	asym_param.hash_type = ALG_SM3;
	in_buf_len = 64;
	port_printf("api_asym_sign in_buf:\n");
	for(int i = 0;i<in_buf_len;i++)
	{
			port_printf("%02x",in_buf[i]);
	}
	port_printf("\n");	
	ret = api_asym_sign (&pri_key, &asym_param, in_buf, in_buf_len, out_buf, &out_buf_len);//use the private key (k') to sign
	if ( ret != SE_SUCCESS)
	{
		LOGE("failed to api_asym_sign!\n");
		return ret;
	} 

	pubkey.id = 0x77;
	pubkey.alg = ALG_SM2;
	pubkey.type = PUB;
	ret = api_asym_verify (&pubkey, &asym_param, in_buf, in_buf_len, out_buf, &out_buf_len);
	if ( ret != SE_SUCCESS)
	{
		LOGE("failed to api_asym_verify!\n");
		return ret;
	}	

	/****************************************reconsitution key demo*****************************************/
//1.Host device calls the v2x_gen_key_derive_seed interface to generate the derived seeds. (kS & kE & A & P) are sent to the PRA by EeRaCertRequest.   
//2.After receiving the SCTij, the host device verifies the signature by the public key of PCA certificate.  
//3.If the signature is verified successfully, the host device calls the v2x_reconsitution_key to obtain the private key sij stored in the SE and output the pseudonym certificate. Carefully need generate the derive key and obtain the security file write permission before calling the v2x_reconsitution_key  

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


