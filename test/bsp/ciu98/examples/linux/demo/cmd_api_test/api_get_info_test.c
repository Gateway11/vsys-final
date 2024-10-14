/******************************************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd.
 File name: 		api_get_info_test.c
 Author:			  liangww 
 Version:			  V1.0	
 Date:			    2023-11-02	
 Description:	  Main program body
 History:		

******************************************************************************/


/***************************************************************************
* Include Header Files                                                                           
***************************************************************************/


#include "se.h"
#include "comm.h"
#include "pin.h"
#include "soft_alg.h"
#include "string2byte.h"
#include "key.h"
#include "info.h"
#include "auth.h"
#include <stdio.h>
#include <stdlib.h>                                               
#include <unistd.h>
#include <stdint.h>
void helpmenu(void)
{
  	port_printf("\nHelp menu: api_get_info_test <option> ...<option>\n");
    port_printf("option:- \n");
    port_printf("-u <info type>         : CHIP_ID : 0x00\n");
    port_printf("                       : PRODUCT_INFO : 0x01\n"); 
    port_printf("                       : LOADER_VERSION : 0x02\n");
    port_printf("                       : LOADER_FEK_INFO : 0x03\n");  
    port_printf("                       : LOADER_FVK_INFO : 0x04\n");            
    port_printf("-h                     : Print this help \n");
}

static uint32_t trans_int(const char *aArg)
{
	uint32_t value;
                                                                                                                               
	if (strncmp(aArg, "0x",2) == 0)
		sscanf(aArg,"%x",&value);
	else
		sscanf(aArg,"%d",&value);

	return value;
}

int main(int argc, char * argv[])
{
  uint8_t com_outbuf[300] = {0};
  uint8_t in_buf[512] = {0};
  uint8_t out_buf[500] = {0};
  uint32_t in_buf_len = 0;
  uint32_t out_buf_len = 0;
  uint32_t outlen =0;	
  se_error_t ret = 0;
  uint32_t i = 0;
  uint32_t transkey_len = 0;
  char transkey_input[16]; 
  uint32_t u_flag=0;
  bool if_app_key;
  int ch;
  uint8_t trankey_val[16]= {0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F};
  sym_key_t trankey={0};
  uint8_t pin_buf[16] = {0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0,0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0};
  pin_t pin={0};
  uint8_t random[16]={0};
  //uint8_t key_buf[16]={0};
  uint8_t usage;
  se_info_t info = {0};

	//MCU initialization
	port_mcu_init();
   
   LOGI("test begin\n"); 
   
   if (argc < 2)
    {
      port_printf("\n");
      port_printf("--------------------------\n");
			helpmenu();
			exit(0);
		}
    
    
       while ((ch = getopt(argc, argv, "u:")) != -1)
       {
           switch (ch) 
        {
                case 'u':
                       u_flag = 1;
                       usage = trans_int(optarg); 
                        if((usage!=0x00)&&(usage!=0x01)&&(usage!=0x02)&&(usage!=0x03)&&(usage!=0x04)&&(usage!=0xFF))
                        {
                            port_printf("The usage value must be 0x00 0x01 0x02 0x03 0x04 0xFF\n");
                            exit(0);
                            }
                       break;
              case 'h': 
		         	default:  
				      helpmenu();
				      exit(0);
              break;
         }
       }
  
  /****************************************SE Register, SE Select, SE Connect and transport key setting*****************************************/	 
	//---- 1.Register SE----
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

  /****************************************api_get_info*****************************************/	
	  
   if( u_flag == 1)
   {
    ret = api_get_info (usage, &info) ;
	if ( ret != SE_SUCCESS)
	{
		LOGE("failed to api_get_info!\n");
		return ret;
	}

	port_printf("\n");
    port_printf("\n\n**************************************\n");
    switch(usage)
	{
		case CHIP_ID:
            port_printf("api_get_info CHIP_ID info:\n");
            for(i = 0;i<sizeof(info.chip_id);i++)
            {
                port_printf("%02x",info.chip_id[i]);
            }
			break;
		case PRODUCT_INFO:
            port_printf("api_get_info PRODUCT_INFO info:\n");
            for(i = 0;i<sizeof(info.product_info);i++)
            {
                port_printf("%02x",info.product_info[i]);
            }
			break;
		case LOADER_VERSION:
            port_printf("api_get_info LOADER_VERSION info:\n");
            for(i = 0;i<sizeof(info.loader_version);i++)
            {
                port_printf("%02x",info.loader_version[i]);
            }
			break;
		case LOADER_FEK_INFO:
            port_printf("api_get_info LOADER_FEK_INFO info:\n");
            for(i = 0;i<sizeof(info.loader_FEK_info);i++)
            {
                port_printf("%02x",info.loader_FEK_info[i]);
            }
			break;
		case LOADER_FVK_INFO:
            port_printf("api_get_info LOADER_FVK_INFO info:\n");
            for(i = 0;i<sizeof(info.loader_FVK_info);i++)
            {
                port_printf("%02x",info.loader_FVK_info[i]);
            }
			break;
		case COS_VERSION:
            port_printf("api_get_info COS_VERSION info:\n");
            for(i = 0;i<sizeof(info.cos_version);i++)
            {
                port_printf("%02x",info.cos_version[i]);
            }
			break;
	}
    port_printf("\napi_get_info successfully!");
    port_printf("\n\n**************************************\n");
     }
     else
     {
        port_printf("\n\n**************************************\n");
        port_printf("\n\nthe input parameter is error!\n");
        port_printf("\n\n**************************************\n");
     }
    
     
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


/************************ Copyright(C),CEC Huada Electronic Design Co.,Ltd. *****END OF FILE****/
