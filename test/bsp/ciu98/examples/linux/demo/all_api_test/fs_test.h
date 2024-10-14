/**@file  fs_test.h
* @brief  fs_test interface declearation	 
* @author liangww
* @date  2021-05-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/

#ifndef __FS_TEST_H__
#define __FS_TEST_H__

#include <stdint.h>
#include "fs.h"
#include "soft_alg.h"
#include "auth_test.h"




/** @addtogroup SE_APP_TEST
  * @{
  */


/** @defgroup FS_TEST FS_TEST
  * @brief fs_test interface api.
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup FS_TEST_Exported_Types FS_TEST Exported Types
  * @{
  */

/**
  * @brief  Fs_Data_MAX definition
  */

#define FS_DATA_SIZE 400
/**
  * @}
  */


/* Exported functions --------------------------------------------------------*/
/** @defgroup FS_Exported_Functions FS Exported Functions
  * @{
  */
se_error_t fs_test(void);


se_error_t writefile_test(void);


se_error_t readfile_test(void);


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
