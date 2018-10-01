
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f2xx_hal.h"
#include "adc.h"
#include "dma.h"
#include "fatfs.h"
#include "i2c.h"
#include "iwdg.h"
#include "rtc.h"
#include "sdio.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */

#include <string.h>
#include <stdlib.h>  // provisional atoi()
#include <stdarg.h>
#include <time.h>  // provionale time()
#include "math.h"

#include "Definitions.h"

#ifdef COMMUNICATION_M95
#include "M95lite.h"
#include "circular.h"
#endif
#include "southbound_ec.h"
#include "MQTTAPI.H"
#include "NBinterface.h"
#include "utils.h"
#include <stdbool.h>
#include "crc16.h"
#include "Shared.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

TIM_HandleTypeDef	TimHandle;
TIM_OC_InitTypeDef	sConfig;

uint16_t elapsed10seconds=0; 					// At beginning this is 0
uint8_t WDT_ENABLED=1;						 /// Enable for activate independent watch dog timer
uint8_t timeoutGPRS=0; 						/// At beginning this is 0

#ifdef COMMUNICATION_M95

extern DMA_HandleTypeDef hdma_usart6_rx;

char *Read_TFVOL(const char *);

int elapsed10secondsAux=0;
uint8_t LOG_ACTIVATED=0;				 		/// Enable to 1 if you want to show log through logUART
uint8_t LOG_GPRS=0;  							/// For showing only GPRS information
uint32_t timeout=1000;				 		/// Timeout between AT command sending is 1000 milliseconds.
uint8_t rebootSystem=0;						 /// At beginning this is 0
uint8_t nTimesMaximumFail_GPRS=2; 			/// For initial loop of initializing GPRS device
uint8_t retriesGPRS=1; 						/// only one retries per AT command if something goes wrong
uint8_t existDNS=1; 							/// The IP of main server to connect is not 4 number separated by dot. It is a DNS.
uint8_t offsetLocalHour=0; 					/// for getting UTC time
uint8_t calendar[10];               			/// Array for saving all calendar parameters get from NTC server
uint8_t idSIM[30];                 			 /// Array where is saved ID from SIMcard
uint8_t openFastConnection=0;      			 /// by default to 0, it is used for doing a quick connection when it is needed to call the connect function again
uint8_t setTransparentConnection=1;  			/// 1 for transparent connection, 0 for non-transparent. Then all data flow is command AT+ data
uint8_t GPRSbuffer[SIZE_GPRS_BUFFER];			 /// received buffer with data from GPRS
uint8_t dataByteBufferIRQ;  					/// Last received byte from GPRS
uint16_t GPRSBufferReceivedBytes;     		/// Number of received data from GPRS after a cleanningReceptionBuffer() is called
uint16_t indexGPRSBufferReceived;
uint16_t indexPickingReceivedBytes=0;

int	nirqs = 0;

st_CB *DataBuffer;

int modem_init = 0;

#endif

int hmqtt = 0;

char topictr[128];

#if defined(DEBUG_ENABLE_MQTT_QUEUE_PUB)
char topicmqdebug[128];
#endif

int today = 0; // Current day, used for sunrise/sunset calculation
int prevMinute = 0, prevHour = 0;
#if defined(TIME_CORRECT_PERIODICALLY)
int lastTimeCorrectHour = 0;
#endif

volatile bool reconnectScheduled = false;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

int NEWCONN()
{
    reconnectScheduled = true;
    return 1;
}

void Modem_preinit()
{
//    HAL_GPIO_WritePin(EX_ENABLE_GPRS_BATTERY_GPIO_Port, EX_ENABLE_GPRS_BATTERY_Pin, GPIO_PIN_SET);
//    HAL_Delay(1000);

    HAL_UART_DMAStop(&HUART_GPRS);

    int rc;
    int n = 0;
    do {
        rc = Modem_Init();
        n++;
    } while (rc != M95_OK);
    modem_init = 1;

    int tries = 0;
    HAL_StatusTypeDef rc2;
    do {
        rc2 = HAL_UART_Receive_DMA(&HUART_GPRS, DataBuffer->buffer, DataBuffer->size); // starts DMA reception
        HAL_Delay(200);
        tries++;
    } while  (rc2 != HAL_OK);
}

int Reconnect(int *handle)
{
    if (*handle > 0) MqttDisconnect(*handle);
    int ntries = 0;
    do {
        *handle = COMM_Init();
        ntries++;
    } while (*handle <= 0);

    if (!UpdateSharedClientConnected())
    {
#if defined(DEBUG)
        tprintf(*handle, "Update shared memory failed!");
#endif
    }

    return ntries;
}

