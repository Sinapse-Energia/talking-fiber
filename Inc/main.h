/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H__
#define __MAIN_H__

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx_hal.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define LED_BLUE_Pin GPIO_PIN_13
#define LED_BLUE_GPIO_Port GPIOC
#define LED_RED_Pin GPIO_PIN_14
#define LED_RED_GPIO_Port GPIOC
#define LED_GREEN_Pin GPIO_PIN_15
#define LED_GREEN_GPIO_Port GPIOC
#define STATUS_Pin GPIO_PIN_1
#define STATUS_GPIO_Port GPIOC
#define NETLIGHT_Pin GPIO_PIN_2
#define NETLIGHT_GPIO_Port GPIOC
#define AMP_RSSI_TO_ANALOG_Pin GPIO_PIN_1
#define AMP_RSSI_TO_ANALOG_GPIO_Port GPIOA
#define AD_EXTERNAL_BATTERY_Pin GPIO_PIN_5
#define AD_EXTERNAL_BATTERY_GPIO_Port GPIOA
#define PFM_TO_ANALOGUE_Pin GPIO_PIN_6
#define PFM_TO_ANALOGUE_GPIO_Port GPIOA
#define RSSI_TO_ANALOGUE_Pin GPIO_PIN_7
#define RSSI_TO_ANALOGUE_GPIO_Port GPIOA
#define nSHDN_Pin GPIO_PIN_5
#define nSHDN_GPIO_Port GPIOC
#define SDCARD_CD_Pin GPIO_PIN_1
#define SDCARD_CD_GPIO_Port GPIOB
#define IN_RESET_HARDWARE_Pin GPIO_PIN_12
#define IN_RESET_HARDWARE_GPIO_Port GPIOB
#define EX_RESET_PHOTODIODE_Pin GPIO_PIN_14
#define EX_RESET_PHOTODIODE_GPIO_Port GPIOB
#define EX_ENABLE_GPRS_BATTERY_Pin GPIO_PIN_15
#define EX_ENABLE_GPRS_BATTERY_GPIO_Port GPIOB
#define GPIO_SLEEP_GPRS_Pin GPIO_PIN_9
#define GPIO_SLEEP_GPRS_GPIO_Port GPIOC
#define CTRL_EMERG_RESET_Pin GPIO_PIN_11
#define CTRL_EMERG_RESET_GPIO_Port GPIOA
#define CONTROL_PWRKEY_Pin GPIO_PIN_12
#define CONTROL_PWRKEY_GPIO_Port GPIOA
#define SPI1_NSS_Pin GPIO_PIN_15
#define SPI1_NSS_GPIO_Port GPIOA

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
/* #define USE_FULL_ASSERT    1U */

/* USER CODE BEGIN Private defines */

#define M95_CTRL_EMERG_GPIO_Port 	CTRL_EMERG_RESET_GPIO_Port
#define M95_CTRL_PWRKEY_GPIO_Port 	CONTROL_PWRKEY_GPIO_Port
#define M95_STATUS_GPIO_Port 		STATUS_GPIO_Port

#define M95_CTRL_EMERG_Pin 			CTRL_EMERG_RESET_Pin
#define M95_CTRL_PWRKEY_Pin 		CONTROL_PWRKEY_Pin
#define M95_STATUS_Pin 				STATUS_Pin

#ifdef __cplusplus
 extern "C" {
#endif

int	COMM_Init();

#ifdef __cplusplus
}
#endif

/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
