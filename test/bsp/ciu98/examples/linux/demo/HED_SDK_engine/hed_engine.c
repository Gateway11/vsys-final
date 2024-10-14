
#include <string.h>
#include <openssl/engine.h>
#include "hed_engine.h"
#include "hed_engine_ec.h"
#include "hed_engine_rsa.h"
#include "se.h"
#include "comm.h"
#include "pin.h"
#include "key.h"
#include "crypto.h"
#include "auth.h"
#include <stdio.h>
#include <stdlib.h>                                               
#include <unistd.h>
#include <stdint.h>

//Globe
hed_ctx_t hed_ctx;

//Local
static const char *engine_id   = "hed_engine";
static const char *engine_name = "CIU98_B hed engine";

static int engine_init(ENGINE *e);
static int engine_finish(ENGINE *e);
static int engine_destroy(ENGINE *e);

static int engine_init(ENGINE *e)
{
 
  static int initialized = 0;
	uint8_t com_outbuf[300] = {0};
	uint32_t outlen =0;	
	se_error_t ret = 0;
	uint32_t i = 0;
	uint8_t atr[30] = {0};	
	uint32_t atr_len = 0;
  uint8_t trankey_val[16]= {0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F};
  sym_key_t trankey={0};
  const uint8_t pin_buf[16] = {0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0,0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0}; 
  pin_t pin={0};
  uint8_t random[16]={0};
	
	do{
		if (initialized) {
			LOGI("Already initialized\n");
			ret = SE_SUCCESS;
			break;
		}
			
		LOGI("Engine 0x%x init\n", (unsigned int) e);    
		hed_ctx.alg = 0;
		hed_ctx.kid = INIT_KID;
    hed_ctx.pubkeyfilename[0] = '\0';
   
    //HED SDK initialization
	  //---- 1. Register SE----
		ret = api_register(PERIPHERAL_SPI, SPI_PERIPHERAL_SE0);
		if(ret!=SE_SUCCESS)
		{		
			 LOGE("failed to spi api_register\n");
			 return ret;
		}
		
		//---- 2. Select SE ----
		ret = api_select(PERIPHERAL_SPI, SPI_PERIPHERAL_SE0);
		if(ret!=SE_SUCCESS)
		{		
			 LOGE("failed to spi api_select\n");
			 return ret;
		}
	
#ifndef CONNECT_NEED_AUTH

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

		 //---- 4. …Ë÷√¥´ ‰√‹‘ø ----
		 memcpy(trankey.val,trankey_val,16);
		 trankey.val_len = 16;
		 trankey.id = 0x02;
		 pin.owner = ADMIN_PIN;
		 pin.pin_len = 0x10;
		 memcpy(pin.pin_value, pin_buf,pin.pin_len);
		 ret = api_verify_pin(&pin);//verify admin pin
		 if(ret!=SE_SUCCESS)
		 {	  
			  LOGE("failed to api_verify_pin\n");
			  return ret;
		 }
		   
		 ret =	api_set_transkey (&trankey);//set transport key
		 if(ret!=SE_SUCCESS)
		 {		 
			  LOGE("failed to api_set_transkey\n");
			  return ret;
		 }

   
		//Init EC
		ret = hedEngine_init_ec(e);
		if (ret != HED_ENGINE_SUCCESS) 
		{
			LOGE("Engine context init failed");
			break;
		}

		//Init RSA
		ret = hedEngine_init_rsa(e);
		if (ret != HED_ENGINE_SUCCESS) 
		{
			LOGE("Engine context init failed");
			break;
		}	

		
		ret = HED_ENGINE_SUCCESS;
		initialized = 1;
  }while(FALSE);
  return ret;
}

static int engine_destroy(ENGINE *e)
{   se_error_t ret = 0;
    LOGI("Engine 0x%x destroy", (unsigned int) e);
  	ret = api_disconnect ();
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to api_disconnect\n");
		 return ret;
	}
	
  return HED_ENGINE_SUCCESS;
}

static int engine_finish(ENGINE *e)
{
  LOGI("Engine 0x%x finish", (unsigned int) e);
  return HED_ENGINE_SUCCESS;
}

static int bind(ENGINE *e, const char *id)
{
	int ret = HED_ENGINE_FAIL;	
	do {
		if (!ENGINE_set_id(e, engine_id)) {
			LOGI("ENGINE_set_id failed\n");
			break;
		}
		if (!ENGINE_set_name(e, engine_name)) {
			LOGI("ENGINE_set_name failed\n");
			break;
		}
		if (!engine_init(e)) {
			LOGI("Hed enigne initialization failed\n");
			break;
		}

		if (!ENGINE_set_load_privkey_function(e, hed_loadKey)) {
			LOGI("ENGINE_set_load_privkey_function failed\n");
			break;
		}
		
		if (!ENGINE_set_finish_function(e, engine_finish)) {
			LOGI("ENGINE_set_finish_function failed\n");
			break;
		}

		if (!ENGINE_set_destroy_function(e, engine_destroy)) {
			LOGI("ENGINE_set_destroy_function failed\n");
			break;
		}
		ret = HED_ENGINE_SUCCESS;
	}while(FALSE);

    return ret;
  }
 
IMPLEMENT_DYNAMIC_BIND_FN(bind)
IMPLEMENT_DYNAMIC_CHECK_FN()
