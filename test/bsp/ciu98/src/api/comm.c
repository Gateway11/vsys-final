/**@file  comm.c
* @brief  comm interface declearation	 
* @author  liangww
* @date  2021-04-28
* @version	V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/


/***************************************************************************
* Include Header Files
***************************************************************************/
#include "comm.h"
#include "util.h"
#include "error.h"
#include "log.h"
#include "auth.h"
#include "crypto.h"
#include "sm4.h"


/** @addtogroup SE_Service
  * @{
  */

/** @addtogroup API 
  * @brief API layer.
  * @{
  */

/** @defgroup COMM COMM
  * @brief comm interface api.
  * @{
  */




/**************************************************************************
* Global Variable Declaration
***************************************************************************/
static peripheral_bus_driver *g_drivers[MAX_PERIPHERAL_BUS_DRIVER] = {0};

/* Exported types ------------------------------------------------------------*/
/** @defgroup COMM_Exported_Types comm_driver_holder Exported Types
  * @{
  */

/**
  * @brief  driver_holder Structure definition
  */

struct driver_holder {
    peripheral_bus_driver *driver;
    peripheral *periph;
};

/**
  * @}
  */


#ifdef __MULTITASK
// Multitasking environment
// The TLS of each task is associated with a selected peripheral_driver object
#ifdef __FREERTOS
#endif

#ifdef __LINUX
#endif
#else
// Single task environment
// Associate a selected peripheral_driver object through global variables
static struct driver_holder g_selected_driver = {NULL, NULL};

#define setSelectedDriver(driver) g_selected_driver.driver = driver
#define setSelectedPeriph(periph) g_selected_driver.periph = periph

#define getSelectedDriver() g_selected_driver.driver
#define getSelectedPeriph() g_selected_driver.periph
#endif

#define checkSelectedDriverAndDevice() \
    do {    \
        if (getSelectedDriver() == NULL || getSelectedDriver() == NULL)   \
            return NULL;    \
    }while(0)

/* Exported functions --------------------------------------------------------*/

/** @defgroup COMM_Exported_Functions COMM Exported Functions
  * @{
  */



/**
* @brief Increase device handle
* @param [in] driver	peripheral bus drive handle data
* @param [in] periph    peripheral handle
* @return refer error.h
* @note no
* @see no
*/
static peripheral* add_periph(peripheral_bus_driver *driver, peripheral *periph) {
    int i=0;

    for (; i<MAX_PERIPHERAL_DEVICE; i++) {
        if (driver->periph[i] == periph)
            return periph;
        else
            if (driver->periph[i] == NULL) {
                driver->periph[i] = periph;
                return periph;
            }
    }

    return NULL;
}


/**
* @brief Add peripheral bus driver handle and device handle
* @param [in] driver	peripheral bus drive handle 
* @param [in] periph    peripheral handle
* @return refer error.h
* @note no
* @see  add_periph
*/
static peripheral_bus_driver* add_driver(peripheral_bus_driver *driver, peripheral *periph) {
    int i=0;

    for (; i<MAX_PERIPHERAL_BUS_DRIVER; i++) { 
        if (g_drivers[i] == driver) {
            if (add_periph(driver, periph) != NULL)
                return driver;
            else
                return NULL;
        }
        else {
            if (g_drivers[i] == NULL) {
                g_drivers[i] = driver;
                if (add_periph(driver, periph) != NULL)
                    return driver;
                else
                    return NULL;
            }
        }
    }

    return NULL;
}



/**
* @brief According to the peripheral type, find the peripheral bus driver handle from g_driver
* @param [in] type	 peripheral peripheral type dynamic handle 
* @return  peripheral bus driver handle
* @note no
* @see no
*/

static peripheral_bus_driver* find_driver(peripheral_type type) {
    int i=0;
	
    for (; i<MAX_PERIPHERAL_BUS_DRIVER; i++) { 
        if (g_drivers[i]->type == type)
            return g_drivers[i];
    }

    return NULL;
}



