/******************************************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd.
 File name: 		all_api_test.c
 Author:			  liangwwliuch 
 Version:			  V1.0	
 Date:			    2021-05-12	
 Description:	  Main program body
 History:		

******************************************************************************/


/***************************************************************************
* Include Header Files
***************************************************************************/


#include "all_api_test.h"
#include "se.h"
#include "key.h"
#include "pin.h"
#include "port_config.h"

void helpmenu(void)
{
  	port_printf("\nHelp menu: all_api_test <option> ...<option>\n");
	  port_printf("option:NULL \n");    
    port_printf("-h                     : Print this help \n");
}


int main(int argc, char * argv[])
{
	uint8_t com_outbuf[300] = {0};
	uint32_t outlen =0;	
	se_error_t ret = 0;
    uint8_t trankey_val[16]= {0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F};
	sym_key_t trankey={0};
	uint8_t pin_buf[16] = {0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0,0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0};
    pin_t pin={0};
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
	
	/*
	
	It needs to be executed before SE normal communication: 
	1.Register(api_register), input:the interface and SE id
	2.Select(api_select),input:the interface and SE id
	3.Connect(api_connect), after steps 1 and 2 
	4.Set the transport key
	5.Close(api_close),close SE,if you don't need

	*/
	
	
	
	 /****************************************Communication demos*****************************************/	
	ret = comm_test();
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to comm_test\n");
		 return ret;
	}	
	
	
  /****************************************SE Register, SE Select, SE Connect and transport key setting*****************************************/	


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
		ret = api_reset(com_outbuf, &outlen);  			 
		if(ret!=SE_SUCCESS)
		{		
			 LOGE("failed to spi api_reset\n");
			 return ret;
		}
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
     ret = api_verify_pin(&pin);//verify the admin pin
	 if(ret!=SE_SUCCESS)
     { 	  
	   	  LOGE("failed to api_verify_pin\n");
	   	  return ret;
     }
	 
	 ret =  api_set_transkey (&trankey);//set the transport key
	 if(ret!=SE_SUCCESS)
	 {		 
		  LOGE("failed to api_set_transkey\n");
		  return ret;
	 }

	
	 /****************************************device authentication demos*****************************************/		
	
	ret = auth_test();
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to auth_test\n");
		 return ret;
	}
	/****************************************v2x demo*****************************************/
	ret = v2x_test();
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to v2x_test\n");
		 return ret;
	}
	/****************************************Get SE info demos*****************************************/		
	ret = info_test();
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to auth_test\n");
		 return ret;
	}
	
	/****************************************Key management demos*****************************************/		
	
	ret = key_test();
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to key_test\n");
		 return ret;
	}
	
	/****************************************Calculation function demos*****************************************/	
  ret = crypto_test();
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to crypto_test\n");
		 return ret;
	}		

	
	
  /****************************************Control function demos*****************************************/	
	ret = ctrl_test();
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to auth_test\n");
		 return ret;
	}	
	
	
  /****************************************File function demos*****************************************/	

	ret = fs_test();
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to fs_test\n");
		 return ret;
	}

  /****************************************pin related option demos*****************************************/	

	ret = pin_test();
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to pin_test\n");
		 return ret;
	}

	
	

  /****************************************SE update demos*****************************************/	
	//ret = update_test ();
	if(ret!=SE_SUCCESS)
	{		
		LOGE("failed to update_test\n");
		 return ret;
	}


  /********************************************Close SE**************************************************/			
  ret = api_disconnect ();
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to api_disconnect\n");
		 return ret;
	}
	
	LOGI("test end\n"); 

}






/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