void Disconnect()
{
#ifndef CONNECT_ONLY_TO_SEND_WORKAROUND
    MqttDisconnect(hmqtt);
#endif
    hmqtt = 0;
    static const char pwroffCmd[] = "AT+QPOWD=1\r";
    HAL_UART_Transmit(&HUART_GPRS, (uint8_t*)pwroffCmd, strlen(pwroffCmd), 1000);
//    HAL_GPIO_WritePin(EX_ENABLE_GPRS_BATTERY_GPIO_Port, EX_ENABLE_GPRS_BATTERY_Pin, GPIO_PIN_RESET);
}

void OnDemandHander()
{
    char inbuffer[256];
    char intopic[64];
    char outtopic[64];
    char *mssg = NULL;
    static int cnomssg = 0;

    intopic[0] = '\0';

    RGB_Color_Set(RGB_COLOR_OFF);

    // Get message from queue
    mssg = MqttGetMessage(hmqtt, inbuffer, 256, intopic, 256);

    if (mssg)
    {
        // Message received and will be processed
        RGB_Color_Set(RGB_COLOR_CYAN);

        cnomssg = 0;
        tprintf(hmqtt, "Incoming message #%s#:#%s#", intopic, mssg);

        // Now Context API not checking the topics - do these outside engine

        // By default publish to debug
        sprintf(outtopic, "%s/CMC/DEBUG", GetVariable("MTPRT"));

        if ((strlen(mssg) >= 4 && memcmp("207;", mssg, 4) == 0) ||
            (strlen(mssg) >= 4 && memcmp("208;", mssg, 4) == 0) ||
            (strlen(mssg) >= 24 &&
                    memcmp("L3_TF_CONF_LISTEN_STATE;", mssg, 24) == 0) ||
            (strlen(mssg) >= 29 &&
                    memcmp("L3_TF_CONF_SEND_STATUS_STATE;", mssg, 29) == 0))
        {
            sprintf(outtopic, "%s/CMC/CONFIG", GetVariable("MTPRT"));
        }

        // Process using context API
        char *out = ProcessMessage(mssg);

        if ((uint32_t)out != 0 && memcmp("999;", out, 4) == 0)
        { // if Unknown command
            if (strlen(mssg) == 15 &&
                memcmp("314;", mssg, 4) == 0 &&
                memcmp(";12345678;", &mssg[5], 10) == 0)
            { // Erase command received

                // response for the Erase command
                out = "714;TRUE;";

                // Erase NVM
#ifdef USE_SD_CARD
                switch (mssg[4])
                {
                case '0':
                {
                    if(f_mkfs((TCHAR const*)SDPath, 0, 0) != FR_OK)
                    {
                        out = "714;FALSE;";
                        RGB_Color_Set(RGB_COLOR_YELLOW);
                    }
                    break;
                }
                default:
                    out = "714;WrongArgument;";
                }
#endif
                // TODO Erase

                // Send response immediately
                MqttPutMessage(hmqtt, outtopic, out);
                MqttPutMessage(hmqtt, outtopic, out);

                HAL_Delay(1000);

                // Reboot device
                NVIC_SystemReset();
            }
        }

        // put out into send buffer
        if ((uint32_t)out != 0)
        {
            int rc = MqttPutMessage(hmqtt, outtopic, out);
            if (rc < 1)
            {
                int n = Reconnect(&hmqtt);
                tprintf(hmqtt, "RECONNECTED because of communication breakdown after %i tries!!!!", n);
            }
        }
    }
    else
    {
        cnomssg++;
        if (cnomssg > 3)
        {
            // Send Ping request
            int rc = MqttPing(hmqtt);

            if (rc < 1)
            {
              //  It seems we are'nt receiving anymore
              tprintf(hmqtt, "Sended Ping without getting PINGRESP (deafness?)");

              // Reconnect
              tprintf(hmqtt, "Ready  to reconnexion or reboot!!! ");
              int ntries = Reconnect(&hmqtt);
              tprintf(hmqtt, "Hearing recovered after %d tries!!", ntries);
            }
            else
            {
              tprintf(hmqtt, "Sended Ping getting %d", rc);
            }

            cnomssg = 0;
        }
    }
}

void ExecPeriodic(void)
{
	char *out = ProcessMessage("L3_TF_STATUS_OD;");
	// put out into send buffer
	if ((uint32_t)out != 0)
	{
		char outtopic[64];
		// Topic to publish answer
		sprintf(outtopic, "%s/%s",
			  GetVariable("MTPRT"),
			  GetVariable("DOPPT"));
	#if defined(CONNECT_ONLY_TO_SEND)
		Modem_preinit();
		Reconnect(&hmqtt);
		tprintf(hmqtt, "Connected!");
	#endif
		int rc = MqttPutMessage(hmqtt, outtopic, out);
		if (rc < 1)
		{
			int n = Reconnect(&hmqtt);
			tprintf(hmqtt, "RECONNECTED because of communication breakdown after %i tries!!!!", n);
		}
	#if defined(CONNECT_ONLY_TO_SEND)
		tprintf(hmqtt, "Disconnecting!");
		Disconnect();
	#endif
	}
}