/**
* @brief Find the peripheral handle according to the peripheral bus drive handle and peripheral ID
* @param [in] driver	peripheral bus drive handle
* @param [in] dev_id    peripheral ID
* @return peripheral handle
* @note no
* @see no
*/
static peripheral* find_slave_device(peripheral_bus_driver *driver, uint32_t dev_id) {
    int i=0;
	
    for (; i<MAX_PERIPHERAL_DEVICE; i++) { 
        if (driver->periph[i]->id == dev_id)
            return driver->periph[i];
    }

    return NULL;
}





/**
* @brief Register according to the peripheral bus driver type and peripheral ID
* @param [in] driver	peripheral bus driver handle
* @param [in] periph    peripheral handle
* @return refer error.h
* @note no
* @see add_driver
*/
se_error_t _api_register(peripheral_bus_driver *driver, peripheral *periph) 
{
	if (driver == NULL || periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}
	if(add_driver(driver, periph) == NULL)
	{
		return SE_ERR_HANDLE_INVALID;
	}

	return SE_SUCCESS;
}




/**
* @brief From the multiple registered peripherals, select the peripheral that needs to be operated
* @param [in] type	  peripheral type
* @param [in] dev_id  peripheral ID
* @return refer error.h
* @note no
* @see  find_driver find_slave_device setSelectedDriver setSelectedPeriph
*/
se_error_t api_select(peripheral_type type, uint32_t dev_id) 
{
	peripheral_bus_driver *driver = NULL;
	peripheral *periph = NULL;

	if (g_drivers[0] == NULL || g_drivers[0]->periph[0] == NULL)
	{
		return SE_ERR_PARAM_INVALID; //Î´Ö´ÐÐacl_register ²Ù×÷
	}

	driver = find_driver(type);
	if (driver == NULL)
	{
		return SE_ERR_HANDLE_INVALID; 
	}

	periph = find_slave_device(driver, dev_id);
	if (periph == NULL)
	{
		return SE_ERR_HANDLE_INVALID; 
	}

	setSelectedDriver(driver);
	setSelectedPeriph(periph);

	return SE_SUCCESS;
}



/**
* @brief Connect peripherals and get peripheral ATR
* @param [out] out_buf	    start address of output
* @param [out] out_buf_len  output data length
* @return refer error.h
* @note 1. Check the parameters, load the driver of the selected peripheral
        2. According to the loaded peripheral driver, call the init initialization function and the open function
* @see  getSelectedDriver getSelectedPeriph init open
*/

se_error_t api_connect (uint8_t *out_buf, uint32_t *out_buf_len)
{
	se_error_t ret_code = SE_SUCCESS;
	peripheral_bus_driver *driver = NULL;
	peripheral *periph = NULL;

	do
	{
		if (((out_buf == NULL) && (out_buf_len != NULL))||((out_buf != NULL) && (out_buf_len == NULL)))
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}
	
		//1. Parameter check and load driver handle
		if (g_drivers[0] == NULL || g_drivers[0]->periph[0] == NULL)
		{
			ret_code = SE_ERR_PARAM_INVALID; //the api_register operation was not performed
			break;
		}
		
		if (getSelectedDriver() == NULL || getSelectedPeriph() == NULL) 
		{
			if (g_drivers[1] != NULL || g_drivers[0]->periph[1] != NULL)
			{
				ret_code = SE_ERR_NO_SELECT; //if multiple peripherals are registered, the peripherals must be selected first
				break;
			}
			//when only one peripheral is registered, there is no need to select, and the first registered peripheral is used directly
			driver = g_drivers[0];
			periph = g_drivers[0]->periph[0];
		}
		else
		{
			//the api_select function is executed, and the specified device is selected
			driver = getSelectedDriver();
			periph = getSelectedPeriph();
		}

		//2.initialize the device
		ret_code = driver->init(periph);   
		if(ret_code != SE_SUCCESS)
		{
			break;
		}

		//3.open the device and get ATR
		ret_code = driver->open(periph, out_buf , out_buf_len); 
		if(ret_code != SE_SUCCESS)
		{
			break;
		}
	
	}while(0);
	
	return ret_code;
}

