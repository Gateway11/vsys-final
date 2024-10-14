/**@file  comm_test.c
* @brief  comm_test interface declearation	 
* @author liangww
* @date  2021-05-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/

/***************************************************************************
* Include Header Files
***************************************************************************/
#include "comm_test.h"
#include "comm.h"


/** @addtogroup SE_APP_TEST
  * @{
  */



/** @defgroup COMM_TEST COMM_TEST
  * @brief comm_test interface api.
  * @{
  */


/* Exported functions --------------------------------------------------------*/
/** @defgroup COMM_TEST_Exported_Functions COMM_TEST Exported Functions
  * @{
  */


/**
* @brief Examples of communicating
* @param no
* @return refer error.h
* @note no
* @see api_register  api_select  api_connect  api_transceive  api_reset  api_disconnect
*/
se_error_t comm_test (void)
{

	
	uint8_t com_outbuf[300] = {0};
	uint32_t outlen =0;	
	uint8_t in_buf[5] = {0};
  uint32_t in_buf_len = 0;
	uint8_t out_buf[16] = {0};
  uint32_t out_buf_en = 0;	
	uint8_t atr[30] = {0};	
	uint32_t atr_len = 0;
	uint32_t i = 0;
	se_error_t ret = 0;
	
	
  /****************************************SE Register/Select/Connect*****************************************/	
	
	
	
	#if 1
	//---- 1. Register SE----
	ret = api_register(PERIPHERAL_SPI, SPI_PERIPHERAL_SE0);
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to spi  api_register\n");
		 return ret;
	}
	
	//---- 2. Select SE ----
	ret = api_select(PERIPHERAL_SPI, SPI_PERIPHERAL_SE0);
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to  spi  api_select\n");
		 return ret;
	}

	#else
	//---- 1. Register SE by HED I2C protocol----
	ret = api_register(PERIPHERAL_I2C, I2C_PERIPHERAL_SE0);
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to i2c api_register\n");
		 return ret;
	}
	
	//---- 2. Register SE ----
	ret = api_select(PERIPHERAL_I2C, I2C_PERIPHERAL_SE0);
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to i2c api_select\n");
		 return ret;
	}
	#endif

	
	
#ifndef CONNECT_NEED_AUTH
		//---- 3. connect----
		ret = api_connect(com_outbuf, &outlen);
		if(ret!=SE_SUCCESS)
		{		
			 LOGE("failed to api_connect\n");
			 return ret;
		}
#endif
	
#ifdef CONNECT_NEED_AUTH
		uint8_t e_key_bin[16]={0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F};
		sym_key_t e_key = {0};
		//---- 3.auth connect ----
		e_key.val_len = 0x10;
		e_key.alg = ALG_SM4;
		memcpy(e_key.val, e_key_bin, 0x10); 
		ret = api_connect_auth(&e_key, com_outbuf, &outlen);
		if(ret!=SE_SUCCESS)
		{	   
			LOGE("failed to api_connect_auth\n");
			return ret;
		}
#endif

	port_printf("api_connect atr:\n");
	for(i = 0;i<outlen;i++)
	{
		port_printf("%02x",com_outbuf[i]);
	}
	port_printf("\n");
	
	
  /****************************************SE transmit*****************************************/	
	
		//---- 4. get 8-bytes randoms ----
	in_buf_len = 0x05;
	memcpy(in_buf, "\x00\x84\x00\x00\x08", in_buf_len);
	
	ret = api_transceive(in_buf, in_buf_len, out_buf, &out_buf_en);
	if(ret != SE_SUCCESS)
	{
		return ret;
	}

	port_printf("api_transceive out_buf:\n");
	for(i = 0;i<out_buf_en;i++)
	{
		port_printf("%02x",out_buf[i]);
	}
	port_printf("\n");
	


  /****************************************SE warm reset, receive ATR*****************************************/	
	atr_len = 0;
	
	ret = api_reset(atr, &atr_len);
	if(ret != SE_SUCCESS)
	{
		LOGE("failed to api_reset\n");
		return ret;
	}
	
	
	
	 /****************************************SE close*****************************************/	
	
	ret = api_disconnect ();
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to api_disconnect\n");
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




