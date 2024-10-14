/******************************************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd.
 File name: 		personal.c
 Author:			  liangwwliuch 
 Version:			  V1.0	
 Date:			    2021-05-12	
 Description:	  Main program body
 History:		

******************************************************************************/


/***************************************************************************
* Include Header Files
***************************************************************************/
#include "comm.h"
#include "se.h"
#include "key.h"
#include "pin.h"
#include "port_config.h"
#include "string2byte.h"
#include "soft_alg.h"
#include "crypto.h"
#include "auth.h"
#define SEAdminPIN_SDK   "123456789ABCDEF0123456789ABCDEF0"
#define SEMasterKey "404142434445464748494A4B4C4D4E4F"

/**
* @brief verify admin pin process
* @param no
* @return refer error.h
* @note no
* @see api_verify_pin StringToByte
*/
se_error_t pin_admin_cert() 
{
	pin_t pin;
	pin.owner=ADMIN_PIN;
	uint32_t retCode = 0;

	//verify admin pin 
	StringToByte(SEAdminPIN_SDK, pin.pin_value, 16 * 2);
	//memcpy(pin.pin_value,admin_pin,16);
	pin.pin_len=0x10;
	retCode = api_verify_pin(&pin);
	if ( retCode != SE_SUCCESS)
	{
		LOGE("failed to api_verify_pin!\n");
		return retCode;
	} 
	// printf("api_verify_pin admin  Success!\n");	

	return retCode;
}