#ifdef CONNECT_NEED_AUTH
/**
* @brief external auth connect
* @param [in] ekey-> alg alg type (SM4)
* @param [in] ekey-> val_len key length
* @param [in] ekey-> val key value
* @param [out] out_buf	output data
* @param [out] out_buf_len  output data length
* @return refer error.h
* @note no
* @see getSelectedDriver getSelectedPeriph init open
*/

se_error_t api_connect_auth (sym_key_t *ekey,  uint8_t *out_buf, uint32_t *out_buf_len)
{
	uint8_t random[16]={0};
	uint8_t cipher_buf[32] = {0};
	se_error_t ret_code = SE_SUCCESS;

	//Parameter check
	if ((ekey->alg == ALG_SM4) && (ekey->val_len != 16))
	{
		LOGE("failed to api_connect_auth input params!\n");
		return SE_ERR_PARAM_INVALID;
	}

	//connect
	ret_code = api_connect(out_buf, out_buf_len);
	
	//obtain random
    ret_code = api_get_random (0x10, random);
	if ( ret_code != SE_SUCCESS)
	{
		LOGE("failed to api_get_random!\n");
		return ret_code;
	}
	
    //encrypt the random by SM4
    switch( ekey ->alg )
    {
		case ALG_SM4:
			sm4_crypt_ecb(ekey->val, SM4_ENCRYPT, 0x10, random, cipher_buf);	
			break;
		default:
			LOGE("failed to support the sym alg!\n");
			ret_code = SE_ERR_PARAM_INVALID;
			return ret_code;
	}		

	//call api_pair_auth
     ret_code = api_pair_auth(cipher_buf,0x10);
    if ( ret_code != SE_SUCCESS)
	{
		LOGE("failed to api_pair_auth!\n");
		return ret_code;
	}


	return ret_code;
}

#endif

/**
* @brief Disconnect peripherals
* @param no
* @return refer error.h
* @note no
* @see getSelectedDriver getSelectedPeriph deinit close
*/
se_error_t api_disconnect (void)
{
	peripheral_bus_driver *driver = NULL;
	peripheral *periph = NULL;

	se_error_t ret_code = SE_SUCCESS;

	do
	{
		if (g_drivers[0] == NULL || g_drivers[0]->periph[0] == NULL)
		{
			ret_code = SE_ERR_HANDLE_INVALID;//the acl_register operation is not performed
			break;  
		}

		//1.load the driver
		if (getSelectedDriver() == NULL || getSelectedPeriph() == NULL) 
		{
			//the acl_select function is not executed, and the first registered device is used
			driver = g_drivers[0];
			periph = g_drivers[0]->periph[0];
		}
		else
		{
			//the acl_select function is executed, and the specified device is selected
			driver = getSelectedDriver();
			periph = getSelectedPeriph();
		}	

		//2.termination of equipment
		ret_code = driver->deinit(periph);   
		if(ret_code != SE_SUCCESS)
		{
			break;
		}

		//3.close the device
		ret_code = driver->close(periph);   
		if(ret_code != SE_SUCCESS)
		{
			break;
		}
		
	}while(0);
	
	return ret_code;
}



