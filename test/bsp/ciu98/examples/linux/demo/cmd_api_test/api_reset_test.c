/******************************************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd.
 File name: 		api_reset_test.c
 Author:			  liangww 
 Version:			  V1.0	
 Date:			    2020-06-24	
 Description:	  Main program body
 History:		

******************************************************************************/


/***************************************************************************
* Include Header Files
***************************************************************************/


#include "se.h"
#include "key.h"
#include "comm.h"

void helpmenu(void)
{
  	port_printf("\nHelp menu: api_reset_test <option> ...<option>\n");
	  port_printf("option:NULL \n");    
    port_printf("-h                     : Print this help \n");
}

int main(int argc, char * argv[])
{
  uint8_t com_outbuf[300] = {0};
	uint32_t outlen =0;	
	se_error_t ret = 0;
  uint32_t i = 0;
  uint8_t atr[30] = {0};	
	uint32_t atr_len = 0;
  uint32_t ch;
	//MCU initialization
	port_mcu_init();
   
   LOGI("test begin\n"); 
    
       while ((ch = getopt(argc, argv, "h")) != -1)
       {
           switch (ch) 
        {
              case 'h': 
				      helpmenu();
				      exit(0);
              break;
         }
       }
  /****************************************SE Register, SE Select, SE Connect and transport key setting*****************************************/	
	
	
	
	#if 1
	//---- 1.Register SE----
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
	//---- 1. Register SE----
	ret = api_register(PERIPHERAL_I2C, I2C_PERIPHERAL_SE0);
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to i2c api_register\n");
		 return ret;
	}
	
	//---- 2.  Select SE  ----
	ret = api_select(PERIPHERAL_I2C, I2C_PERIPHERAL_SE0);
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to i2c api_select\n");
		 return ret;
	}
	#endif

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

  //---- 4. SE wait reset, get ATR ----
  ret = api_reset(atr, &atr_len);

	if(ret != SE_SUCCESS)
	{
		LOGE("failed to api_reset\n");
		return ret;
	}
  port_printf("\n\n**************************************\n");
  port_printf("\napi_reset atr:");
	for(i = 0;i<outlen;i++)
	{
		port_printf("%02x",com_outbuf[i]);
	}	
  port_printf("\n\n**************************************\n");
 
  
	//---- 4. close SE ----
	ret = api_disconnect ();
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to api_disconnect\n");
		 return ret;
	}
	
 LOGI("test end\n"); 	
	
	return SE_SUCCESS;
  

	
}






/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
