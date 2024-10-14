/**@file  key_test.h
* @brief  key_test interface declearation	 
* @author liangww
* @date  2021-05-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/



#ifndef _KEY_TEST_H_
#define _KEY_TEST_H_


#include <stdint.h>
#include "key.h"
#include "soft_alg.h"
#include "auth_test.h"
#include "string2byte.h"


/** @addtogroup SE_APP_TEST
  * @{
  */



/** @defgroup KEY_TEST KEY_TEST
  * @brief key_test interface api.
  * @{
  */


/* Exported types ------------------------------------------------------------*/
/** @defgroup KEY_TEST_Exported_Types KEY_TEST Exported Types
  * @{
  */

/**
  * @brief  KEY_Data_MAX definition
  */
#define KEY_DATA_SIZE 528
/**
  * @}
  */
	



/* Exported functions --------------------------------------------------------*/
/** @defgroup KEY_TEST_Exported_Functions KEY_TEST Exported Functions
  * @{
  */
se_error_t key_test (void);

se_error_t update_mkey_test(void);

#ifdef CONNECT_NEED_AUTH
se_error_t update_ekey_test(void);
#endif


se_error_t set_transkey_test(void);

se_error_t generate_keypair_test (void);

se_error_t generate_symkey_test (void);

se_error_t generate_shared_key_test(void);

se_error_t import_key_test(void);

se_error_t get_key_info_test(void);

se_error_t del_key_test(void);

se_error_t export_key_test(void);








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