/**
* @brief Send commands and receive responses to the slave device in a two-way queue format
* @param [in] in_buf	    start address of the input two-way queue
* @param [in] in_buf_len    the input two-way queue length
* @param [out] out_buf	    start address of the output two-way queue
* @param [out] out_buf_len  the output two-way queue length
* @return refer error.h
* @note no
* @see getSelectedDriver getSelectedPeriph transceive
*/
se_error_t api_transceive_queue(uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len)
{
	se_error_t ret_code = SE_SUCCESS;
	peripheral_bus_driver *driver = NULL;
	peripheral *periph = NULL;

	do
	{
		//check the parameters
		if ((in_buf == NULL)||(in_buf_len == 0U) || (out_buf == NULL)||(out_buf_len==NULL))
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}
		
		if (g_drivers[0] == NULL || g_drivers[0]->periph[0] == NULL)
		{
			ret_code = SE_ERR_HANDLE_INVALID;//the acl_register operation is not performed
			break;  
		}

		//1.load the driver
		if (getSelectedDriver() == NULL || getSelectedPeriph()  == NULL) 
		{
			//the acl_select function is not executed, and the first registered device is used
			driver = g_drivers[0];
			periph = g_drivers[0]->periph[0];
		}
		else
		{
			//the acl_select function is executed, and the specified device is selected
			driver = getSelectedDriver();
			periph = getSelectedPeriph();
		}	

		//---call proto_spi_transceive or proto_i2c_transceive in proto---
		//send commands to the selected device and receive command response data
		ret_code = driver->transceive(periph, in_buf, in_buf_len, out_buf, out_buf_len); 
		if(ret_code != SE_SUCCESS)
		{
			break;
		}

	}while(0);	
	
	return ret_code;
}



/**
* @brief SE command transmission concrete realization
* @param [in] in_buf	     start address of input data
* @param [in] in_buf_len     input data length
* @param [out] out_buf	     start address of output data
* @param [out] out_buf_len   output data length
* @return refer error.h
* @note  no
* @see  util_queue_init util_queue_rear_push api_transceive_queue
*/
se_error_t api_transceive(const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len)
{
	se_error_t ret_code = SE_SUCCESS;
	uint32_t out_len = 0;
	uint16_t front_node = 0;

	double_queue_node queue_in ={0} ;
	double_queue_node queue_out ={0} ;

	do
	{
		//check the parameters
		if ((in_buf == NULL) || (out_buf == NULL)||(out_buf_len==NULL))
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}

		if ((in_buf_len<COMM_DATA_LEN_MIN)||(in_buf_len>COMM_DATA_LEN_MAX))
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}

		//deque initialization
		util_queue_init(&queue_in);
		util_queue_init(&queue_out);

		//input data is stored in deque
		util_queue_rear_push((uint8_t *)in_buf,in_buf_len, &queue_in);

		//send the data in the input two-way queue to the device according to the protocol format
        //and store the response data in the output two-way queue
		ret_code = api_transceive_queue((uint8_t *)&queue_in, util_queue_size(&queue_in), (uint8_t *)&queue_out, &out_len);
		if(ret_code != SE_SUCCESS)
		{
			return ret_code;
		}

		//copy back the output data from the deque
		front_node = queue_out.front_node;
		memcpy(out_buf,&queue_out.q_buf[front_node],queue_out.q_buf_len);
		*out_buf_len = queue_out.q_buf_len;

	}while(0);

	return ret_code;
}



