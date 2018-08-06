/*****************************************************************************
 * Module name: CAN_Util.c
 *
 * First written on Mar 10, 2018 by Owais.
 *
 * Module Description:
 * CAN high and low level details
 *
 * Updates:
 *****************************************************************************/

/*****************************************************************************
 *  Include section
 *****************************************************************************/
#include "CAN_Util.h"
#if defined(BUILD_CANBUS)
/*****************************************************************************
 *  global variables
 *****************************************************************************/
CAN_HandleTypeDef hcan1;
uint32_t ListenAddresses[MAX_LISTEN_ADDRESSES];
uint32_t CntListenAddresses = 0;
bool bListenEnabled = false; /* Is CAN1 supposed to listen for messages */
bool bListenAll = false;
uint32_t BroadcastAddress = 0; /* If any address in list is 0 means will listen to all messages */
CanRxMsgTypeDef Can1RxMsg1, Can1RxMsg2;
CanTxMsgTypeDef Can1TxMsg;

uint32_t CntRxFilterOK = 0;
uint32_t CntRxFilterFail = 0;

CircularQueue CQ_CAN_Frames;
CanRxMsgTypeDef ArrCanRXMessages[CNT_RX_CAN_QUEUE_FRAMES];

uint32_t CntEnqOK = 0;
uint32_t CntEnqFail = 0;
/*****************************************************************************
 *  Function Prototypes
 *****************************************************************************/
bool CAN_Id_Search(uint32_t id_rx);
HAL_StatusTypeDef CAN1_InitFiltersRxAll();
/*****************************************************************************
 * Function name    : CAN1_Init
 *    returns       : bool, true if successful initialization
 *    arg1          : EnumCANBitRate enBR, the chosen bit rate
 *    arg2          : bool bListen, true if supposed to listen CAN messages for reception and processing
 *    arg3          : uint32_t address_list[], the address list or id list to look for and receive
 *    arg4          : uint32_t address_count, the count of addresses to look for
 * Created by       : Owais
 * Date created     :
 * Description      : Initialize CAN1 peripheral
 * Notes            :
 *****************************************************************************/
bool CAN1_Init(EnumCANBitRate enBR, bool bListen, uint32_t address_list[], uint32_t address_count)
{
  bool result = true;
  uint32_t index;
  hcan1.Instance = CAN1;
  hcan1.pRxMsg = &Can1RxMsg1;
  hcan1.pRx1Msg = &Can1RxMsg2;
  hcan1.pTxMsg = &Can1TxMsg;
  bListenEnabled = bListen;
  CntListenAddresses = address_count;

  __HAL_RCC_GPIOB_CLK_ENABLE();

  switch(enBR)
  {
  case EnBitRate_62_5K:
	  hcan1.Init.Prescaler = 32;
	  break;
  case EnBitRate_125K:
	  hcan1.Init.Prescaler = 16;
	  break;
  case EnBitRate_250K:
	  hcan1.Init.Prescaler = 8;
	  break;
  case EnBitRate_500K:
	  hcan1.Init.Prescaler = 4;
	  break;
  case EnBitRate_1000K:
	  hcan1.Init.Prescaler = 2;
	  break;
  default:
	  result = false;
	  break;
  }

  if( (address_count <= MAX_LISTEN_ADDRESSES) && (result == true))
  {
	  hcan1.Init.Mode = CAN_MODE_NORMAL;
	  hcan1.Init.SJW = CAN_SJW_4TQ;
	  hcan1.Init.BS1 = CAN_BS1_8TQ;
	  hcan1.Init.BS2 = CAN_BS2_4TQ;
	  hcan1.Init.TTCM = DISABLE;
	  hcan1.Init.ABOM = ENABLE;
	  hcan1.Init.AWUM = DISABLE;
	  hcan1.Init.NART = DISABLE;
	  hcan1.Init.RFLM = ENABLE;
	  hcan1.Init.TXFP = DISABLE;

	  int rc;
	  if ((rc = HAL_CAN_Init(&hcan1)) != HAL_OK)
	  {
//		  _Error_Handler(__FILE__, __LINE__);
		  return rc;
	  }

	  if(bListen == true)
	  {
		  for(index = 0; index < address_count; index++)
		  {
			  /* copy the addresses */
			  if(address_list[index] == BroadcastAddress)
			  {
				  bListenAll = true;
				  break;
			  }
			  ListenAddresses[index] = address_list[index];
		  }
	  }

	  /* If address_count is less or equal to the hardware filters available, use hardware filters
	   * otherwise use software filter list just copied above */

	  HAL_StatusTypeDef resultConfig = CAN1_InitFiltersRxAll();

	  if(resultConfig != HAL_OK)
	  {
		  Error_Handler();
	  }

	  /* start reception */
	  if(HAL_CAN_Receive_IT(&hcan1, CAN_FIFO0) != HAL_OK)
	  {
		  /* Reception Error */
		  Error_Handler();
	  }

	  if(HAL_CAN_Receive_IT(&hcan1, CAN_FIFO1) != HAL_OK)
	  {
		  /* Reception Error */
		  Error_Handler();
	  }

	  CQ_Init(&CQ_CAN_Frames, &ArrCanRXMessages, ( sizeof (ArrCanRXMessages) / sizeof (ArrCanRXMessages[0]) ) - 1, sizeof (CanRxMsgTypeDef));
	  result = true;
  }
  else
  {
	  result = false;
  }
  return result;
}
/*****************************************************************************
 * Function name    : HAL_CAN_RxCpltCallback
 *    returns       : void
 *    arg1          : CAN_HandleTypeDef* hcan, CAN handle
 *    arg2          : uint8_t FIFONumber, the FIFO number which caused interrupt
 * Created by       : Owais
 * Date created     : 10-MAR-2018
 * Description      : CAN1 Message reception callback function
 * Notes            : The received message is from FIF0 or FIFO1, process non blocking
 *                    i.e., don't start anything which causes any delays or pending calls
 *
 *                    Store message in FIFO if need to do any blocking or time consuming calls
 *****************************************************************************/
