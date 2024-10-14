/******************************************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd. 
 File name: 		api_asym_encrypt_test.c
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
  	port_printf("\nHelp menu: api_asym_encrypt_test <option> ...<option>\n");
	  port_printf("option:- \n");
    port_printf("-a <alg>  : alg type: ALG_SM2     : 0x50 \n");        
    port_printf("                      ALG_RSA1024 : 0x31 \n");   
    port_printf("                      ALG_RSA2048 : 0x33 \n");  
    port_printf("                      ALG_ECC256_NIST : 0x70 \n");
	  port_printf("-k <kid>                 : The kid for the asym encrypt \n");
    port_printf("-l <input data length>   : The input data length \n");
    port_printf("-i <input data file>         : The input data file  \n");    
    port_printf("-p <pading type just for rsa> : pading_type: PADDING_NOPADDING : 0x00 \n"); 
    port_printf("                                             PADDING_PKCS1 : 0x04 \n");
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
  uint8_t out_buf[2048] = {0};
  uint8_t in_buf[2048] = {0};
  uint32_t in_len = 0;
  uint32_t out_buf_len = 0;
  uint32_t outlen =0;	
  se_error_t ret = 0;
  uint32_t i = 0;
  uint32_t k_flag=0;
  uint32_t i_flag=0;
  uint32_t l_flag=0;
  uint32_t a_flag=0;
  uint32_t t_flag=0;
   uint32_t m_flag=0;
  uint32_t p_flag=0;
  int ch;
  uint8_t trankey_val[16]= {0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F};
  sym_key_t trankey={0};
  const uint8_t pin_buf[16] = {0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0,0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0}; 
  pin_t pin={0};
  uint8_t random[16]={0};
  //uint8_t key_buf[16]={0};
  uint8_t kid;
  uint8_t alg_type;
  char *input_file = NULL;
  char file_in_buf[4096] ;
  uint32_t len;
  FILE *fp;
  pub_key_t key={0};
  uint8_t pading_type;
  alg_asym_param_t asym_param={0};
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
    
    
       while ((ch = getopt(argc, argv, "a:k:l:i:p:")) != -1)
       {
           switch (ch) 
        {
               case 'a':
                       a_flag = 1;
                       alg_type = trans_int(optarg);             
                       break;
                case 'k':
                       k_flag = 1;
                       kid = trans_int(optarg); 
      				       if(kid>0xFF)
      			        	{
                          port_printf("The key id must between 0x00 ~ 0xFF\n");
      						        exit(0);
      					      }
                       break;
                       
               case 'i':
                       if(l_flag != 1)
                       {
                        port_printf("wrong : need input the length of key before input the key data!\n");
                        exit(0);
                       }  
                       i_flag = 1;
                       input_file = optarg; 
                       fp = fopen((const char *)optarg, "rb");
                       if(!fp)
                       {
                         port_printf("can not open the file\n");
                         return -1;
                       }        
                         fread(file_in_buf, 1,len*2, fp);  
                         fclose(fp);       
                       break;
                case 'l':
                       l_flag = 1;
                       len = trans_int(optarg);             
                       break;
                 case 'p':
                       p_flag = 1;
                       pading_type = trans_int(optarg);             
                       break;
              case 'h': 
		         	default:  
				      helpmenu();
				      exit(0);
              break;
         }
       }
  
  /****************************************SE Register, SE Select, SE Connect and transport key setting******************************************/	

   
	//---- 1. Register SE----
	ret = api_register(PERIPHERAL_SPI, SPI_PERIPHERAL_SE0);
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to spi api_register\n");
		 return ret;
	}
	
	//---- 2. Select SE  ----
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

	 //---- 4. Set transport key  ----
	 memcpy(trankey.val,trankey_val,16);
	 trankey.val_len = 16;
	 trankey.id = 0x02;
     pin.owner = ADMIN_PIN;
	 pin.pin_len = 0x10;
	 memcpy(pin.pin_value, pin_buf,pin.pin_len);
     ret = api_verify_pin(&pin);//verify the admin pin
	 if(ret!=SE_SUCCESS)
     { 	  
	   	  LOGE("failed to api_connect\n");
	   	  return ret;
     }
	   
	 ret =  api_set_transkey (&trankey);//set the transport key
   
	 if(ret!=SE_SUCCESS)
	 {		 
		  LOGE("failed to api_connect\n");
		  return ret;
	 }
  /****************************************api_sym_encrypt*****************************************/	
	
   if((a_flag == 1)&& (i_flag == 1)&& (l_flag == 1)&& (k_flag == 1))
   {
        key.alg = alg_type;
	      key.id = kid;
        StringToByte(file_in_buf,in_buf,2*len);
      	in_len = len;
      	asym_param.padding_type = pading_type;
        
      	ret = api_asym_encrypt (&key, &asym_param, in_buf, in_len, out_buf, &out_buf_len);//asymmetric decryption
        
      	if ( ret != SE_SUCCESS)
      	{
      		LOGE("failed to api_asym_encrypt!\n");
      		return ret;
      	}
        port_printf("\n\n**************************************\n");
      	port_printf("api_asym_encrypt in_buf:\n");
      	for(i = 0;i<in_len;i++)
      	{
      			port_printf("%02x",in_buf[i]);
      	}
      	port_printf("\n");	      
        switch(alg_type)
        {
          
          case ALG_RSA1024_CRT:
              port_printf("the alg is:%s\n","ALG_RSA1024");
              break;
          case ALG_RSA2048_CRT:
              port_printf("the alg is:%s\n","ALG_RSA2048");
              break; 
          case ALG_ECC256_NIST:
              port_printf("the alg is:%s\n","ALG_ECC256_NIST");
              break;
          case ALG_SM2:
              port_printf("the alg is:%s\n","ALG_SM2");
              break;                                                                                                          
        }
        port_printf("\nthe kid is:0x%02x\n",kid);
        if(pading_type == PADDING_NOPADDING)
        {
         port_printf("the pading_type is : %s\n","PADDING_NOPADDING");
        }
        else if(pading_type == PADDING_PKCS1)
        {
         port_printf("the pading_type is : %s\n","PADDING_PKCS1");
        }
        port_printf("\nthe output encypted data length:%d\n",out_buf_len);
        port_printf("the output encypted data :\n");
        for(i = 0;i<out_buf_len;i++)
      	{
      		port_printf("%02x",out_buf[i]);
      	}
        port_printf("\napi_asym_encrypt successfully!");
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






/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
