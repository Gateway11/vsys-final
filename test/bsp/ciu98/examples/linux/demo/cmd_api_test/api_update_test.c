/******************************************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd. 
 File name: 		api_update_test.c
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
#include "update.h"
#include <stdio.h>
#include <stdlib.h>                                               
#include <unistd.h>
#include <stdint.h>
void helpmenu(void)
{
  	port_printf("\nHelp menu: api_update_test <option> ...<option>\n");
	  port_printf("option:- \n");
    port_printf("-i <update data file>       : The update data file \n");    
    port_printf("-h                          : Print this help \n");
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
  uint32_t i_flag=0;
  int ch;
  //uint8_t key_buf[16]={0};
  char *readbuf;
  uint32_t ilen;
  uint32_t retlen;
  uint32_t fd = -1;
  
  const uint8_t* pimage;
  uint32_t image_len = 0; 
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
    
    
       while ((ch = getopt(argc, argv, "i:")) != -1)
       {
           switch (ch) 
        {
            
          
               case 'i':
                       i_flag = 1;
                       fd = open((const char *)optarg, O_RDONLY);
                       if(fd==-1)
                       {
                         port_printf("can not open the file\n");
                         return -1;
                       } 
                         ilen = lseek(fd, 0, SEEK_END);
                         readbuf = (char*)malloc(ilen);
                         lseek(fd, 0, SEEK_SET);       
                         retlen=read(fd, readbuf, ilen);  
                        close(fd);       
              break;          
              case 'h': 
		         	default:  
				      helpmenu();
				      exit(0);
              break;
         }
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
		 LOGE("failed to spi api_select\n");
		 return ret;
	}



	
		ret = api_connect(com_outbuf, &outlen);
		if(ret!=SE_SUCCESS)
		{		
			 LOGE("failed to api_connect\n");
			 return ret;
		}
  /****************************************api_update*****************************************/	
	                                                          
   if(i_flag == 1)
   {
       
      	//loader  data
        StringToByte(readbuf,(uint8_t *)readbuf,retlen);
        //pimage = (uint8_t *)readbuf; 
      	image_len = retlen/2;
      	ret = api_loader_download( (uint8_t *)readbuf, image_len);
      	if(ret!=SE_SUCCESS)
      	{
      		LOGE("Failed to api_update\n");  
      		return ret;
      	}
        port_printf("\n\n**************************************\n");
        port_printf("\napi_update successfully!");
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
