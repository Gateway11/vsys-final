#include "hed_engine_loadkey.h"

EVP_PKEY *hed_loadKey(
  ENGINE      *e,
  const char  *key_id,
  UI_METHOD   *ui,
  void        *cb_data)
{  
  EVP_PKEY    *key         = NULL;
  uint32_t value;
	char in[4096];
	char *token[6];
  uint8_t kid;
  FILE *fp;
  char *name;
  char *header;
  uint8_t *data;
  uint32_t len;
  uint8_t pubkey[300];  
  uint16_t i;
 

  LOGI("\nhed_loadKey begin**************\n");  
  LOGI("key_id=<%s>\n", key_id);
  LOGI("cb_data=0x<%x>\n", (unsigned int) cb_data);

 
  while (1)
  {
	  
  	strncpy(in, key_id,1024);
  
	if (key_id == NULL)
	{
		LOGI("No input key parameters present. (key_oid:<pubkeyfile>)");
		return (EVP_PKEY *) NULL; // RETURN FAIL
	}
	
	i = 0;
	token[0] = strtok(in, ":");
	
	if (token[0] == NULL)
	{
	  LOGI("\nToo few parameters in key parameters list. (key_oid:<pubkeyfile>)\n");
	  return (EVP_PKEY *) NULL; // RETURN FAIL
	} 
	
	while (token[i] != NULL)
	{
		i++;
		token[i] = strtok(NULL, ":");
	}

	if (i > 2)
	{
	  LOGI("\nToo many parameters in key parameters list. (key_oid:<pubkeyfile>)\n");
	  return (EVP_PKEY *) NULL; // RETURN FAIL
	}
 
		
	if (strncmp(token[0], "0x",2) == 0)
		sscanf(token[0],"%x",&value); //获取kid的整型数据
	else
	{
		LOGI("failed to get key %s\n",token[0]);
    	break;	
	}

	hed_ctx.kid = value;
	LOGI("\nvalue 0x%x\n", value);

	
	if (token[1] != NULL)
	{
		strncpy(hed_ctx.pubkeyfilename, token[1], PUBKEYFILE_SIZE);
	}
	else
	{
		LOGI("\nnot find pubkeyfile\n");
    	break;
	}


	kid = hed_ctx.kid;
   
	LOGI("\nKID : 0x%x\n",kid);
	
	LOGI("filename : %s\n",hed_ctx.pubkeyfilename);
	//打开公钥文件
	fp = fopen((const char *)hed_ctx.pubkeyfilename,"r");
	if (!fp)
	{
		LOGI("failed to open file %s\n",hed_ctx.pubkeyfilename);
		break;
	}
	PEM_read(fp, &name,&header,&data,(long int *)&len);
	hed_ctx.pubkeylen = len;
	for (i=0; i < len ; i++)
	{
	  hed_ctx.pubkey[i] = *(data+i);
	}
	
	LOGI("len: %d",len);
	printf("\n");	
	key = d2i_PUBKEY(NULL,(const unsigned char **)&data,len);
					
  return key; // SUCCESS
  }

  LOGI("\nhed_loadKey end**************\n");  
  return (EVP_PKEY *) NULL; // RETURN FAIL 

}   