void ExecAlert(int mv)
{
	char outtopic[64];
	char out[64];
	snprintf(out, 128, "L3_TF_ALERT;%s;%i;%s;",
			GetVariable("ID"), mv, GetVariable("TFVTS"));
	// Topic to publish answer
	sprintf(outtopic, "%s/%s",
			GetVariable("MTPRT"),
			GetVariable("DALPT"));
#if defined(CONNECT_ONLY_TO_SEND)
	Modem_preinit();
	Reconnect(&hmqtt);
	tprintf(hmqtt, "Connected!");
#endif
	int rc = MqttPutMessage(hmqtt, outtopic, out);
	if (rc < 1)
	{
		int n = Reconnect(&hmqtt);
		tprintf(hmqtt, "RECONNECTED because of communication breakdown after %i tries!!!!", n);
	}
#if defined(CONNECT_ONLY_TO_SEND)
	tprintf(hmqtt, "Disconnecting!");
	Disconnect();
#endif
}

/**
 * @brief Initiate MCU sleep/standby and wake it up in sec period
 */
void Sleep_Delay(uint32_t sec)
{
	if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, sec, RTC_WAKEUPCLOCK_CK_SPRE_16BITS) != HAL_OK)
	  {
	    _Error_Handler(__FILE__, __LINE__);
	  }
#if defined(USE_SLEEPMODE)
	HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFE);// WFE, cause WKUPTimer is event
#endif

#if defined(USE_STOPMODE)
	HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
#endif

#if defined(USE_STANDBY)
	HAL_PWR_EnterSTANDBYMode();
#endif

}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

	__enable_irq();

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  HAL_Delay(1000);
  /* Enable Power Clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
//  MX_I2C1_Init();
#ifdef USE_SD_CARD
  MX_SDIO_SD_Init();
#endif
//  MX_SPI1_Init();
//  MX_USART1_UART_Init();
//  MX_USART2_UART_Init();
  MX_USART6_UART_Init();
  MX_RTC_Init();
  MX_TIM7_Init();
  MX_IWDG_Init();
#ifdef USE_SD_CARD
  MX_FATFS_Init();
#endif
  /* USER CODE BEGIN 2 */

  RGB_Color_Set(RGB_COLOR_YELLOW);

  HAL_TIM_Base_Start_IT(&htim7); //Activate IRQ for Timer7

  HAL_Delay(30);

  if (WDT_ENABLED==1) {
	  MX_IWDG_Init();
	  __HAL_IWDG_START(&hiwdg); //no se inicializar watchdog, se deshabilita para debug
	  HAL_IWDG_Refresh(&hiwdg);
  }

#ifdef COMMUNICATION_M95
	DataBuffer = CircularBuffer (256, &hdma_usart6_rx);
    HAL_GPIO_WritePin(EX_ENABLE_GPRS_BATTERY_GPIO_Port, EX_ENABLE_GPRS_BATTERY_Pin, GPIO_PIN_SET);
	Modem_preinit();
#endif

    // Initialize context and connect

    // Create the Variable's Context
    CreateContext();

    // Read the messages's metadata
    ReadMetadata("", "");

    RestoreSharedValues();

    // Initialize debug topic
    sprintf (topictr, "%s/CMC/DEBUG", GetVariable("MTPRT"));

#if defined(DEBUG_ENABLE_MQTT_QUEUE_PUB)
    sprintf (topicmqdebug, "%s/%s/MQTT_DEBUG", GetVariable("MTPRT"), GetVariable("ID"));
#endif

#ifdef HYBRID_M95_WLAN_CODE
    transport_get_time();
//    transport_update_tz();
#endif

    Reconnect(&hmqtt);

#if defined(DEBUG_ENABLE_MQTT_QUEUE_PUB)
    // Update debug topic - ID could be changed after connection (IMEI case)
    sprintf (topicmqdebug, "%s/%s/MQTT_DEBUG", GetVariable("MTPRT"), GetVariable("ID"));
#endif

#if defined(TIME_CORRECT_PERIODICALLY)
    lastTimeCorrectHour = atoi(getHour());
#endif

#ifdef CONNECT_ONLY_TO_SEND_WORKAROUND
#if defined(ENABLE_PERIODIC)
	  ExecPeriodic();
