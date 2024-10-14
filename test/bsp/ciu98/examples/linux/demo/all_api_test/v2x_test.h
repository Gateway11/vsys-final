/**@file  v2x_test.h
* @brief  v2x_test interface declearation	 
* @author liuch
* @date  2022-07-26
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/

#ifndef __V2X_TEST_H__
#define __V2X_TEST_H__

#include <stdint.h>
#include "pin.h"
#include "v2x.h"
#include "key.h"
#include "crypto.h"
#include "string2byte.h"

/** @addtogroup SE_APP_TEST
  * @{
  */


/** @defgroup V2X_TEST V2X_TEST
  * @brief v2x_test interface api.
  * @{
  */


/* Exported functions --------------------------------------------------------*/
/** @defgroup V2X_TEST_Exported_Functions V2X_TEST Exported Functions
  * @{
  */

se_error_t v2x_test (void);

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
