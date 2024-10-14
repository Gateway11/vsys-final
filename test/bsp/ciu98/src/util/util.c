/**@file  util.c
* @brief Provide deque or other common functions
* @author  zhengwd
* @date  2021-5-12
* @version  V1.0
* @copyright  Copyright(C),CEC Huada Electronic Design Co.,Ltd.
*/

/***************************************************************************
* Include Header Files
***************************************************************************/
//#include "port_bcm2711_util.h"
#include "util.h"


/** @addtogroup SE_Driver
  * @{
  */

/** @defgroup UTIL UTIL
  * @brief util module driver.
  * @{
  */


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @defgroup UTIL_Queue_Exported_Functions UTIL_Queue Exported Functions
  * @{
  */



/**
* @brief Deque initialization
* @param [in] d_queue  deque instance address
* @return deque instance address		
* @note After the deque is initialized, the related parameters of the queue are restored to the default values
*/
double_queue util_queue_init(double_queue d_queue)
{
	d_queue ->q_buf_len =0;
	d_queue ->capacity=DEQUE_MAX_SIZE;
	d_queue ->front_node=20;
	d_queue ->rear_node =20;
	memset(d_queue->q_buf,0x00,DEQUE_MAX_SIZE);
	return d_queue;
}


/**
* @brief Increase the data at the head of the deque
* @param [in] in_buf      input data address
* @param [in] in_buf_len  input data length
* @param [in] d_queue     deque instance address
* @return no	
* @note no
* @see  util_queue_front_pop
*/
void util_queue_front_push(uint8_t*in_buf,uint32_t in_buf_len, double_queue d_queue)
{
	uint32_t off = 0;
	
	if(d_queue ->front_node==0)
	{
		d_queue ->front_node = d_queue ->capacity- in_buf_len;
	}
	else
	{
		d_queue ->front_node-= in_buf_len;
	}
	
	off = d_queue ->front_node;
	memcpy(d_queue->q_buf+off, in_buf, in_buf_len); 
	d_queue ->q_buf_len += in_buf_len;
}


/**
* @brief Deque head delete data
* @param [in] in_buf_len     delete data length
* @param [in] d_queue        deque instance address
* @return no	
* @note no
* @see  util_queue_front_push
*/
void util_queue_front_pop(uint32_t in_buf_len ,double_queue d_queue)
{
	uint8_t item[DEQUE_MAX_SIZE] = {0x00};
	uint32_t off = d_queue->front_node;  
	
	if(d_queue ->front_node ==(d_queue ->capacity- in_buf_len))
	{
		memcpy(item,d_queue->q_buf,in_buf_len);
		d_queue ->front_node =0;
		
	}
	else 
	{
		memcpy(item,d_queue->q_buf+off,in_buf_len);
		d_queue ->front_node += in_buf_len;
	}
	
	d_queue ->q_buf_len -= in_buf_len;
}



/**
* @brief Increase data at the end of the deque
* @param [in] in_buf     input data address
* @param [in] in_buf_len input data length
* @param [in] d_queue    deque instance address
* @return no	
* @note no
* @see  util_queue_rear_pop
*/
void util_queue_rear_push(uint8_t*in_buf,uint32_t in_buf_len, double_queue d_queue)
{
	uint32_t off = d_queue->rear_node;
	
	memcpy(d_queue->q_buf+off, in_buf, in_buf_len); 
	if(d_queue ->rear_node== d_queue ->capacity- in_buf_len)
	{	
		d_queue ->rear_node = 0;
	}
	else
	{
		d_queue ->rear_node += in_buf_len;
	}

	d_queue ->q_buf_len += in_buf_len;
}



/**
* @brief Delete data at the end of the deque
* @param [in] in_buf_len    delete data length
* @param [in] d_queue       deque instance address
* @return no	
* @note no
* @see  util_queue_rear_push
*/
void util_queue_rear_pop(uint32_t in_buf_len ,double_queue d_queue)
{
	uint8_t item[DEQUE_MAX_SIZE] = {0x00};
	uint32_t off = d_queue->rear_node;	
	
	if(d_queue ->rear_node == 0)
	{
		off=d_queue ->capacity-in_buf_len;
		memcpy(item,d_queue->q_buf+off,in_buf_len);
		d_queue ->capacity-= in_buf_len;
	}
	else 
	{
		memcpy(item,d_queue->q_buf+off,in_buf_len);
		d_queue ->rear_node -= in_buf_len;
	}

	d_queue ->q_buf_len -= in_buf_len;
}

/**
* @brief Get the length of the data stored in the deque
* @param [in] d_queue deque instance address
* @return The length of the data currently stored in the deque	
* @note no
*/
uint32_t util_queue_size (double_queue d_queue)
{
	if(d_queue ->rear_node> d_queue ->front_node)
	{
		return (d_queue ->rear_node- d_queue ->front_node);
	}
	else
	{
		return (d_queue ->front_node- d_queue ->rear_node);
	}
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




