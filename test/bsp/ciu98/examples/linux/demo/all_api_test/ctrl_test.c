/**@file  ctrl_test.c
* @brief  ctrl_test interface declearation	 
* @author liangww
* @date  2021-05-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/

/***************************************************************************
* Include Header Files
***************************************************************************/
#include "ctrl_test.h"

/** @addtogroup SE_APP_TEST
  * @{
  */



/** @defgroup CTRL_TEST CTRL_TEST
  * @brief ctrl_test interface api.
  * @{
  */



/* Exported functions --------------------------------------------------------*/

/** @defgroup CTRL_TEST_Exported_Functions CTRL_TEST Exported Functions
  * @{
  */


/**
* @brief Work mode switch demo
* @param no
* @return refer error.h
* @note no
* @see  api_switch_mode
*/
se_error_t ctrl_test (void)
{
	se_error_t ret = 0;
	work_mode   mode_type = STANDBY ;

	
    /****************************************work mode change test*****************************************/	
   //enter standby mode
	ret = api_switch_mode (mode_type);
	if ( ret != SE_SUCCESS)
	{
		LOGE("failed to api_switch_mode!\n");
		return ret;
	}

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





