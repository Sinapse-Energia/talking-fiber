#include <stdlib.h> // RAND_MAX...
#include "southbound_ec.h"

extern TIM_HandleTypeDef    htim3;
extern TIM_OC_InitTypeDef sConfig;

static void MX_TIM3_Init(void);
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

// void relayActivation(
//		GPIO_TypeDef* gpio_PORT,
//		uint16_t gpio_PIN
// 		)
// Input parameters:
// ----> uint16_t gpio_PIN: Is the position in the port in STM32Fxx where relay is connected
// Output parameters: NONE
// Modified parameters:
// ----> GPIO_TypeDef* gpio_PORT: It is the handler to port in STM32Fxx microcontroller where relay is connected
// Type of routine: GENERIC (non dependent of device)
// Dependencies: NOTE
// Description:
// This function is used to activate one relay connected to gpio_PIN unto gpio_PORT of stm32fxx microcontroller.
// NOTE: For using this function, the GPIO must be initialized
// Example: relayActivation(GPIOX2_GPIO_Port,GPIOX2_Pin);
void relayActivation(GPIO_TypeDef* gpio_PORT, uint16_t gpio_PIN)
{
	/* Check the parameters */
	 assert_param(IS_GPIO_ALL_INSTANCE(gpio_PORT));
	 assert_param(IS_GPIO_PIN(gpio_PIN));

	HAL_GPIO_WritePin(gpio_PORT, gpio_PIN, GPIO_PIN_SET);


}

// void relayDeactivation(
//		GPIO_TypeDef* gpio_PORT,
//		uint16_t gpio_PIN
// 		)
// Input parameters:
// ----> uint16_t gpio_PIN: Is the position in the port in STM32Fxx where relay is connected
// Output parameters: NONE
// Modified parameters:
// ----> GPIO_TypeDef* gpio_PORT: It is the handler to port in STM32Fxx microcontroller where relay is connected
// Type of routine: GENERIC (non dependent of device)
// Dependencies: NOTE
// Description:
// This function is used to deactivate one relay connected to gpio_PIN unto gpio_PORT of stm32fxx microcontroller.
// NOTE: For using this function, the GPIO must be initialized
// Example: relayDeactivation(GPIOX2_GPIO_Port,GPIOX2_Pin);
void relayDeactivation(GPIO_TypeDef* gpio_PORT, uint16_t gpio_PIN)
{
	/* Check the parameters */
	assert_param(IS_GPIO_ALL_INSTANCE(gpio_PORT));
	assert_param(IS_GPIO_PIN(gpio_PIN));

	HAL_GPIO_WritePin(gpio_PORT, gpio_PIN, GPIO_PIN_RESET);

}



////////////////////
/////////////////////  PWM

/* TIM3 init function */
static void MX_TIM3_Init(void)
{

  TIM_MasterConfigTypeDef sMasterConfig;


  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = PERIOD_PWM;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfig.OCMode = TIM_OCMODE_PWM1;
  sConfig.Pulse = 0;
  sConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfig.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfig, TIM_CHANNEL_3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim3);

}


void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* htim_pwm)
{

  if(htim_pwm->Instance==TIM3)
  {
  /* USER CODE BEGIN TIM3_MspInit 0 */

  /* USER CODE END TIM3_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_TIM3_CLK_ENABLE();
  /* USER CODE BEGIN TIM3_MspInit 1 */

  /* USER CODE END TIM3_MspInit 1 */
  }

}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(htim->Instance==TIM3)
  {
  /* USER CODE BEGIN TIM3_MspPostInit 0 */

  /* USER CODE END TIM3_MspPostInit 0 */

    /**TIM3 GPIO Configuration
    PC8     ------> TIM3_CH3
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN TIM3_MspPostInit 1 */

  /* USER CODE END TIM3_MspPostInit 1 */
  }

}

void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef* htim_pwm)
{

  if(htim_pwm->Instance==TIM3)
  {
  /* USER CODE BEGIN TIM3_MspDeInit 0 */

  /* USER CODE END TIM3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM3_CLK_DISABLE();
  /* USER CODE BEGIN TIM3_MspDeInit 1 */

  /* USER CODE END TIM3_MspDeInit 1 */
  }

}



void initializePWM(void)
{

	MX_TIM3_Init();

}





/// Function that does one regulation over 1-10V output. If regulation is one value from 0 to 100,
/// Output is (regulation/10) Volts.
int dimming(int regulation)
{

	/// Francis TO REVIEW HW PWM for STM32F215RE

	 /* Set the pulse value for channel 2 */
	float temp=0.0;
	if (regulation>100) return -1;




		temp= (PERIOD_PWM/100.0)*(100-(regulation*0.9));   /// 100 - () because PWM inverts., regulation*0.9 to adapt scale.
		sConfig.Pulse = (uint32_t)temp;
		  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfig, TIM_CHANNEL_3) != HAL_OK)
		  {
		    /* Configuration Error */
		   Error_Handler();
		   return -1;
		  }


		  /* Start channel 3 */
		  if (HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3) != HAL_OK)
		  {
		    /* PWM Generation Error */
		   Error_Handler();
		   return -1;
		  }

		  return 1;

}