/**
* @brief personalization process
* @param no
* @return refer error.h
* @note no
* @see api_transceive pin_admin_cert api_get_random StringToByte ex_sm4_enc_dec
*/
se_error_t personal()
{
	uint32_t retCode = 0;
	uint8_t outbuf[128] = {0};
	uint32_t outlen = 0;
	uint8_t mkey[32] = {0};//mkey
	bool if_cipher = false;
	bool if_trasns_key = false;
	unikey_t ukey = {0};
	unikey_t inkey = {0};
	char s_appkey[1900]={0};
	uint32_t s_appkey_len = 0;
	uint8_t createBF1[47] = {0x80, 0xE0, 0x00, 0x03, 0x29, 0x00, 0x06, 0x10, 0x00, 0x00, 0x00, 0x02, 0x02, 0x20, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F};
	uint8_t createBF2[15] = {0x80, 0xE0, 0x00, 0x03, 0x0A, 0x00, 0x07, 0x10, 0x00, 0x80, 0x80, 0x02, 0x02, 0x01, 0x00};
	uint16_t status_word = 0;
  
	//get random
	retCode = api_get_random(0x10, outbuf);	
	if ( retCode != SE_SUCCESS)
	{
		LOGE("failed to api_get_random!\n");
		return retCode;
	} 

	//encrypt data
	StringToByte(SEMasterKey, mkey, 16 * 2);
	retCode = ex_sm4_enc_dec(outbuf, 0x10, mkey, 0x10, 0, 0, outbuf, &outlen);
	
	//device auth
	retCode = api_ext_auth(outbuf, 0x10);
	if ( retCode != SE_SUCCESS)
	{
		LOGE("failed to api_ext_auth!\n");
		return retCode;
	} 

	//verify admin pin 
	retCode = pin_admin_cert();
	if ( retCode != SE_SUCCESS)
	{
		LOGE("failed to pin_admin_cert!\n");
		return retCode;
	} 

	//create binary file
	retCode = api_transceive(createBF1, 47, outbuf, &outlen);
	if ( retCode != SE_SUCCESS)
	{
		LOGE("failed to createBF1!\n");
		return retCode;
	} 
  else
	{
		status_word=((outbuf[outlen - 2] << 8) & 0xFF00) | (outbuf[outlen - 1] & 0xFF);
		if(status_word!=0x9000)
		{
			LOGE("failed to createBF1!\n");
			return  tpdu_change_error_code(status_word);
		}
	}
  
	retCode = api_transceive(createBF2, 15, outbuf, &outlen);
	if ( retCode != SE_SUCCESS)
	{
		LOGE("failed to createBF1!\n");
		return retCode;
	} 
  else
	{
		status_word=((outbuf[outlen - 2] << 8) & 0xFF00) | (outbuf[outlen - 1] & 0xFF);
		if(status_word!=0x9000)
		{
			LOGE("failed to createBF2!\n");
			return  tpdu_change_error_code(status_word);
		}
	}
  
	//import app key
	//1.import SM4 key	 
	 inkey.alg = ALG_SM4;
	 inkey.id = 0x06;
	 inkey.type = SYM;
	 inkey.val_len = 0x10;
	 ukey.alg = 0x00;
	 ukey.id = 0x00;
     memset(inkey.val,0x00,16);
	 if_cipher = false;
	 if_trasns_key = false;
	 retCode =  api_import_key (&ukey,&inkey, if_cipher, if_trasns_key);//import key
	 if(retCode!=SE_SUCCESS)
	 {		 
		  LOGE("failed to api_import_key\n");
		  return retCode;
	 }

	//2.import DES key 
	 inkey.alg = ALG_DES128;
	 inkey.id = 0x03;
	 inkey.type = SYM;
	 inkey.val_len = 0x10;
	 ukey.alg = 0x00;
	 ukey.id = 0x00;
     memset(inkey.val,0x00,16);
	 if_cipher = false;
	 if_trasns_key = false;
	 retCode =  api_import_key (&ukey,&inkey, if_cipher, if_trasns_key);//import key
	 if(retCode!=SE_SUCCESS)
	 {		 
		  LOGE("failed to api_import_key\n");
		  return retCode;
	 }

	 
	//3.import AES key	 
	 inkey.alg = ALG_AES128;
	 inkey.id = 0x04;
	 inkey.type = SYM;
	 inkey.val_len = 0x10;
	 ukey.alg = 0x00;
	 ukey.id = 0x00;
     memset(inkey.val,0x00,16);
	 if_cipher = false;
	 if_trasns_key = false;
	 retCode =  api_import_key (&ukey,&inkey, if_cipher, if_trasns_key);//import key
	 if(retCode!=SE_SUCCESS)
	 {		 
		  LOGE("failed to api_import_key\n");
		  return retCode;
	 }


     //4.import ECC keypair
	 strcpy(s_appkey,"D17ED15EDFFFFBC6DB023E1F4E4B9A45C590A816F8CAE301633940E483791728C85E5C328BC059F7A30496B846B5932E7E3E1D34AB53B3CBFF1D98954EFDF511160B55FF3B18EEAEF8F0789B5B0912269EBDE19D41079D87B27522F345723EA9");
	 s_appkey_len = 192;
	 StringToByte(s_appkey,inkey.val,s_appkey_len); 
	 inkey.alg = ALG_ECC256_NIST;
	 inkey.id = 0x09;
	 inkey.type = PRI_PUB_PAIR;
	 inkey.val_len = 0x60;
	 ukey.alg = 0x00;
	 ukey.id = 0x00;
	 if_cipher = false;
	 if_trasns_key = false;
	 retCode =  api_import_key (&ukey,&inkey, if_cipher, if_trasns_key);//import key
	 if(retCode!=SE_SUCCESS)
	 {		 
		  LOGE("failed to api_import_key\n");
		  return retCode;
	 }

	  //5.import RSA1024 keypair 
	strcpy(s_appkey,"00010001CE0BA793E1271566F863C81CC2276BE17B3A581CBDE097536BD37EF6C61E30C36D4E0808E47243B9997D6FAEA664B00EEF7873F5F247263E835F1C9DB9EFA9AFFCDE658B3F72966B056D27A09C2F41D8B02A3A5697D2B40BEDA4C8BA750A80FD68521DB1ED441B94DB2A5B4E1BBCD44C64D6D2917B423AA91F0C146788D69055FEE8AC71C6526090A9835EA1C0A4DDA6BC7157A77F9B9D614472C665A9CEC2C257A9D775C2BDA170DE62B9EA6278C3B8A9C9D641DB4A895384747576A9608DD1CEED6FE2C2E3454CD9A984784E4C64B68DAEB7E8E0A6A34EE2B86563A7E1C894E7D5889FCBBA49D5A35367CE735A70C4467DEEA5DBD3D2BA61D1E68492BDA745BE6C96960FEF6E0FE3722DF96BEE3D5ED79B5C3DD6882B93840CB1C5348B2ED6FBA1F741138F91D0BD70C72E1F0DD438592E5C8EAA010E81838B744C86CF886156FC9D5238170E24D6435ABAB03F90FEE4E493CF43D10949BA294605F4A70789014454A0C825D1B00B6E9E6EBD3341C060D9C39ED9A05C2F99BB812A2DFDA7F9456FE77BBD2493BAD9D767C9B36A303AD30CED3703095A909FA0FBBA3FC1B0B6046F05E551D8598F1C690D678D743B985E7029CD6D0E1A3975C7314829C2ED68");
	 s_appkey_len = 904;
	 StringToByte(s_appkey,inkey.val,s_appkey_len); 
	 inkey.alg = ALG_RSA1024_CRT;
	 inkey.id = 0x0B;
	 inkey.type = PRI_PUB_PAIR;
	 inkey.val_len = 0x01C4;
	 ukey.alg = 0x00;
	 ukey.id = 0x00;
	 if_cipher = false;
	 if_trasns_key = false;
	 retCode =  api_import_key (&ukey,&inkey, if_cipher, if_trasns_key);//import key
	 if(retCode!=SE_SUCCESS)
	 {		 
		  LOGE("failed to api_import_key\n");
		  return retCode;
	 }

	 
	 //6.import RSA2048 keypair 
	 strcpy(s_appkey,"00010001B25AE3DFDDF5C244817320AAC543D8DF6546E41ECFFC641EE22B4738315428A1F2E5FAE7EAF419FE36DF948F0AFD042D675E750C1FF712CD80DE7C5861C8D1594A561B0741716824222631F694E318A0400E9ACE270838B3A1EE067C9147C6CD14C8F0419FB12742EB0DE25B7D2F59A7270C31EA04BC9CF93CD6D4B8511A8913DA9BF951F0E29915B53C395658E9BA74A3B529D6C884171E3ADCE5C25C56678C5512C3171C08939CDDDCB71B4382CDC9C32295C4133B236AD1E5DC4E41C6B90CB52C8AC6AD3019E5F0CF867C67C96F1955CAF07921984BAA6EC97B669526CD4D1974D7A20263E5442E770DA9825AB5232E0053895F6DB82DAA5E417ECE67F959ED4F60E9BEC7D190988E92BC8248434EC3AFE678C568E57F4C4D6289BC53B9648990E3D2C3B34C4A47997B619E7054CBCB6DC6EF777D6946E0A0D2528D99A5B4815796CC574268838DC98979CE52D478B05D567DB587905E54708C697DACDED1DAD959D2C56AE3AD69ECA6D3577367C25A5549577E527545B5D894B755B3D46FC066DDC078545BB1567FB5C492974F4C78B07488BB7572D391BA5E834191B4B28DE0945470A095C7645AC3E14DC4836FA89A4BAA7766CC9B4C57E67F71C6C3BFAAC57DDF82A5D4996BD9D8C66DC3E26A759BA38C5191A5E2904BBDE3988CD1AA71768EAE5FB0CA4E8C71CD9D539AD7D3BB6667B6C0C862809956806E9F91C2B7522AFDFF715376B87E5A3F6C8E1FBF4E726B617DC7BCBE5A096D720506F46668ED4901D964719CA4CB8DD52EC3D1594B07310784BAF6ED90E10E4E44CF4AB8197BFF7BF35CF35D84CF7F4CDEA416020397ED79992555BF232A519E0C98BB569B8B0F5F0E9FD496E8E098545B31188080C70E68CAA6AE9E7478B67927D1C0E679B83785A8CC4D910189DD7B8F3C002E07FD228E61808322AF59BF84D0CDCE11A2485FB805E5548C343E6CFD51D2A10E6BB196124EA446442F89783C14D83E449C5689034D270D5A328F6624BD50C99616F28653A07D5523EC7AD65A78F94E134DCB97856385F182B2949C3E0F9DC60B520A0331D8745B289D12B41502563C3C9F48D0E05C4F49E480C09BDF0AEB712DE24556F2ECA76F1FEA2E33DBAAE63AEFD4E5479C31212F6648A63A81821F13C5F5C88F68FCC37B8D4653EFD8A3F2109DE6EBFBEA00CB29557A64C6730CB47FF0CEBBE6738EBB5FD455EE159E4BA41583AF981CAE7BF2A72F36794D705587155D85E75E8D5013A2A2A08316999880CE2A92");
	 s_appkey_len = 1800;
	 StringToByte(s_appkey,inkey.val,s_appkey_len); 
	 inkey.alg = ALG_RSA2048_CRT;
	 inkey.id = 0x0D;
	 inkey.type = PRI_PUB_PAIR;
	 inkey.val_len = 0x0384;
	 ukey.alg = 0x00;
	 ukey.id = 0x00;
	 if_cipher = false;
	 if_trasns_key = false;
	 retCode =  api_import_key (&ukey,&inkey, if_cipher, if_trasns_key);//import key
	 if(retCode!=SE_SUCCESS)
	 {		 
		  LOGE("failed to api_import_key\n");
		  return retCode;
	 }

	//7.import SM2 keypair
	 strcpy(s_appkey,"160E12897DF4EDB61DD812FEB96748FBD3CCF4FFE26AA6F6DB9540AF49C942324A7DAD08BB9A459531694BEB20AA489D6649975E1BFCF8C4741B78B4B223007F81EB26E941BB5AF16DF116495F90695272AE2CD63D6C4AE1678418BE48230029");
	 s_appkey_len = 192;
	 StringToByte(s_appkey,inkey.val,s_appkey_len); 
	 inkey.alg = ALG_SM2;
	 inkey.id = 0x0F;
	 inkey.type = PRI_PUB_PAIR;
	 inkey.val_len = 0x60;
	 ukey.alg = 0x00;
	 ukey.id = 0x00;
	 if_cipher = false;
	 if_trasns_key = false;
	 retCode =  api_import_key (&ukey,&inkey, if_cipher, if_trasns_key);//import key
	 if(retCode!=SE_SUCCESS)
	 {		 
		  LOGE("failed to api_import_key\n");
		  return retCode;
	 }

	 return retCode;
}

