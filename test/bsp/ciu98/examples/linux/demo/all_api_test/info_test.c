/**@file  info_test.h
* @brief  info_test interface declearation	 
* @author liangww
* @date  2021-05-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/


/***************************************************************************
* Include Header Files
***************************************************************************/
#include "info_test.h"
#include "auth.h"
#include "soft_alg.h"
#include "crypto.h"

/** @addtogroup SE_APP_TEST
  * @{
  */


/** @defgroup INFO_TEST INFO_TEST
  * @brief info_test interface api.
  * @{
  */


/* Exported functions --------------------------------------------------------*/
/** @defgroup INFO_TEST_Exported_Functions INFO_TEST Exported Functions
  * @{
  */


/**
* @brief get SE information and id, version
* @param no
* @return refer error.h
* @note no
* @see  api_get_info api_get_id api_sdk_version_number api_sdk_version_string
*/
se_error_t info_test (void)
{
	info_type type = PRODUCT_INFO;//PRODUCT_INFO  CHIP_ID
	se_info_t info = {0};
	se_error_t ret = 0;
	se_id_t se_id ={0};
	se_id_t se_id_out ={0};
	uint8_t SEID[16] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88}; 
	uint32_t num = 0;
	char str[40] = {0};
	uint32_t i = 0;
	uint8_t random[16]={0};
	uint8_t mkey[16]={0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F};
    uint8_t out_buf[16]={0};
	uint32_t out_buf_len;
	
	 /****************************************Get SE info test*****************************************/
	ret = api_get_info (type, &info) ;
	if ( ret != SE_SUCCESS)
	{
		LOGE("failed to api_get_info!\n");
		return ret;
	}
	port_printf("api_get_info info:\n");
	
	for(i = 0;i<8;i++)
	{
				port_printf("%02x",info.product_info[i]);
	}
	port_printf("\n");
	
  /****************************************write SE ID test*****************************************/
  //obtain the device authentication before writing the SEID 
  //get 16 bytes random
  api_get_random (0x10, random);
  //Use the device key to encrypt the random into authentication data by SM4.
  ex_sm4_enc_dec(random,0x10,mkey,0x10,ECB,ENCRYPT,out_buf,&out_buf_len);
  //Call the api_ext_auth interface 
   ret = api_ext_auth(out_buf,out_buf_len);
    if ( ret != SE_SUCCESS)
	{
		LOGE("failed to auth_test!\n");
		return ret;
	}
   se_id.val_len = 16;
	 memcpy(se_id.val, SEID, 16);
	 ret = api_write_id (&se_id);
	 if ( ret != SE_SUCCESS)
	 {
		 LOGE("failed to api_write_id!\n");	 
		 return ret;
	 }
	 /****************************************Get SE ID test*****************************************/	
	//If SE ID is not loaded, return 0x10006985
	ret = api_get_id (&se_id_out);
	if ( ret != SE_SUCCESS)
	{
		LOGE("failed to api_get_id!\n");	
		return ret;
	}
	port_printf("api_get_id  se_id:\n");
	for(i = 0;i<se_id_out.val_len;i++)
	{
			port_printf("%02x",se_id_out.val[i]);
	}
	port_printf("\n");


	
	 /****************************************Get SDK version test*****************************************/		
	
	ret = api_sdk_version_number(&num);
	if ( ret != SE_SUCCESS)
	{
		LOGE("failed to api_sdk_version_number!\n");
		return ret;
	}
	if(num!=0x01020702)
	{
		 LOGE("failed to SDK_VERSION_NUM!\n");
		return 0xFFFFFFFF;
  }
	ret =  api_sdk_version_string (str);
	if ( ret != SE_SUCCESS)
	{
		LOGE("failed to api_sdk_version_string!\n");
		return ret;
	}
	if(strcmp(str,"CIU98_B V2 Host SDK[V1.2.7b-release]")!=0)
	{
	  LOGE("failed to SDK_VERSION_STRING!\n");
		return 0xFFFFFFFF;
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