#define DEBUG
extern int hmqtt;
static int traza = 0;

#if defined(BUILD_CANBUS)
/// CAN Functions
//  CAN device(s) configuration
int config_can(EnumCANBitRate baudRate, bool listen, uint32_t address_list[]){
	int result;
	int naddress = 0;
	while (address_list[naddress])
		naddress++;
	bool bInitRes = CAN1_Init(baudRate, listen, address_list, naddress);

	 if(bInitRes == true) {
		 if (traza)
			 tprintf (hmqtt, "Init CAN OK");
		 result = 1;
	 }
	 else {
		 if (traza)
			 tprintf (hmqtt, "ERROR on Init CAN!!!!!!");
		 result = 0;
//		 _Error_Handler(__FILE__, __LINE__);
	 }
	 __enable_irq();

	return result;
}

//  CAN device send
int	write_can_message(uint32_t can_address, uint8_t *payload){
	 CanTxMsgTypeDef canMsgTx;
	HAL_StatusTypeDef hal_txStat;

	 canMsgTx.DLC = 8;
	 canMsgTx.IDE = CAN_ID_STD;
	 canMsgTx.StdId = can_address;
	 canMsgTx.RTR = CAN_RTR_DATA;

	 memset (canMsgTx.Data, 0, 8);
	 strcpy (canMsgTx.Data, payload);
//	 for(int i=0; i<8; i++)
//		 canMsgTx.Data[i] = payload[i];

	 if (traza)
		 tprintf (hmqtt, "writing...%8.8s message to 0x%X address", canMsgTx.Data, can_address);
	 hal_txStat = CAN1_Write_Msg(&canMsgTx);
	 if(hal_txStat == HAL_OK)
		 return 1;
	 else
		 return 0;
}

//  CAN device receive
int read_can_bus(uint32_t *address, uint8_t *payload){
	CanRxMsgTypeDef* ptrRxMsg;
	EnumCQErrors enCQErr;
	CanRxMsgTypeDef canMsgRx;

	if (traza){
		tprintf (hmqtt, "Before to call CAN1_Read_Msg");
		HAL_Delay (5000);
	}

	 ptrRxMsg = CAN1_Read_Msg(&enCQErr);
	 canMsgRx = *ptrRxMsg;
	 if (traza)
		 tprintf (hmqtt, "After CAN1_Read_Msg give %d", enCQErr);

	 if(enCQErr == CQ_ERROR_OK){
		 *address = ptrRxMsg->StdId;
		 memcpy (payload, ptrRxMsg->Data, 8);
		 return enCQErr;
	 }
	 else
		 return enCQErr;
}

#endif


/////////////////////////  Analog & Digital Sensor interfaces
#define MAX	 3.0
#define MIN	 0.1

double 	read_analog_signal(int pin){
	float r = (float)rand()/(float)RAND_MAX;
	double val = MIN + r * (MAX - MIN);

	return val;
}

#define ODDS	50
bool	read_digital_signal(int pin){
	int r = rand() % 100;
	return (r >= ODDS);
}






///////////////// Flash functions.



/**
  * @brief  erase Application or its copy FLASH
  * @param  fl_bank: flash area (application or its copy bank)
  * @retval true if OK,  otherwise return false
  */
HAL_StatusTypeDef FlashNVM_EraseSector(uint8_t sector)
{
	HAL_StatusTypeDef status;
	FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t SectorError = 0;
	uint8_t sector_start, sectors_n;



	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
	                           FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);


	EraseInitStruct.Sector = sector;
	EraseInitStruct.TypeErase = TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.NbSectors = 1;

	status = HAL_BUSY;
	while (status == HAL_BUSY) {
		status = HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
	}
	HAL_FLASH_Lock();

	return status;
}





/**
  * @brief  Read a binary array from FLASH
  * @param  address: FLASH relative address to read
  * @param  data_out: output data array pointer
  * @param  size: array length
  * @retval operation status
  */