int main(int argc, char * argv[])
{
	uint8_t com_outbuf[300] = {0};
	uint32_t outlen =0;	
	se_error_t ret = 0;
	//MCU initialization
	port_mcu_init();
	
	LOGI("personal begin\n"); 
	/*
	
	It needs to be executed before SE normal communication: 
	1.Register(api_register), input:the interface and SE id
	2.Select(api_select),input:the interface and SE id
	3.Connect(api_connect), after steps 1 and 2 
	4.Set the transport key
	5.Close(api_close),close SE,if you don't need

	*/

  /****************************************SE Register, SE Select, SE Connect and transport key setting*****************************************/	


  
	//---- 1. Register SE----
	ret = api_register(PERIPHERAL_SPI, SPI_PERIPHERAL_SE0);
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to spi  api_register\n");
		 return ret;
	}
	
	//---- 2. Select SE  ----
	ret = api_select(PERIPHERAL_SPI, SPI_PERIPHERAL_SE0);
	if(ret!=SE_SUCCESS)
	{		
		 LOGE("failed to  spi  api_select\n");
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

  /********************************************SE personalization process**************************************************/	
  ret =  personal();
  if(ret!=SE_SUCCESS)
  {		
	LOGE("failed to personal\n");
	return ret;
  }

  /********************************************SE disconnect**************************************************/			
  ret = api_disconnect ();
  if(ret!=SE_SUCCESS)
  {		
	LOGE("failed to api_disconnect\n");
	return ret;
  }
	
	LOGI("personal end\n"); 

}

/************************ (C) COPYRIGHT HED *****END OF FILE****/