/**
* @brief Reset peripherals and get peripheral ATR
* @param [out] out_buf	    start address of output
* @param [out] out_buf_len  start address of output data length
* @return refer error.h
* @note no
* @see  getSelectedDriver getSelectedPeriph reset
*/
se_error_t api_reset (uint8_t *out_buf, uint32_t *out_buf_len)
{
	se_error_t ret_code = SE_SUCCESS;
	peripheral_bus_driver *driver = NULL;
	peripheral *periph = NULL;

	do
	{
		if (((out_buf == NULL) && (out_buf_len != NULL))||((out_buf != NULL) && (out_buf_len == NULL)))
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}

		//1. parameter check and load driver handle
		if (g_drivers[0] == NULL || g_drivers[0]->periph[0] == NULL)
		{
			ret_code = SE_ERR_PARAM_INVALID; //the acl_register operation is not performed
			break;
		}
		
		if (getSelectedDriver() == NULL || getSelectedPeriph()  == NULL) 
		{
			if (g_drivers[1] != NULL || g_drivers[0]->periph[1] != NULL)
			{
				ret_code = SE_ERR_NO_SELECT; //if multiple peripherals are registered, the peripherals must be selected first
				break;
			}
			//when only one peripheral is registered, there is no need to select, and the first registered peripheral is used directly
			driver = g_drivers[0];
			periph = g_drivers[0]->periph[0];
		}
		else
		{
			//the acl_select function is executed, and the specified peripheral is selected
			driver = getSelectedDriver();
			periph = getSelectedPeriph();
		}


		//2.reset peripherals, get ATR
		ret_code = driver->reset(periph, out_buf , out_buf_len); 
		if(ret_code != SE_SUCCESS)
		{
			break;
		}

	}while(0);

	return ret_code;
}




/**
* @brief Microsecond delay interface
* @param [in] us	input dalay 
* @return refer error.h
* @see getSelectedDriver getSelectedPeriph delay 
*/
se_error_t api_delay (uint32_t us)
{
	se_error_t ret_code = SE_SUCCESS;
	peripheral_bus_driver *driver = NULL;
	peripheral *periph = NULL;

	do
	{
		if (us == 0)
		{
			ret_code = SE_ERR_PARAM_INVALID;
			break;
		}

		//1. parameter check and load driver handle
		if (g_drivers[0] == NULL || g_drivers[0]->periph[0] == NULL)
		{
			ret_code = SE_ERR_PARAM_INVALID; //the acl_register operation is not performed
			break;
		}
		
		if (getSelectedDriver() == NULL || getSelectedPeriph()  == NULL) 
		{
			if (g_drivers[1] != NULL || g_drivers[0]->periph[1] != NULL)
			{
				ret_code = SE_ERR_NO_SELECT; //if multiple peripherals are registered, the peripherals must be selected first
				break;
			}
			//when only one peripheral is registered, there is no need to select, and the first registered peripheral is used directly
			driver = g_drivers[0];
			periph = g_drivers[0]->periph[0];
		}
		else
		{
			//the acl_select function is executed, and the specified peripheral is selected
			driver = getSelectedDriver();
			periph = getSelectedPeriph();
		}


		//2.delay specified microseconds
		ret_code = driver->delay(periph, us); 
		if(ret_code != SE_SUCCESS)
		{
			break;
		}

	}while(0);

	return ret_code;
}


/**
* @brief Send control commands for low-level hard control operations
* @param [in] ctrlcode	   command control code
* @param [in] in_buf       start address of input data
* @param [in] in_buf_len   input data length
* @param [out] out_buf	   start address of output data
* @param [out] out_buf_len output data length
* @return refer error.h
* @note  no
* @see  getSelectedDriver getSelectedPeriph control
*/
se_error_t _api_control(uint32_t ctrlcode, uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf, uint32_t *out_buf_len)
{
	se_error_t retCode = SE_SUCCESS;

	peripheral_bus_driver *driver = NULL;
	peripheral *periph = NULL;

	if (g_drivers[0] == NULL || g_drivers[0]->periph[0] == NULL)
	{
		return SE_ERR_HANDLE_INVALID;  //the acl_register operation is not performed
	}

	if (getSelectedDriver() == NULL || getSelectedPeriph()  == NULL) 
	{
		//the acl_select function is not executed, and the first registered device is used
		driver = g_drivers[0];
		periph = g_drivers[0]->periph[0];
	}
	else
	{
		//the acl_select function is executed, and the specified device is selected
		driver = getSelectedDriver();
		periph = getSelectedPeriph();
	}

	
	//send control commands to the selected device and receive response data for the control commands
	retCode = driver->control(periph, ctrlcode, in_buf, in_buf_len, out_buf, out_buf_len); 

	return retCode;
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
