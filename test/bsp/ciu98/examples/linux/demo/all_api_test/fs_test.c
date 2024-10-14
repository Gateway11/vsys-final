/**@file  fs_test.c
* @brief  fs_test interface declearation	 
* @author liangww
* @date  2021-05-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/

/***************************************************************************
* Include Header Files
***************************************************************************/
#include"fs_test.h"
#include"fs.h"
#include "auth_test.h"
#include "pin.h"
/** @addtogroup SE_APP_TEST
  * @{
  */


/** @defgroup FS_TEST FS_TEST
  * @brief fs_test interface api.
  * @{
  */



/* Exported functions --------------------------------------------------------*/

/** @defgroup FS_TEST_Exported_Functions FS_TEST Exported Functions
  * @{
  */


/**
* @brief File read and write demo
* @param no
* @return refer error.h
* @note no
* @see  writefile_test readfile_test
*/
se_error_t fs_test(void)
{
	/*
		the 0007 file readed need admin authentication and write authentication
	*/
	
	
	se_error_t ret = 0;
	ret = writefile_test();
	if ( ret != SE_SUCCESS )
	{
		return ret;
	}
	ret = readfile_test();
	if ( ret != SE_SUCCESS )
	{
		return ret;
	}


	return SE_SUCCESS;
}


/**
* @brief File write demo
* @param no
* @return refer error.h
* @note no
* @see api_select_file api_verify_pin api_write_file
*/
se_error_t writefile_test(void)
{
	se_error_t ret = 0;
	uint32_t fid=0x0000;
	bool if_encrypt;
	uint8_t in_buf[FS_DATA_SIZE]={0};
	uint32_t in_buf_len=32;
	uint32_t offset = 0;
	uint8_t init_input[50] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0x00};
    pin_t pin={0};
    uint32_t i = 0;
	uint32_t in_len = 0;
	uint8_t pin_buf[16] = {0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0,0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0};

	//Set the input data, you can control the input length according to the i value
	while(i<1)
	{
		memcpy(in_buf+i*in_buf_len,init_input,in_buf_len);
		i++;
	}


	//get the admin authentication and write the 0007 file by plain text. 
	fid = 0x0007;
	ret = api_select_file(fid);
	if ( ret != SE_SUCCESS )
	{
		return ret;
	}
    //get the admin authentication
    pin.owner = ADMIN_PIN;
	pin.pin_len = 0x10;
	memcpy(pin.pin_value, pin_buf,pin.pin_len);
    ret =  api_verify_pin(&pin);
    //write by plain text
	if_encrypt=false;
	in_len = 240;
	for(i = 0;i<in_len;i++)
	{
		port_printf("%02x",in_buf[i]);
	}
	port_printf("\n");	
	
	//write the file
    ret =api_write_file (offset,in_buf, in_len,if_encrypt);
	if ( ret != SE_SUCCESS )
	{
		return ret;
	}

	//get the admin authentication
    pin.owner = ADMIN_PIN;
	pin.pin_len = 0x10;
	memcpy(pin.pin_value, pin_buf,pin.pin_len);
    api_verify_pin(&pin);
    //write by cipher text
	if_encrypt=false;
	in_len = 240;
	for(i = 0;i<in_len;i++)
	{
		port_printf("%02x",in_buf[i]);
	}
	port_printf("\n");	
	
	//write the file
    ret =api_write_file (offset,in_buf, in_len,if_encrypt);
	if ( ret != SE_SUCCESS )
	{
		return ret;
	}

	return SE_SUCCESS;
}


/**
* @brief File read demo
* @param no
* @return refer error.h
* @note no
* @see api_select_file api_verify_pin api_read_file
*/
se_error_t readfile_test(void)
{	
	se_error_t ret = 0;
	uint32_t fid=0x0000;
	bool if_encrypt;
	uint8_t out_buf[FS_DATA_SIZE]={0};
	uint32_t out_buf_len=32;
	uint32_t offset = 0;
	uint32_t i = 0;
	pin_t pin={0};
	uint8_t pin_buf[16] = {0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0,0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0};


	//get the admin authentication and read the 0007 file by plain text. 
	fid = 0x0007;
	ret = api_select_file(fid);
	if ( ret != SE_SUCCESS )
	{
		return ret;
	}
	//get the admin authentication
    pin.owner = ADMIN_PIN;
	pin.pin_len = 0x10;
	memcpy(pin.pin_value, pin_buf,pin.pin_len);
    api_verify_pin(&pin);
	if ( ret != SE_SUCCESS )
	{
		return ret;
	}
	offset = 0;
	//read by plain text
	if_encrypt=false;
	out_buf_len = 0x0E;
    ret =api_read_file( offset,out_buf_len,if_encrypt ,out_buf, &out_buf_len);
	if ( ret != SE_SUCCESS )
	{
		return ret;
	}
	for(i = 0;i<out_buf_len;i++)
	{
			port_printf("%02x",out_buf[i]);
	}
	port_printf("\n");

	//get the admin authentication
    pin.owner = ADMIN_PIN;
	pin.pin_len = 0x10;
	memcpy(pin.pin_value, pin_buf,pin.pin_len);
    ret = api_verify_pin(&pin);
	if ( ret != SE_SUCCESS )
	{
		return ret;
	}
	//read by cipher text
	if_encrypt=true;
	out_buf_len = 0x0E;
	ret =api_read_file( offset,out_buf_len,if_encrypt ,out_buf, &out_buf_len);
	if ( ret != SE_SUCCESS )
	{
		return ret;
	}
	for(i = 0;i<out_buf_len;i++)
	{
			port_printf("%02x",out_buf[i]);
	}
	port_printf("\n");	

	return SE_SUCCESS;
}
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

