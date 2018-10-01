#ifndef __SOUTHBOUNDEC_H
#define __SOUTHBOUNDEC_H


#include "stm32f2xx_hal.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#include "stm32f2xx_hal.h"


//// Flash functions.

// Public Functions


//#define ORIGIN_SECTOR    ((uint32_t)0x08000000) /* first flash sector 16k */

#define ORIGIN_SECTOR    ((uint32_t)0x080E0000) /* last 128k */

HAL_StatusTypeDef FlashNVM_EraseSector(uint8_t sector);
HAL_StatusTypeDef FlashNVM_Read(uint32_t start_address, uint8_t* data_out, uint32_t size);
HAL_StatusTypeDef FlashNVM_Write(uint32_t start_address, const uint8_t* data_in, uint32_t size);

int MIC_Flash_Memory_Write(const uint8_t *data_in, uint32_t size);
int MIC_Flash_Memory_Read(const uint8_t *data_out, uint32_t size);

void MIC_Get_RTC(RTC_HandleTypeDef *hrtc, RTC_TimeTypeDef *sTime,RTC_DateTypeDef *sDate, uint32_t Format);
void MIC_Set_RTC (RTC_HandleTypeDef *hrtc, RTC_TimeTypeDef *sTime,RTC_DateTypeDef *sDate, uint32_t Format);

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
}
#endif


#endif
