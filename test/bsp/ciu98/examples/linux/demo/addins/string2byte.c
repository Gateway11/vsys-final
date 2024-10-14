/**@file  string2byte.c
* @brief  string2byte interface definition
* @author liuch
* @date  2020-06-02
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/

/***************************************************************************
* Include Header Files
***************************************************************************/

#include"string2byte.h"
#include "ctype.h"

/** @addtogroup SE_APP_TEST
  * @{
  */


/** @addtogroup ADDINS
  * @{
  */



/** @defgroup STRING2BYTE STRING2BYTE
  * @brief string2byte interface api.
  * @{
  */


/* Exported functions --------------------------------------------------------*/
/** @defgroup STRING2BYTE_Exported_Functions STRING2BYTE Exported Functions
  * @{
  */


/**
* @brief 字符串转字节数组
* @param [in] source  字符串源数据
* @param [in] sourceLen  字符串源数据长度
* @param [out] dest  字节数组
* @return no
* @note no
* @see no
*/
void StringToByte(char* source, uint8_t* dest, int sourceLen)
{
	int i;
	uint8_t highByte, lowByte;

	for (i = 0; i < sourceLen; i += 2)
	{
		highByte = toupper(source[i]);
		lowByte  = toupper(source[i + 1]);

		if (highByte > 0x39)
			highByte -= 0x37;
		else
			highByte -= 0x30;

		if (lowByte > 0x39)
			lowByte -= 0x37;
		else
			lowByte -= 0x30;

		dest[i / 2] = (highByte << 4) | lowByte;
	}
	return ;
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

/**
  * @}
  */
	