void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* hc, uint8_t FIFONumber)
{
	bool bSearch;
	EnumCQErrors cqErr;
	CanRxMsgTypeDef* pReceiveMsg;

	if( FIFONumber == CAN_FIFO0 )
	{
		pReceiveMsg = hc->pRxMsg;
	}
	else if (FIFONumber == CAN_FIFO1 )
	{
		pReceiveMsg = hc->pRx1Msg;
	}

	/* process hcan1->pRxMsg in a NON BLOCKING way*/
	/* Enqueue after Filter through manual list if (bListenEnabled = true && bListenAll == false) */
	/* Enqueue anyway if bListenAll = TRUE */

	if(bListenAll == true)
	{
		//enqueue unconditionally
		cqErr = CQ_Enqueue(&CQ_CAN_Frames, (void*)pReceiveMsg);
	}
	else if( (bListenEnabled == true) && (bListenAll == false) )
	{
		//search for id in list
		if(pReceiveMsg->IDE == CAN_ID_STD)
		{
			bSearch = CAN_Id_Search(pReceiveMsg->StdId);
		}
		else
		{
			bSearch = CAN_Id_Search(pReceiveMsg->ExtId);
		}

		//enqueue if search result is true
		if(bSearch)
		{
			CntRxFilterOK++;
			//enqueue here
			cqErr = CQ_Enqueue(&CQ_CAN_Frames, (void*)pReceiveMsg);
		}
		else
		{
			CntRxFilterFail++;
		}
	}

	if(cqErr == CQ_ERROR_OK)
		CntEnqOK++;
	else
		CntEnqFail++;

	/* re-arm reception */
	if(HAL_CAN_Receive_IT(hc, FIFONumber) != HAL_OK)
	{
		/* Reception Error */
		Error_Handler();
	}
}
/*****************************************************************************
 * Function name    : CAN_Id_Search
 *    returns       : bool, true if id_rx was found in address list
 *    arg1          : uint32_t id_rx, the received id to search for
 * Created by       : Owais
 * Date created     : 11-Mar-2018
 * Description      : Search ListenAddressesfor id_rx
 * Notes            :
 *****************************************************************************/
bool CAN_Id_Search(uint32_t id_rx)
{
	bool result = false;

	for(uint32_t index=0; index < CntListenAddresses; index++)
	{
		if(ListenAddresses[index] == id_rx)
		{
			result = true;
			break;
		}
	}

	return result;
}
/*****************************************************************************
 * Function name    : CAN1_Write_Msg
 *    returns       : HAL_StatusTypeDef, returns either HAL_OK, HAL_ERROR, HAL_BUSY, or HAL_TIMEOUT
 *    arg1          : CanTxMsgTypeDef* pMsg, structure which encompasses
 *                    CAN id, payload data bytes, data length and Id type
 * Created by       : Owais
 * Date created     : 11-Mar-2018
 * Description      : Write a CAN message, polling mode
 * Notes            :
 *****************************************************************************/
