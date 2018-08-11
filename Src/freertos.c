/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */     
#include <stm32f2xx_hal.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "threadSafeQueue.h"
#include "threadSafeWrappers.h"
#include "utils.h"
#include "MQTTAPI.H"
#include "Definitions.h"
#include "NBinterface.h"
#include "rtc.h"
#include "iwdg.h"
#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "wlan.h"
#include "ff_gen_drv.h"
#include "sd_diskio.h"
extern char SDPath[4];
#ifdef HYBRID_M95_WLAN_CODE
#include "M95lite.h"
#endif
/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId mqttReceptionHandle;
osThreadId mqttSendHandle;
osThreadId contextDaemonHandle;
osThreadId schedulerTriggeHandle;

/* USER CODE BEGIN Variables */

osThreadId iwdgRefresherHandle;

volatile bool rebootScheduled = false;
volatile bool pingScheduled = false;
volatile bool reconnectScheduled = false;
volatile long rebootAtTimestamp = 0;
volatile bool initialized = false;
volatile bool processing = false;
// TODO next checks should be improved in future
volatile uint32_t mqttRecvTaskLastTS = 0;
volatile uint32_t schedulerTaskLastTS = 0;

static threadSafeQueueHandler publishQueue;

int hmqtt = 0;

char topictr[128];

#if defined(DEBUG_ENABLE_MQTT_QUEUE_PUB)
char topicmqdebug[128];
#endif

