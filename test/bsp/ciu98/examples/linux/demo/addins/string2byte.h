/**@file  string2byte.h
* @brief  string2byte interface declearation	 
* @author  liuch
* @date  2020-06-02
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/

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

#ifndef  _STRING2BYTE_H_
#define _STRING2BYTE_H_

#include <stdint.h>

void StringToByte(char* source, uint8_t* dest, int sourceLen);

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


#endif