HAL_StatusTypeDef CAN1_Write_Msg(CanTxMsgTypeDef* pMsg)
{
	HAL_StatusTypeDef result;

	/* Copy fields */
	hcan1.pTxMsg->DLC = pMsg->DLC;
	hcan1.pTxMsg->ExtId = pMsg->ExtId;
	hcan1.pTxMsg->IDE = pMsg->IDE;
	hcan1.pTxMsg->RTR = pMsg->RTR;
	hcan1.pTxMsg->StdId = pMsg->StdId;
	hcan1.pTxMsg->Data[0] = pMsg->Data[0];
	hcan1.pTxMsg->Data[1] = pMsg->Data[1];
	hcan1.pTxMsg->Data[2] = pMsg->Data[2];
	hcan1.pTxMsg->Data[3] = pMsg->Data[3];
	hcan1.pTxMsg->Data[4] = pMsg->Data[4];
	hcan1.pTxMsg->Data[5] = pMsg->Data[5];
	hcan1.pTxMsg->Data[6] = pMsg->Data[6];
	hcan1.pTxMsg->Data[7] = pMsg->Data[7];

	result = HAL_CAN_Transmit(&hcan1, TIMEOUT_TX_MS); /* await for transmission to complete */
	return result;
}
/*****************************************************************************
 * Function name    : CAN1_Read_Msg
 *    returns       : CanRxMsgTypeDef*, the pointer to the message being read out from queue
 *    arg1          : EnumCQErrors* cqErrOut, the error code for function call,
 *                    return value must be checked and is valid only when
 *                    value of cqErrOut is CQ_ERROR_OK
 * Created by       : Owais
 * Date created     : 11-Mar-2018
 * Description      : Read a message from circular queue in FIFO fashion.
 *                    Must check error code before using result
 * Notes            :
 *****************************************************************************/
CanRxMsgTypeDef* CAN1_Read_Msg(EnumCQErrors* cqErrOut)
{
	CanRxMsgTypeDef* pCF = (CanRxMsgTypeDef*)CQ_Dequeue(&CQ_CAN_Frames, cqErrOut);
	return pCF;
}
/*****************************************************************************
 * Function name    : CAN1_InitFiltersRxAll
 *    returns       : HAL_StatusTypeDef
 * Created by       : Owais
 * Date created     : 04-Apr-2018
 * Description      : Sets up CAN1 to receive all messages
 *                    FIL0 -> EXT
 *                    FIL1 -> STD
 * Notes            :
 *****************************************************************************/
HAL_StatusTypeDef CAN1_InitFiltersRxAll()
{
	HAL_StatusTypeDef result;
	CAN_FilterConfTypeDef  sFilterConfig;

	sFilterConfig.FilterNumber = 0;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	sFilterConfig.FilterIdHigh = 0xffff;
	sFilterConfig.FilterIdLow = 0xfffe;
	sFilterConfig.FilterMaskIdHigh = 0;
	sFilterConfig.FilterMaskIdLow = 0;
	sFilterConfig.FilterFIFOAssignment = CAN_FIFO0;
	sFilterConfig.FilterActivation = ENABLE;
	sFilterConfig.BankNumber = 14;
	HAL_StatusTypeDef res1  = HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig);

	sFilterConfig.FilterNumber = 1;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_16BIT;
	sFilterConfig.FilterIdHigh = 0xffff;
	sFilterConfig.FilterIdLow = 0xfffe;
	sFilterConfig.FilterMaskIdHigh = 0;
	sFilterConfig.FilterMaskIdLow = 0;
	sFilterConfig.FilterFIFOAssignment = CAN_FIFO1;
	sFilterConfig.FilterActivation = ENABLE;
	sFilterConfig.BankNumber = 14;
	HAL_StatusTypeDef res2  = HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig);

	if( ( res1 != HAL_OK) || (res2 != HAL_OK) )
	{
		result = HAL_ERROR;
	}
	else
	{
		result = HAL_OK;
	}
	return result;
}
#endif
