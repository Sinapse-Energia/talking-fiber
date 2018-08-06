#ifndef __SOUTHBOUNDEC_H
#define __SOUTHBOUNDEC_H


#include "stm32f2xx_hal.h"
#include "CAN_Util.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* User can use this section to tailor TIMx instance used and associated
   resources */
/* Definition for TIMx clock resources */
#define PERIOD_PWM 1082


#define TIMx_CLK_ENABLE()              __HAL_RCC_TIM3_CLK_ENABLE()

/* Definition for TIMx Channel Pins */
#define TIMx_CHANNEL_GPIO_PORT()       __HAL_RCC_GPIOB_CLK_ENABLE()
#define TIMx_GPIO_PORT_CHANNEL1        GPIOB
#define TIMx_GPIO_PORT_CHANNEL2        GPIOB
#define TIMx_GPIO_PORT_CHANNEL3        GPIOB
#define TIMx_GPIO_PORT_CHANNEL4        GPIOB
#define TIMx_GPIO_PIN_CHANNEL1         GPIO_PIN_4
#define TIMx_GPIO_PIN_CHANNEL2         GPIO_PIN_5
#define TIMx_GPIO_PIN_CHANNEL3         GPIO_PIN_0
#define TIMx_GPIO_PIN_CHANNEL4         GPIO_PIN_1
#define TIMx_GPIO_AF_CHANNEL1          GPIO_AF1_TIM3
#define TIMx_GPIO_AF_CHANNEL2          GPIO_AF1_TIM3
#define TIMx_GPIO_AF_CHANNEL3          GPIO_AF1_TIM3
#define TIMx_GPIO_AF_CHANNEL4          GPIO_AF1_TIM3
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#include "stm32f2xx_hal.h"


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


#if defined(BUILD_CANBUS)
/// Functions to carry out CAN operations
//  CAN device(s) configuration
int config_can(EnumCANBitRate baudRate, bool listen, uint32_t address_list[]);

//  CAN device send
int	write_can_message(uint32_t can_address, uint8_t *payload);

//  CAN device receive
int read_can_bus(uint32_t *address, uint8_t *payload);
#endif

// Sensor interfaces

double 	read_analog_signal(int pin);
bool	read_digital_signal(int pin);




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
