/*******************************************************************************
 * Module name: CAN_Util.h
 *
 * First written on Mar 9, 2018. by Owais
 *
 * Module Description:
 * Header for CAN_Util module
 *
 *
 *******************************************************************************/
#ifndef CAN_UTIL_H_
#define CAN_UTIL_H_
/*******************************************************************************
 *  Include section
 *******************************************************************************/
#include "stm32f2xx_hal.h"
#include "stdint.h"
#include "stdbool.h"
#if defined(BUILD_CANBUS)
#include "CQ_Gen.h"
/*******************************************************************************
 *  #defines section
 *******************************************************************************/
#define MAX_LISTEN_ADDRESSES    100
#define TIMEOUT_TX_MS           50  /* time out for CAN transmission */
#define CNT_RX_CAN_QUEUE_FRAMES 50

typedef enum
{
	EnBitRate_62_5K,
	EnBitRate_125K,
	EnBitRate_250K,
	EnBitRate_500K,
	EnBitRate_1000K
}EnumCANBitRate;
/*******************************************************************************
 *  global variables
 *******************************************************************************/

/*******************************************************************************
 *  global functions (public)
 *******************************************************************************/
bool              CAN1_Init(EnumCANBitRate enBR, bool bListen, uint32_t address_list[], uint32_t address_count);
HAL_StatusTypeDef CAN1_Write_Msg(CanTxMsgTypeDef* pMsg);
CanRxMsgTypeDef*  CAN1_Read_Msg(EnumCQErrors* cqErrOut);

#endif

#endif /* CAN_UTIL_H_ */
