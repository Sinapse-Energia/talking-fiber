#ifndef __SOUTHBOUNDEC_h
#define __SOUTHBOUNDEC_H


//#define FAMILY_F0
//#define FAMILY_F4

#include "stm32f2xx_hal.h"



#ifdef __cplusplus
extern "C" {
#endif

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* User can use this section to tailor TIMx instance used and associated
   resources */
/* Definition for TIMx clock resources */



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
void relayActivation(GPIO_TypeDef* gpio_PORT, uint16_t gpio_PIN);


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
void relayDeactivation(GPIO_TypeDef* gpio_PORT, uint16_t gpio_PIN);

/// HAL funtion to configure timer as PWM mode
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim);

/// Initialize clocks and duty cycle for PWM in timer3
/// #define PERIOD_PWM must have one value for period, in general 1082
void initializePWM(void);

/// Function that does one regulation over 1-10V output. If regulation is one value from 0 to 100,
/// Output is (regulation/10) Volts.
int dimming(int regulation);


//// Flash functions.

// Public Functions
#define FLASH_SECTOR_10    10U /*!< Sector Number 10  */
#define FLASH_SECTOR_11    11U /*!< Sector Number 11  */

#define ADDR_FLASH_SECTOR_5   ((uint32_t)0x8020000) /* sector 5, 128k*/
#define ADDR_FLASH_SECTOR_6   ((uint32_t)0x8040000) /* sector 6, 128k*/
#define ADDR_FLASH_SECTOR_7   ((uint32_t)0x8060000) /* sector 7, 128k*/
#define ADDR_FLASH_SECTOR_8   ((uint32_t)0x8080000) /* sector 8, 128k*/
#define ADDR_FLASH_SECTOR_9   ((uint32_t)0x80A0000) /* sector 9, 128k*/
#define ADDR_FLASH_SECTOR_10   ((uint32_t)0x80C0000) /* sector 10, 128k*/
#define ADDR_FLASH_SECTOR_11   ((uint32_t)0x80E0000) /* sector 11, 128k*/

HAL_StatusTypeDef FlashNVM_EraseSector(uint32_t sector);
HAL_StatusTypeDef FlashNVM_Read(uint32_t start_address, uint8_t* data_out, uint32_t size);
HAL_StatusTypeDef FlashNVM_Write(uint32_t start_address, const uint8_t* data_in, uint32_t size);

int MIC_Flash_Memory_Write(const uint8_t *data_in, uint32_t size);
int MIC_Flash_Memory_Read(const uint8_t *data_out, uint32_t size);




#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
}
#endif


#endif