HAL_StatusTypeDef FlashNVM_Read(uint32_t start_address, uint8_t* data_out, uint32_t size)
{
	/// from M0 m2m2
    //uint32_t sizeCounter = 0;

	//while (sizeCounter < size) {
	//    *data_out = (*(__IO uint8_t*)start_address);
	//    data_out++;
	//    start_address++;
	//    sizeCounter++;
	//}
    //return HAL_OK;

	//from new m2m as cortex m4
	uint32_t sizeCounter = 0;

		// Check input data
	    if (!IS_FLASH_ADDRESS(start_address)) {
	        // It's not Flash's address
	    	return HAL_ERROR;
		}

		while (sizeCounter < size) {
		    *data_out = (*(__IO uint32_t*)start_address);
		    data_out++;
		    start_address++;
		    sizeCounter++;
		}
	    return 1;
}


/**
  * @brief  write data array to PREVIOSLY ERISED FLASH memory
  * @param  fl_bank: flash area (application or its copy bank)
  * @retval true if OK,  otherwise return false
  */
HAL_StatusTypeDef FlashNVM_Write(uint32_t start_address, const uint8_t* data_in, uint32_t size)
{
	HAL_StatusTypeDef status = HAL_ERROR;
	uint32_t i;
	uint16_t *integerPointer;
	integerPointer = (uint16_t *)data_in;

	// remove for M0
	// Check input data
    //if (!IS_FLASH_ADDRESS(start_address)) {
        // It's not Flash's address
    //	return HAL_ERROR;
	//}

	// for new M2M
	// Check input data
	 if (!IS_FLASH_ADDRESS(start_address)) {
	      // It's not Flash's address
	    	return HAL_ERROR;
	 }

	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
	                           FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

	//__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR );



	// Write data
	  for (i = 0; i < size; i++) {
	   	status = HAL_BUSY;
	   	while (status == HAL_BUSY) {
	   		status = HAL_FLASH_Program(TYPEPROGRAM_BYTE, start_address + i, data_in[i]);
	   	}
	   	if ( status != HAL_OK) {
	   		break;
	   	}
   }

	HAL_FLASH_Lock();

	return status;
}

int MIC_Flash_Memory_Write(const uint8_t *data_in, uint32_t size)
{


	HAL_StatusTypeDef status = HAL_ERROR;
	uint32_t sizeReceived=0;


	//status=  FlashNVM_EraseSector(FLASH_SECTOR_0);// it erased 16kb at beginning of 0x8000000
	status=  FlashNVM_EraseSector(FLASH_SECTOR_11);// it is erased last flash sector TO CHANGE. A lot of bytes 128kb

	if (status==HAL_ERROR) return -1; // if fails, it returns -1;

	status=FlashNVM_Write(ORIGIN_SECTOR+4, data_in, size); // data are written with +4 offset from starting. First 4 bytes are used to write size of data
																// and watching if there are data or not.
	if (status==HAL_ERROR) return -1; // if fails, it returns -1;

	status=FlashNVM_Write(ORIGIN_SECTOR, &size, 4); // quantity of data are saved at first 4 bytes.

	if (status==HAL_ERROR) return -1; // if fails, it returns -1;

	status=FlashNVM_Read(ORIGIN_SECTOR, &sizeReceived, 4); // quantity of data are read again to verify it wrote good.

	if (status==HAL_ERROR) return -1; // if fails, it returns -1;

	if (status==HAL_OK) return sizeReceived; // if all goes fine, it returns size of data.


}

int MIC_Flash_Memory_Read(const uint8_t *data_out, uint32_t size)
{
	HAL_StatusTypeDef status = HAL_ERROR;
	uint32_t sizeReceived=0;

	status=FlashNVM_Read(ORIGIN_SECTOR+4, data_out, size); // start to read all data.
	status=FlashNVM_Read(ORIGIN_SECTOR+4, data_out, size); // start to read all data.

	if (status==HAL_ERROR) return -1; // if fails, it returns -1;

	status=FlashNVM_Read(ORIGIN_SECTOR, &sizeReceived, 4); // received quantity of saved bytes.

	if (status==HAL_ERROR) return -1; // if fails, it returns -1;

	if ((status==HAL_OK)&&(sizeReceived==size)) return sizeReceived; // if all goes fine, it returns size of data.


}

void MIC_Get_RTC(RTC_HandleTypeDef *hrtc, RTC_TimeTypeDef *sTime,RTC_DateTypeDef *sDate, uint32_t Format)
{
	//Get Time & Date from Internal_RTC
	HAL_RTC_GetTime(hrtc, sTime, Format);
	HAL_RTC_GetDate(hrtc, sDate, Format);
}

void MIC_Set_RTC (RTC_HandleTypeDef *hrtc, RTC_TimeTypeDef *sTime,RTC_DateTypeDef *sDate, uint32_t Format)
{
	//Set Time & Date to Internal_RTC
	HAL_RTC_SetTime(hrtc, sTime, RTC_FORMAT_BIN);
	HAL_RTC_SetDate(hrtc, sDate, RTC_FORMAT_BIN);
}