/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void mqttReceptionProc(void const * argument);
void mqttSendProc(void const * argument);
void contextDaemonProc(void const * argument);
void schedulerTriggersProc(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */

void iwdgRefresherProc(void const * argument);

/* USER CODE END FunctionPrototypes */

/* Hook prototypes */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

/* USER CODE BEGIN 4 */
__weak void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
	NVIC_SystemReset();
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
__weak void vApplicationMallocFailedHook(void)
{
   /* vApplicationMallocFailedHook() will only be called if
   configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h. It is a hook
   function that will get called if a call to pvPortMalloc() fails.
   pvPortMalloc() is called internally by the kernel whenever a task, queue,
   timer or semaphore is created. It is also called by various parts of the
   demo application. If heap_1.c or heap_2.c are used, then the size of the
   heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
   FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
   to query the size of free heap space that remains (although it does not
   provide information on how the remaining heap might be fragmented). */
	NVIC_SystemReset();
}
/* USER CODE END 5 */

/* Init FreeRTOS */

void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
	publishQueue = threadSafeQueueInit();
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
	ThreadsafeWrappersInit();
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of mqttReception */
  osThreadDef(mqttReception, mqttReceptionProc, osPriorityNormal, 0, 4096);
  mqttReceptionHandle = osThreadCreate(osThread(mqttReception), NULL);

  /* definition and creation of mqttSend */
  osThreadDef(mqttSend, mqttSendProc, osPriorityNormal, 0, 1024);
  mqttSendHandle = osThreadCreate(osThread(mqttSend), NULL);

  /* definition and creation of contextDaemon */
  osThreadDef(contextDaemon, contextDaemonProc, osPriorityNormal, 0, 1024);
  contextDaemonHandle = osThreadCreate(osThread(contextDaemon), NULL);

  /* definition and creation of schedulerTrigge */
  osThreadDef(schedulerTrigge, schedulerTriggersProc, osPriorityNormal, 0, 4096);
  schedulerTriggeHandle = osThreadCreate(osThread(schedulerTrigge), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  osThreadDef(iwdgRefresher, iwdgRefresherProc, osPriorityRealtime, 0, 256);
  iwdgRefresherHandle = osThreadCreate(osThread(iwdgRefresher), NULL);

  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
}

/* mqttReceptionProc function */
void mqttReceptionProc(void const * argument)
{

  /* USER CODE BEGIN mqttReceptionProc */

	/// This procedure do next:
	/// - Receive incoming MQTT messages
	/// - Process messages with engine
	/// - Subscribe to new EPD topics, schedule ping and reboot actions
	/// - Put output messages to publish queue

	char inbuffer[256];
	char intopic[64];
	char outtopic[64];
	char *mssg = NULL;
	int cnomssg = 0;

	while (!initialized) osDelay(100);

	/* Infinite loop */
	for(;;)
	{
		mqttRecvTaskLastTS = HAL_GetTick();

		intopic[0] = '\0';

		RGB_Color_Set(RGB_COLOR_OFF);
		processing = false;

		// Get message from queue
		mssg = MqttGetMessage(hmqtt, inbuffer, 256, intopic, 256);

		if (mssg)
		{
			// Message received and will be processed
			processing = true;
			RGB_Color_Set(RGB_COLOR_CYAN);

			cnomssg = 0;
			TraceThreadSafe(hmqtt, "Incoming message #%s#:#%s#", intopic, mssg);

			// Now Context API not checking the topics - do these outside engine

			ContextLock();

			// By default publish to debug
			sprintf(outtopic, "%s/CMC/DEBUG", GetVariable("MTPRT"));

			if ((strlen(mssg) >= 4 && memcmp("204;", mssg, 4) == 0) ||
					(strlen(mssg) >= 4 && memcmp("207;", mssg, 4) == 0) ||
					(strlen(mssg) >= 4 && memcmp("208;", mssg, 4) == 0) ||
					(strlen(mssg) >= 4 && memcmp("209;", mssg, 4) == 0))
			{
				sprintf(outtopic, "%s/CMC/CONFIG", GetVariable("MTPRT"));
			}

			// Process using context API
			char *out = ProcessMessage(mssg);

			ContextUnlock();

			if ((uint32_t)out != 0 && memcmp("999;", out, 4) == 0)
			{ // if Unknown command
				if (strlen(mssg) >= 4 &&
					memcmp("398;", mssg, 4) == 0)
				{ // Reboot command received
					long minutes = 0;
					if (strlen(mssg) > 4)
					{
						char buf[16] = { 0 };
						// Copy minutes to buffer
						memcpy(buf, &mssg[4], (strlen(&mssg[4])-1));
						// Parse minutes
						sscanf(buf, "%li", &minutes);
					}
					// Calculate reboot timestamp
					rebootAtTimestamp = GetTimeStamp() + minutes*60;

					// Schedule reboot
					rebootScheduled = true;

					// No responce for the Reboot command
					out = NULL;
				}
				else if (strlen(mssg) == 15 &&
					memcmp("314;", mssg, 4) == 0 &&
					memcmp(";12345678;", &mssg[5], 10) == 0)
				{ // Erase command received

					// response for the Erase command
					out = "714;TRUE;";

					// Disallow all operations during erasing NVM
					ContextLock();

					// Erase NVM
					switch (mssg[4])
					{
					case '0':
					{
						static uint8_t buffer[_MAX_SS];
						if(f_mkfs((TCHAR const*)SDPath, FM_ANY, 0, buffer, sizeof(buffer)) != FR_OK)
						{
							out = "714;FALSE;";
							RGB_Color_Set(RGB_COLOR_YELLOW);
						}
						break;
					}
					default:
						out = "714;WrongArgument;";
					}

					// Send response immediately
					MqttPutMessage(hmqtt, outtopic, out);
					MqttPutMessage(hmqtt, outtopic, out);

					osDelay(1000);

					// Reboot device
					NVIC_SystemReset();
				}
				else if (strlen(mssg) > 4 &&
					memcmp("127;", mssg, 4) == 0)
				{ // Set time command

					// Parse command parameters
					int yr = -1, mo = -1, da = -1, hr = -1, mn = -1, sc = -1;
					sscanf(mssg+4, "%d;%d;%d;%d;%d;%d;", &yr, &mo, &da,
							&hr, &mn, &sc);

					// Check parameters
					if (!(yr < 0 || mo < 1 || mo > 12 || da < 1 || da > 31 ||
							hr < 0 || hr > 23 || mn < 0 || mn > 59 ||
							sc < 0 || sc > 59))
					{
						// Normalize year
						yr = yr % 100;

						// Set time
						char buf[32];
						sprintf(&buf[0], "  %d/%d/%d,%d:%d:%d", yr, mo, da, hr, mn, sc);
						SetDateTime(&buf[0]);

						// Set time changed flag
						SetVariableThreadSafe("TIMEFL", "1");
						SaveDevParamsToNVMThreadSafe();

						// No response for the command
						out = NULL;
					}
				}
			}

			// put out into send buffer
			if ((uint32_t)out != 0)
			{
				threadSafeQueueEnqueue(publishQueue, outtopic);
				threadSafeQueueEnqueue(publishQueue, out);
			}
		}
		else
		{
			cnomssg++;
			if (cnomssg > 3)
			{
				// Schedule ping
				pingScheduled = true;

				cnomssg = 0;
			}
		}

		osDelay(10);
	}

  /* USER CODE END mqttReceptionProc */
}

/* mqttSendProc function */
void mqttSendProc(void const * argument)
{
  /* USER CODE BEGIN mqttSendProc */

	/// This procedure do next:
	/// - Get output messages from publish queue
	/// - Send output messages
	/// - Reconnect when send fails

	char outtopic[64];
	char outbuf[MQTT_RESPONSE_BUF_SIZE];

	while (!initialized) osDelay(100);

  /* Infinite loop */
	for(;;)
	{
		while (!threadSafeQueueDequeue(publishQueue, outtopic, 64))
			osDelay(10);
		while (!threadSafeQueueDequeue(publishQueue, outbuf, 1024))
			osDelay(10);

		int rc = MqttPutMessageThreadSafe(hmqtt, outtopic, outbuf);

		// Reconnect if publish error
		if (rc < 1)
		{
			int n = ReconnectThreadSafe(&hmqtt);
			TraceThreadSafe(hmqtt, "RECONNECTED because of communication breakdown after %i tries!!!!", n);
		}

		osDelay(10);
	}

  /* USER CODE END mqttSendProc */
}

/* contextDaemonProc function */
void contextDaemonProc(void const * argument)
{
  /* USER CODE BEGIN contextDaemonProc */

	/// This procedure do next:
	/// - Periodically save device context to NVM

	while (!initialized) osDelay(100);

	/* Infinite loop */
	for(;;)
	{
		// Save the device context to NVM when changed (check performed inside function)
#ifdef DISP_THREAD_DEBUG
		TraceThreadSafe(hmqtt, "Start saving Device context to NVM");
#endif
		SaveDevParamsToNVMThreadSafe();
#ifdef DISP_THREAD_DEBUG
		TraceThreadSafe(hmqtt, "End saving Device context to NVM");
#endif

		osDelay(60000); // Granularity - 1 minute
	}

  /* USER CODE END contextDaemonProc */
}

/* schedulerTriggersProc function */
void schedulerTriggersProc(void const * argument)
{
  /* USER CODE BEGIN schedulerTriggersProc */

	/// This procedure do next:
	/// - Perform connection on startup
	/// - Reboot when scheduled
	/// - Ping when scheduled (and reconnect when ping fails)
	/// - Scheduler actions for EPD

	int i;
	long lastAbsMinuteCMC = 0;
	int today = 0; // Current day, used for sunrise/sunset calculation
#if defined(TIME_CORRECT_PERIODICALLY)
	int lastTimeCorrectHour = 0;
#endif

	// Initialize context and connect

	// Create the Variable's Context
	CreateContext();

	// Read the messages's metadata
	ReadMetadata("", "");

	// Initialize debug topic
	sprintf (topictr, "%s/CMC/DEBUG", GetVariable("MTPRT"));

#if defined(DEBUG_ENABLE_MQTT_QUEUE_PUB)
	sprintf (topicmqdebug, "%s/%s/MQTT_DEBUG", GetVariable("MTPRT"), GetVariable("ID"));
#endif

#ifdef HYBRID_M95_WLAN_CODE
	if (GetVariableThreadSafe("TIMEFL")[0] == '0')
	{
		transport_get_time();
	}

	transport_update_tz();
#endif

	ReconnectThreadSafe(&hmqtt);

#if defined(DEBUG_ENABLE_MQTT_QUEUE_PUB)
	// Update debug topic - ID could be changed after connection (IMEI case)
	sprintf (topicmqdebug, "%s/%s/MQTT_DEBUG", GetVariable("MTPRT"), GetVariable("ID"));
#endif

	schedulerTaskLastTS = HAL_GetTick();
	mqttRecvTaskLastTS = HAL_GetTick();

	initialized = true;

#if defined(TIME_CORRECT_PERIODICALLY)
	lastTimeCorrectHour = atoi(getHour());
#endif

    /* Infinite loop */
	for(;;)
	{
		schedulerTaskLastTS = HAL_GetTick();

		// Check for Reboot scheduled
		if (rebootScheduled && (GetTimeStamp() >= rebootAtTimestamp))
		{
			SaveDevParamsToNVMThreadSafe();
			TraceThreadSafe(hmqtt, "Reboot!");
			//rebootScheduled = false;
			taskENTER_CRITICAL();
			NVIC_SystemReset();
		}

		// Check for ping scheduled
		if (pingScheduled)
		{
			// Send Ping request
			int rc = MqttPingThreadSafe(hmqtt);

			if (rc < 1)
			{
				//  It seems we are'nt receiving anymore
				TraceThreadSafe(hmqtt, "Sended Ping without getting PINGRESP (deafness?)");

				// Reconnect
				TraceThreadSafe(hmqtt, "Ready  to reconnexion or reboot!!! ");
				int ntries = ReconnectThreadSafe(&hmqtt);
				TraceThreadSafe(hmqtt, "Hearing recovered after %d tries!!", ntries);
			}
			else
			{
				pingScheduled = false;
				TraceThreadSafe(hmqtt, "Sended Ping getting %d", rc);
			}
		}

		// Check for reconnect scheduled
		if (reconnectScheduled)
		{
			SaveDevParamsToNVMThreadSafe();

			char *host = GetVariableThreadSafe("MURI");
			unsigned int port = atoi(GetVariableThreadSafe("MPORT"));

			TraceThreadSafe(hmqtt,
				"Go to DISCONNECT from this server and connect to %s:%d.\nBye",
					host, port);

			ReconnectThreadSafe(&hmqtt);

			TraceThreadSafe(hmqtt, "This has been a RECONNECTION!!!");

			reconnectScheduled = false;
		}

		// Blink once a second with green RGB if connected
		if (hmqtt > 0 && !processing) RGB_Color_Blink(RGB_COLOR_GREEN);

		osDelay(1000); // 1 second granularity for ping

		// Each minute perform scheduler tasks

		// Get now hour and minute
		int nowHour = atoi(getHour());
		int nowMinute = atoi(getMinute());

#if defined(TIME_CORRECT_PERIODICALLY)
		if (lastTimeCorrectHour != nowHour &&
				(nowHour % TIME_CORRECT_PERIOD_HOURS) == 0)
		{
			lastTimeCorrectHour = nowHour;

			if (GetVariableThreadSafe("TIMEFL")[0] == '0')
			{
				// Performing reboot because for time correcting we need to
				// Disconnect from MQTT - connect to TIME - get time - disconnect from TIME - connect to MQTT
				// And this is the same as reboot
				REBOOT();
			}
		}
#endif

		int nowAbsMinuteCMC = nowMinute + 60 * nowHour;

		if (nowAbsMinuteCMC != lastAbsMinuteCMC)
		{
			lastAbsMinuteCMC = nowAbsMinuteCMC;

			// Sunrise/Sunset calculation once a day
			int nowDay = 0, nowMonth = 0, nowYear = 0;
			getDate(&nowDay, &nowMonth, &nowYear);
			if (today != nowDay)
			{
				today = nowDay;

				// Get coordinates
				ContextLock();
				double lat = atof(GetVariable("GPSLAT"));
				double lon = atof(GetVariable("GPSLON"));
				ContextUnlock();

				// Calculate SR/SS
				int srHour = 6, srMin = 0, ssHour = 21, ssMin = 0;
				CalculateSunRiseSetTimes(lat, lon,
						nowYear, nowMonth, nowDay,
						&srHour, &srMin, &ssHour, &ssMin);

				char buf[8];

				// Set SR/SS variables
				ContextLock();

				sprintf(buf, "%i", srHour);
				SetVariable("GPSSRH", buf);
				sprintf(buf, "%i", srMin);
				SetVariable("GPSSRM", buf);
				sprintf(buf, "%i", ssHour);
				SetVariable("GPSSSH", buf);
				sprintf(buf, "%i", ssMin);
				SetVariable("GPSSSM", buf);

				// Once a day decrement relay periodic NTIMES
				for (i = 1; i < 7; i++)
				{
					char buf[16];
					sprintf(buf, "CMREPR%iN", i);
					int ntimes = atoi(GetVariable(buf));
					if (ntimes > 0)
					{
						ntimes--;
						char valbuf[16];
						sprintf(valbuf, "%i", ntimes);
						SetVariable(buf, valbuf);
					}
				}

				ContextUnlock();
			}
		}

#if defined(DEBUG_ENABLE_MQTT_QUEUE_PUB)
		// Each DEBUG_MQTT_QUEUE_PUB_PERIOD sec publish info about MQTT queues
		if (GetTimeStamp() % DEBUG_MQTT_QUEUE_PUB_PERIOD == 0)
		{
			size_t sendSize = threadSafeQueueGetSize(publishQueue);
			TraceThreadSafeTopic(hmqtt, topicmqdebug,
					"MQTT send queue size %lu strings, %lu messages",
					sendSize, (sendSize/2));
			size_t recvSize = wlanGetAvailableDataMqttPub();
			TraceThreadSafeTopic(hmqtt, topicmqdebug,
					"MQTT recv queue size %lu bytes", recvSize);
			size_t ctrlSize = wlanGetAvailableDataMqttCtrl();
			TraceThreadSafeTopic(hmqtt, topicmqdebug,
					"MQTT ctrl queue size %lu bytes", ctrlSize);
		}
#endif

	}

  /* USER CODE END schedulerTriggersProc */
}

/* USER CODE BEGIN Application */

int NEWCONN()
{
	reconnectScheduled = true;
	return 1;
}

int REBOOT()
{
	rebootAtTimestamp = GetTimeStamp() + 5; // restart in 5 seconds
	rebootScheduled = true;
	return 1;
}

uint32_t tsDiff(uint32_t now, uint32_t prev)
{
	if (now >= prev)
	{
		return (now - prev);
	}
	else
	{
		return (UINT32_MAX - prev + now);
	}
}

void iwdgRefresherProc(void const * argument)
{
	for(;;)
	{
		HAL_IWDG_Refresh(&hiwdg);
		osDelay(1000);

		if (initialized)
		{
			// one minute have 60 000 miliseconds
			if ((tsDiff(HAL_GetTick(), mqttRecvTaskLastTS)/60000) > TASK_WAIT_MAX_MINUTES)
			{
				NVIC_SystemReset();
			}
			if ((tsDiff(HAL_GetTick(), schedulerTaskLastTS)/60000) > TASK_WAIT_MAX_MINUTES)
			{
				NVIC_SystemReset();
			}
		}
	}
}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
