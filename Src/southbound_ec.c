#include <stdlib.h> // RAND_MAX...
#include "southbound_ec.h"
#include "adc.h"




///////////////// Flash functions.

uint32_t adc_read_code(uint32_t channel)
{
    uint32_t code;
    ADC_ChannelConfTypeDef sConfig;

    sConfig.Channel = channel;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10000);
    code = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return code;
}

float adc_read_avdd()
{
  return (3.3f);// * VREFINT_CAL / adc_read_code(ADC_CHANNEL_VREFINT));
}

float adc_read_val(uint32_t channel)
{
  return (adc_read_code(channel) * adc_read_avdd() / ((1 << 12) - 1));
}


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


