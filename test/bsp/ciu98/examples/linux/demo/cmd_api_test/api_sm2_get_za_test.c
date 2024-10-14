/******************************************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd. 
 File name: 	  api_sm2_get_za_test.c
 Author:			  liangww 
 Version:			  V1.0	
 Date:			    2021-06-25	
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
#include "crypto.h"
#include "auth.h"
#include <stdio.h>
#include <stdlib.h>                                               
#include <unistd.h>
#include <stdint.h>
void helpmenu(void)
{
  	port_printf("\nHelp menu: api_sm2_get_za <option> ...<option>\n");
	  port_printf("option:- \n");
    port_printf("-l <uid length>     : The uid length \n");
    port_printf("-u <uid data >      : Input uid data  \n");    
    port_printf("-p <sm2 public key> : input sm2 public key\n");     
    port_printf("-h                       : Print this help \n");
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
  uint8_t out_buf[500] = {0};
  uint8_t in_buf[500] = {0};
  uint32_t in_len = 0;
  uint32_t out_buf_len = 0;
  uint32_t outlen =0;	
  se_error_t ret = 0;
  uint32_t i = 0;
  uint32_t l_flag=0;
  uint32_t u_flag=0;
  uint32_t p_flag=0;
  int ch;
  uint8_t trankey_val[16]= {0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F};
  sym_key_t trankey={0};
  const uint8_t pin_buf[16] = {0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0,0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0}; 
  pin_t pin={0};
  uint8_t random[16]={0};
  //uint8_t key_buf[16]={0};
  char *input_file = NULL;
  char file_in_buf[1000] ;
  //uint32_t len;
  pub_key_t pub_key={0};
  char uid_buf_s[100];
  char  pub_key_buf_s[200];
  uint32_t uid_len;
  uint8_t refer_buf[100] = {0};
  user_id_t uid = {0};
  uint8_t za[32]  = {0};
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
    
    
       while ((ch = getopt(argc, argv, "l:u:p:")) != -1)
       {
           switch (ch) 
        {
            
               case 'u':
                       if(l_flag != 1)
                       {
                        port_printf("wrong : need input the length of key before input the uid data!\n");
                        exit(0);
                       }  
                       u_flag = 1; 
                       memcpy(uid_buf_s, optarg, 2*uid_len);
                       break;
                         
               case 'p':
                       p_flag = 1;         
                       memcpy(pub_key_buf_s, optarg, 128);
                       break;
                case 'l':
                       l_flag = 1;
                       uid_len = trans_int(optarg);             
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

	 //---- 4. Set transport key ----
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
  /****************************************api_sm2_get_za*****************************************/	
	
   if( (l_flag == 1)&& (p_flag == 1)&& (u_flag == 1))
   {
      	StringToByte(uid_buf_s,refer_buf,2*uid_len);
      	uid.val_len = uid_len;
      	memcpy(uid.val,refer_buf,uid.val_len);
      	//set SM2 public key
      	StringToByte(pub_key_buf_s,refer_buf,128);
      	pub_key.val_len = 64;
      	memcpy(pub_key.val,refer_buf,pub_key.val_len);
      	
      	ret = api_sm2_get_za (&uid,&pub_key,za);
        if ( ret != SE_SUCCESS)
      	{
      		LOGE("failed to api_sm2_get_za!\n");
      		return ret;
      	}
        port_printf("\n\n**************************************\n");     
        port_printf("uid_len:%d\n",uid.val_len);
        port_printf("uid:");
      	for(i = 0;i<uid.val_len ;i++)
      	{
      			port_printf("%02x",uid.val[i]);
      	}
        port_printf("\nsm2 public key length:%d\n",64);
        port_printf("sm2 public key:\n");
        
      	for(i = 0;i<pub_key.val_len;i++)
      	{
      			port_printf("%02x",pub_key.val[i]);
      	}	
      	port_printf("\napi_sm2_get_za za:\n");
      	for(i = 0;i<32;i++)
      	{
      			port_printf("%02x",za[i]);
      	}	
            	
        port_printf("api_sm2_get_za successfully!");
        port_printf("\n\n**************************************\n");
     }
     else
     {
        port_printf("\n\n**************************************\n");
        port_printf("\n\nthe input parameter is error!\n");
        port_printf("\n\n**************************************\n");
     }
    
     
	//---- 4.close SE ----
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