#endif
#if defined(ENABLE_ALERT)
	  int mv = atoi(GetVariable("TFVOL"));
	  if (mv < atoi(GetVariable("TFATH")))
	  {
		  ExecAlert(mv);
	  }
#endif
#endif

#if defined(CONNECT_ONLY_TO_SEND) || defined(CONNECT_ONLY_TO_SEND_WORKAROUND)
    // Disconnect and disable M95
    Disconnect();
#endif
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      // Five second granularity
      Sleep_Delay(5);

      HAL_IWDG_Refresh(&hiwdg);

      // Blink once a second with green RGB if connected
      /*if (hmqtt > 0)*/ RGB_Color_Blink(RGB_COLOR_GREEN);

      if (HAL_GPIO_ReadPin(M95_STATUS_GPIO_Port, M95_STATUS_Pin) == GPIO_PIN_SET)
    	  RGB_Color_Set(RGB_COLOR_CYAN);
      else
    	  RGB_Color_Set(RGB_COLOR_OFF);

#if !defined(CONNECT_ONLY_TO_SEND) && !defined(CONNECT_ONLY_TO_SEND_WORKAROUND)
      OnDemandHander();
#endif

#ifndef CONNECT_ONLY_TO_SEND_WORKAROUND
      // Check for reconnect scheduled
      if (reconnectScheduled)
      {
          SaveDevParamsToNVM();

          char *host = GetVariable("MURI");
          unsigned int port = atoi(GetVariable("MPORT"));

          tprintf(hmqtt,
              "Go to DISCONNECT from this server and connect to %s:%d.\nBye",
                  host, port);

          Reconnect(&hmqtt);

          tprintf(hmqtt, "This has been a RECONNECTION!!!");

          reconnectScheduled = false;
      }
#endif

      // Each minute perform scheduler tasks

      // Get now hour and minute
      int nowHour = atoi(getHour());
      int nowMinute = atoi(getMinute());

      if (nowHour != prevHour)
      {
          prevHour = nowHour;

          // Sunrise/Sunset calculation once a day
          int nowDay = 0, nowMonth = 0, nowYear = 0;
          getDate(&nowDay, &nowMonth, &nowYear);
          if (today != nowDay)
          {
              today = nowDay;

              // Get coordinates
              double lat = atof(GetVariable("GPSLAT"));
              double lon = atof(GetVariable("GPSLON"));

              // Calculate SR/SS
              int srHour = 6, srMin = 0, ssHour = 21, ssMin = 0;
              CalculateSunRiseSetTimes(lat, lon,
                      nowYear, nowMonth, nowDay,
                      &srHour, &srMin, &ssHour, &ssMin);

              char buf[8];
              // Set SR/SS variables
              sprintf(buf, "%i", srHour);
              SetVariable("GPSSRH", buf);
              sprintf(buf, "%i", srMin);
              SetVariable("GPSSRM", buf);
              sprintf(buf, "%i", ssHour);
              SetVariable("GPSSSH", buf);
              sprintf(buf, "%i", ssMin);
              SetVariable("GPSSSM", buf);
          }
      }

      if (nowMinute != prevMinute)
      {
          prevMinute = nowMinute;

          SaveDevParamsToNVM();

          int nowAbsMinute = nowMinute + nowHour * 60;

#if defined(ENABLE_PERIODIC)
          if (nowAbsMinute % atoi(GetVariable("TFPER")) == 0)
          {
#ifdef CONNECT_ONLY_TO_SEND_WORKAROUND
        	  NVIC_SystemReset();
#else
              ExecPeriodic();
#endif
          }
#endif
#if defined(ENABLE_ALERT)
          if (nowAbsMinute % atoi(GetVariable("TFALM")) == 0)
          {
        	  int mv = atoi(Read_TFVOL("-1"));
        	  if (mv < atoi(GetVariable("TFATH")))
        	  {

#ifdef CONNECT_ONLY_TO_SEND_WORKAROUND
        		  NVIC_SystemReset();
#else
        		  ExecAlert(mv);
#endif
        	  }
          }
#endif
      }

#if defined(TIME_CORRECT_PERIODICALLY)
      if (lastTimeCorrectHour != nowHour &&
              (nowHour % TIME_CORRECT_PERIOD_HOURS) == 0)
      {
          lastTimeCorrectHour = nowHour;
          // TODO correct time with server
      }
#endif
  }
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 8;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance==TIM7)
  {
    AddSeconds(10);
    
	elapsed10seconds++;
    if (elapsed10seconds%TIMING_TIMEOUT_GPRS==0)
    {
      /// Tiempo timeoutGPRS
      timeoutGPRS=1;
    }
  }
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
