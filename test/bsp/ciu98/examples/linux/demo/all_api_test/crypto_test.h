/**@file  crypto_test.h
* @brief  crypto_test interface declearation	 
* @author liangww
* @date  2021-05-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/


#ifndef __CRYPTO_TEST_H__
#define __CRYPTO_TEST_H__


#include <stdint.h>
#include "crypto.h"
#include "soft_alg.h"
#include "string2byte.h"

/** @addtogroup SE_APP_TEST
  * @{
  */


/** @defgroup CRYPTO_TEST CRYPTO_TEST
  * @brief crypto_test interface api.
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup CRYPTO_TEST_Exported_Types CRYPTO_TEST Exported Types
  * @{
  */

/**
  * @brief  Crypto_Data_MAX definition
  */
#define CRYPTO_DATA_SIZE 700
/**
  * @}
  */


/* Exported functions --------------------------------------------------------*/
/** @defgroup CPYPTO_Exported_Functions CPYPTO Exported Functions
  * @{
  */
se_error_t crypto_test(void);


se_error_t sym_test(void);


se_error_t asym_test(void);


se_error_t digest_test(void);


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
