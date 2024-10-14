/******************************************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd.
 File name: 		api_transceive_test.c
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
#include "string2byte.h"
#include "key.h"
void helpmenu(void)
{
	  port_printf("\nHelp menu: api_transceive_test <option> ...<option>\n");
	  port_printf("option:- \n");
    port_printf("-l <length>      : Apdu length \n");
	  port_printf("-i <apdu>        : Apdu \n");
    port_printf("-h               : Print this help \n");
}

int main(int argc, char * argv[])
{
  uint8_t com_outbuf[300] = {0};
  uint8_t in_buf[500] = {0};
  uint8_t out_buf[500] = {0};
  uint32_t in_buf_len = 0;
  uint32_t out_buf_len = 0;
	uint32_t outlen =0;	
	se_error_t ret = 0;
  uint32_t i = 0;
   uint32_t apdu_len = 0;
  char apdu_input[1000]; 
  uint32_t i_flag=0;
  uint32_t l_flag=0;
  int ch;
  uint8_t trankey_val[16]= {0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F};
	sym_key_t trankey={0};
	uint8_t pin_buf[16] = {0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0,0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0};
  pin_t pin={0};
	//MCU initialization
	port_mcu_init();
   
   LOGI("test begin\n"); 
   
   if (argc < 2)
    {
      printf("\n");
      printf("--------------------------\n");
			helpmenu();
			exit(0);
		}
    
    
       while ((ch = getopt(argc, argv, "l:i:")) != -1)
       {
           switch (ch) 
        {
               case 'i':
                       i_flag = 2;
                       memcpy(apdu_input, optarg, 2*apdu_len );       
                       break;
                case 'l':
                       l_flag = 1;
                       if((strncmp (optarg, "0x", 2 )==0)||(strncmp (optarg, "0X", 2 )==0))
                          sscanf(optarg,"%x",&apdu_len);
                       else
                          sscanf(optarg,"%d",&apdu_len);
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

	 //---- 4. Set transport key----
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

  /****************************************api_transceive*****************************************/	
   if((i_flag == 2)&& (l_flag == 1))
   {
       StringToByte(apdu_input, in_buf, 2*apdu_len);
       in_buf_len =  apdu_len;
       ret = api_transceive(in_buf, in_buf_len, out_buf, &out_buf_len);
       if ( ret != SE_SUCCESS)
    	 {
    		LOGE("failed to api_transceive!\n");
    		return ret;
       }
       port_printf("\n\n**************************************\n");
       port_printf("\napi_transceive output value:\n");
       for(i = 0;i<out_buf_len;i++)
      	{
      		port_printf("%02x",out_buf[i]);
      	}
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
