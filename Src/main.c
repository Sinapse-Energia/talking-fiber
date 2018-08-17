
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

#include "GPRS_transport.h"
#include "Definitions.h"
#include "southbound_ec.h"
#include "MQTTAPI.H"
#include "NBinterface.h"
#include "BLInterface.h"
#include "Application.h"
#include "circular.h"
#include "utils.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

// has to be moved to a utilities header when it should be
extern 	int		tprintf(int hcon, char *texto,...);

extern DMA_HandleTypeDef hdma_usart6_rx;

typedef enum
{
	Board1 = 1,  // Square..
	Board2 = 2,	 // Orange...
	Board0 = 3,	 // Classic...
	TalkingFiberBoard = 4,
} BoardType;

//UART_HandleTypeDef huartDummy;
// It is defined a variable but it is not going to be used.

#define GPRS_UART huart6

DEV_HANDLER		Device_Init	(BoardType);
void			Device_Reset(BoardType);
void			*FACTORY	(BoardType board, DEV_HANDLER hconn, st_CB *cb);
int				NEWCONN();

//  int elapsed10secondsAux=0;
  uint16_t elapsed10seconds=0; 					// At beginning this is 0
//  uint8_t LOG_ACTIVATED=0;				 		/// Enable to 1 if you want to show log through logUART
//  uint8_t LOG_GPRS=0;  							/// For showing only GPRS information
  uint8_t timeoutGPRS=0; 						/// At beginning this is 0
//  uint32_t timeout=1000;				 		/// Timeout between AT command sending is 1000 milliseconds.
//  uint8_t rebootSystem=0;						 /// At beginning this is 0
  uint8_t nTimesMaximumFail_GPRS=2; 			/// For initial loop of initializing GPRS device
//  uint8_t retriesGPRS=1; 						/// only one retries per AT command if something goes wrong
//  uint8_t existDNS=1; 							/// The IP of main server to connect is not 4 number separated by dot. It is a DNS.
//  uint8_t offsetLocalHour=0; 					/// for getting UTC time
//  uint8_t openFastConnection=0;      			 /// by default to 0, it is used for doing a quick connection when it is needed to call the connect function again
//  uint8_t setTransparentConnection=1;  			/// 1 for transparent connection, 0 for non-transparent. Then all data flow is command AT+ data
  uint8_t dataByteBufferIRQ;  					/// Last received byte from GPRS
//  uint8_t dataByteBufferETH;  					/// Last received byte from GPRS
//  //uint16_t GPRSBufferReceivedBytes;     		/// Number of received data from GPRS after a cleanningReceptionBuffer() is called
//  //uint16_t indexGPRSBufferReceived;
//  //uint16_t indexPickingReceivedBytes=0;
//  uint8_t connected=0;
//  //unsigned char buffer2[SIZE_GPRS_BUFFER];
//  int32_t quantityReceived=0;
  RTC_TimeTypeDef structTimeRTC;
  RTC_DateTypeDef structDateRTC;

//extern  unsigned char WDT_ENABLED;						 /// Enable for activate independent watch dog timer

void  Device_full_reset(void);

//extern int	COMM_Init();

char	*topicpub = 0;
char	*topicsub = 0;
char	*topicbrcast = 0;
char	*topictr = 0;

//UART_HandleTypeDef *gprs_uart;

int		bydma = 1;


int		nirqs = 0;

int		hmqtt = 0; 			// handle to the mqtt connection. Has to be a SIGNED integer

int	timepolling = TIMEPOLLING;
int	timeping = TIMEPING;

//int		connections = 0;  // this counter gives us a hint for distinguish reboot/connection

// shoots to get connection times
uint32_t	tc0, //	start connection
			tc1, // after open IP
			tc2, // broker connected
			tc3;


st_CB *DataBuffer;

BoardType	BaseBoard;


int		conti = 0; // provisional

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/////////////////////////////////////////////////////////////////////////////////////////////
//	Open connection (from scratch OR over a established connection)
//		if mode param is 0, open from scratch, else reopen on new host and/or credentials
//
//	The function doesn't take any parameters (get all of then from the Context)
//	Makes all calling the transport and broker functions
//
//    First: try to connect using the 'default' broker parameters
//              (if succeeds, stores those default values as 'last successful connection'
//    If that fails, makes a second try with the 'last successful connection' (contingency)
/////////////////////////////////////////////////////////////////////////////////////////////
int	OpenConnection(){

	int		hconn;
//	int 	try = 0; // 1 default, 2 contingency
//	char 	*kind = "";
	int 	rc;





	// First, try with default broker parameters
	char	*h = GetVariable("IP");
	unsigned int p = atoi(GetVariable("PORT"));
	int	s = atoi(GetVariable("SEC"));
	char	*us = GetVariable("USER");
	char	*pw = GetVariable("PSWD");
	char	*apn = NULL;
#if defined (GPRS_TRANSPORT)
	apn = GetVariable("APN");
#endif

	tc0 = HAL_GetTick();

	hconn = transport_open(h, p, s, apn);

 	if (hconn > 0) {
		tc1 = HAL_GetTick();
 		rc = MqttConnectB(hconn, us, pw);
 		if (rc > 0){
//			kind = "DEFAULT";
 			conti = 0;
 		}
 		else {
			// proactive ..
 			MqttDisconnectB(hconn);
 			transport_close(hconn);
 			hconn = 0;
 		}
 	}
	if (hconn <= 0)  {

		// try with Contingency
 		h = GetVariable("LIP");
 		p = atoi(GetVariable("LPORT"));
 		s = atoi(GetVariable("LSEC"));
 		us = GetVariable("LUSER");
 		pw = GetVariable("LPSWD");
 		apn = GetVariable("LAPN");
 		if (h && p && apn) {
			hconn = transport_open(h, p, s, apn);
			if (hconn > 0 ) {
				tc1 = HAL_GetTick();
				rc = MqttConnectB(hconn, us, pw);
				if (rc > 0){
	//	 			kind = "CONTINGENCY";
					conti = 1;
				}
				else {
					transport_close(hconn);
					hconn = 0;
				}
			}
 		}
	}
	if (hconn > 0) {
		// Update Last success connection vars
		char txt1[8];
		char txt2[8];
		SetVariable("LIP", h);
		itoa(p, txt1, 10);
		SetVariable("LPORT", txt1);
		SetVariable("LUSER", us);
		SetVariable("LPSWD", pw);
		itoa(s, txt2, 10);
		SetVariable("LSEC", txt2);
		SetVariable("LAPN", apn);




		tc2 = HAL_GetTick();

		SaveConnParams(); // SAVE ALL (but only LAST CONNECT GROUP need to be stored)

//		char *id = GetVariable("ID");
//		char *myip = GetVariable("LOCALIP");
//		char *sim = GetVariable("IDSIM");

//		tprintf (hconn, "%s %s (%s, %s by %s) to %s:%d as IP %s and SIM %s.", id, mode?"RECONNECTED":"CONNECT", kind, (s)?"TLS":"TCP", (bydma?"DMA":"IRQ"), h, p, myip, sim);
//		tprintf (hconn, "Times elapsed %ld, %ld, %ld (%ld + %ld = %ld)", t0, t1, t2 , (t1-t0), (t2-t1), (t2-t0));
	}
	return hconn;
}


 int	COMM_Init(){

	 Color(COL_CONNECTING);

 	 int hconn = OpenConnection();
// 	 SetVariable("ID", id0);
 	 if (hconn > 0){
		int rc;

		char *id = GetVariable("ID");
		char *host = GetVariable("LIP");
		char *port = GetVariable("LPORT");
//		char *apn = GetVariable("LAPN");
		char transport_spec[80] = "<transport specific info>";
		/**
		char *sim = GetVariable("IDSIM");
		char *imei = GetVariable("IMEI");
		char *myip = GetVariable("LOCALIP");
		**/

		// allocate the topic strings.
		// Test before to do it only once!!!
		if (!topicpub){
			topicpub = malloc (strlen(topicroot) + 1 + strlen(topicpub0) + 1);
			sprintf (topicpub, "%s/%s", topicroot, topicpub0);
		}
		if (!topicsub){
			topicsub= malloc (strlen(topicroot) + 1 + strlen (topicsub0) + 1 + strlen(id) + 1);
			sprintf (topicsub, "%s/%s/%s", topicroot, topicsub0, id);
		}
		if (!topicbrcast){
			topicbrcast = malloc (strlen(topicroot) + 1 + strlen (topicsub0) + 1 + strlen (broadcast) + 1);
			sprintf (topicbrcast, "%s/%s/%s", topicroot, topicsub0, broadcast);
		}
		if (!topictr){
			topictr = malloc (strlen(topicroot) + 1 + strlen (topictr0) + 1 + strlen (id) + 1);
			sprintf (topictr, "%s/%s/%s", topicroot, topictr0, id);
		}

		//		tprintf (hconn, "%s %s (%s, %s by %s) to %s:%d as IP %s and SIM %s.", id, mode?"RECONNECTED":"CONNECT", kind, (s)?"TLS":"TCP", (bydma?"DMA":"IRQ"), h, p, myip, sim);
		//		tprintf (hconn, "Times elapsed %ld, %ld, %ld (%ld + %ld = %ld)", t0, t1, t2 , (t1-t0), (t2-t1), (t2-t0));
		//		tprintf (hconn, "%s CONNECTED (%s %s) to %s:%s by %s as IP %s, IMEI %s and SIM %s.\n", id, conti?"CONTINGENCY":"DEFAULT", (bydma?"DMA":"IRQ"), host, port, apn, myip, imei, sim);
		tprintf (hconn, "%s CONNECTED (%s %s) to %s:%s  %s.\n", id, conti?"CONTINGENCY":"DEFAULT", (bydma?"DMA":"IRQ"), host, port, transport_spec);

		do {
			 rc = MqttSubscribe(hconn, topicsub) && MqttSubscribe(hconn, topicbrcast);
		 } while (rc < 1);
		 tprintf (hconn, "Successfully subscripted to #%s#, #%s#", topicsub, topicbrcast);
 	 }
	 return hconn;
}

#define	MAX_RESETS 1
int	FULLNEWCONN(){
	int hrec;
	int ntries= 0;
	int nreset = 0;
	do {
		if (ntries >= MAX_CONNECT_TRIES) {
			if (nreset >= MAX_RESETS)
				Device_full_reset();
			else {
				Device_Reset(BaseBoard);
				nreset++;
				ntries = 0;
			}
		}
		hrec = NEWCONN();
		ntries++;
	} while (hrec <= 0);
	tprintf (hmqtt, "Reconnection Times (%ld TCP + %ld MQTT = %ld TOTAL)\n", (tc1-tc0), (tc2-tc1), (tc2-tc0));

	hmqtt = hrec;
	return hmqtt;
}


int	NEWCONN(){
	int rc;
	int	hconn;
	Color(COL_CONNECTING);

	if (1)
		SaveConnParams(); // (maybe a is for 5;) Save (but only DEFAULT GROUP need to be stored)
						  //  to be improved in a future

#ifdef DEBUG
	tprintf (hmqtt, "Go to %s DISCONNECT from this server and connect to %s:%s.\nBye", "FULL", GetVariable("IP"), GetVariable("PORT"));
		HAL_Delay(2000); // give trace time to arrive before disconnect
#endif
	// This performs FULL disconnection
	rc = MqttDisconnectB(hmqtt);

//	if (modo == 0)
//		transport_close(hmqtt);

	hconn = OpenConnection();

	if (hconn > 0){
		char *id = GetVariable("ID");
		char *host = GetVariable("LIP");
		char *port = GetVariable("LPORT");
		char *apn = GetVariable("LAPN");
		char *myip = GetVariable("LOCALIP");

		tprintf (hconn, "%s %s RECONNECT (%s %s) to %s:%s by %s as IP %s .\n", id, "FULL", conti?"CONTINGENCY":"DEFAULT", (bydma?"DMA":"IRQ"), host, port, apn, myip);

		 do {
			 rc = MqttSubscribe(hconn, topicsub) && MqttSubscribe(hconn, topicbrcast);
		 } while (rc < 1);
		 tprintf (hconn, "Successfully subscripted to #%s#, #%s#", topicsub, topicbrcast);
	}
	return hconn;
}

int EveryDayActions(){
//	SetProgramm();
//	SetPeriodics();
	SetTFPeriodic();
	return 1;
}


int daily_rearm(void) {
	EveryDayActions();
	return 1;
}

// Palceholder function to investigate the board type
BoardType getBoard() {
	return DEFAULTBOARD;
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

	int		ntries = 0;		// Counter of the number of attempts to connect
	int 	nreset = 0;		// Counter of the number of soft-resets
	char	strint[8];		// string to convert integers to text
	DEV_HANDLER		hdevice = NULL;
	char	sintegers[8];

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

	// Create de Variable's Context
	CreateContext();

	RecConnParams();

	BaseBoard  = atoi(GetVariable("BOARD"));

	if (BaseBoard == 0){
		BaseBoard = getBoard();
		itoa (BaseBoard, sintegers, 10);
		SetVariable("BOARD", sintegers);
	}

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_SDIO_SD_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART6_UART_Init();
  MX_RTC_Init();
//  MX_IWDG_Init();
  MX_FATFS_Init();
  MX_TIM7_Init();
  /* USER CODE BEGIN 2 */

  Color(COL_OFFLINE);
  GetRTC(&hrtc);

  HAL_TIM_Base_Start_IT(&htim7); 	//Activate IRQ for Timer7
	if (WDT_ENABLED == 1) {
		MX_IWDG_Init();
		__HAL_IWDG_START(&hiwdg);
		HAL_IWDG_Refresh(&hiwdg);
	}

	  HAL_GPIO_WritePin(EX_ENABLE_GPRS_BATTERY_GPIO_Port, EX_ENABLE_GPRS_BATTERY_Pin, GPIO_PIN_SET);


#if defined (GPRS_TRANSPORT)
//		uint32_t ta, tb;

		// (1)  Circular buffer allocation
		if (bydma) { // BYDMA
				DataBuffer	= CircularBuffer (256, &hdma_usart6_rx);
				MX_DMA_Init();					// set DMA clock and priorities
				HAL_UART_DMAStop(&huart6);
		}
		else {
			DataBuffer	= CircularBuffer (256, NULL);
		}

		// (2) Gets the initialized device handler
		do {
			hdevice = Device_Init(BaseBoard);
		} while (!hdevice);


		// (3) Interrupt enable
		if (bydma) {  // BYDMA
			int tries = 0;
			HAL_StatusTypeDef rc;
			do {
				rc = HAL_UART_Receive_DMA(&huart6, DataBuffer->buffer, DataBuffer->size); // starts DMA reception
				HAL_Delay(200);
				tries++;
			} while  (rc != HAL_OK);
		}
		else {
			HAL_UART_Receive_IT(&huart6, &dataByteBufferIRQ, 1); // Enabling IRQ
		}

		// (4) C Language wrapper to the Modem Abstract Factory
		do {
			gtransceiver = FACTORY(BaseBoard, hdevice, DataBuffer);
		} while (!gtransceiver);
#endif

#ifdef DEBUG
	////  Prevalence of locale's connection parameters, if there are
	if (host){
			SetVariable("IP",host);
		if (port){
			char	sport[8];
			itoa(port, sport, 10);
			SetVariable("PORT", sport);
		}
		if (user)
			SetVariable( "USER",user);
		if (password)
			SetVariable("PSWD", password);

		char ssec[8];
		itoa(security, ssec, 10);
		SetVariable("SEC", ssec);

#if defined(GPRS_TRANSPORT)
		if (const_APN)
			SetVariable("APN", const_APN);
#endif
	}
#if defined(BOOTLOADER_DBG)
		if (updfw_protocol)
			SetVariable ("UPDFW_PROTO", updfw_protocol);

		if (updfw_server)
			SetVariable ("UPDFW_HOST", updfw_server);

		if (updfw_route)
			SetVariable("UPDFW_ROUTE", updfw_route);

		if (updfw_name)
			SetVariable("UPDFW_NAME", updfw_name);

		if (updfw_user)
			SetVariable("UPDFW_USER", updfw_user);

		if (updfw_password)
			SetVariable("UPDFW_PSWD", updfw_password);


		if (gpio_status)
			SetVariable ("GPIO", gpio_status);


		itoa(PWM_status, strint, 10);
		SetVariable("PWM", strint);

		itoa(updfw_port, strint, 10);
		SetVariable("UPDFW_PORT", strint);

	}


#endif
#endif

	// Set the version compilation signature
	if (fw_version)
		SetVariable("FWVERSION",(char*)fw_version);

	itoa(update_firmware, strint, 10);
	SetVariable("UPDFW", strint);

	itoa(update_firmware_counter, strint, 10);
	SetVariable("UPDFW_COUNT", strint);

	// Read the messages's metadata
	ReadMetadata("", "");



	do {
		if (ntries >= MAX_CONNECT_TRIES) {
			if (nreset >= MAX_RESETS)
				Device_full_reset();
			else {
				Device_Reset(BaseBoard);
				nreset++;
				ntries = 0;
			}
		}
		hmqtt = COMM_Init();
		ntries++;
	} while (hmqtt <= 0);

	tprintf (hmqtt, "Connection Times (INIT %ld + %ld TCP + %ld MQTT = %ld TOTAL)\n", tc0, (tc1-tc0), (tc2-tc1), (tc2));

	TestSynchonization ();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	if (!hmqtt) NVIC_SystemReset();

			extern char	pretrbuf[];
			long lastget = GetTimeStamp();

			int rc = 1;
//			int cnomssg = 0;
			int gpsstatus = atoi(GetVariable("GPS"));
			if (gpsstatus)
				Color(COL_CONNECTEDGPS);
			else
				Color(COL_CONNECTED);

			EveryDayActions();

		//	DoAt(daily_rearm, 18, 18, 59);
			DoAt(daily_rearm, 24, 00, 00);
#ifdef DEBUG
//			tprintf (hmqtt, "Modem takes %ld msec to init", (tb -ta));
			rc = tprintf (hmqtt, "Go to the loop after %d tries(%s)!!", ntries, strDateTime());
			if (strlen(pretrbuf)){
				tprintf (hmqtt, "PRE is %s", pretrbuf);
			}

#endif
			 /* Main loop only if we are connected and subscribed.
				Inside loop one Ping packet must be sent in order to manage the keep alive
				before expiration time of client elapses. (here, the expiration time is  'data.keepAliveInterval = 300', 300 seconds.
			 */

			RecAppParams();

			while (1) {
				if (atoi(GetVariable("GPS")))
					Color(COL_CONNECTEDGPS);
				else
					Color(COL_CONNECTED);

				Blink();
				if  (rc > 0) {
					char 	buffer[256];
					char *mssg = NULL;
					if (WDT_ENABLED==1) HAL_IWDG_Refresh(&hiwdg);
//						tprintf (hmqtt, "Going to Poll for incoming commands...!!");

						mssg = MqttGetMessage(hmqtt, buffer, 256);

						if (mssg) {
								int previous = getcolor();
								Color(CYAN);
//								cnomssg = 0;
								lastget = GetTimeStamp();
#ifdef DEBUG
								tprintf (hmqtt , "Incoming message #%s#", mssg);
#endif
								char *out = ProcessMessage (mssg);
								if (out) {
									// tprintf (hmqtt , "Message replay is:\n%s", out);
									rc = MqttPutMessage(hmqtt, topicpub, out);
//									tprintf (hmqtt , "Reply %s published into %s", out, topicpub);

								}
								Color(previous);
						}
						else { // NO MSSG
#ifdef DEBUG
						//		tprintf (hmqtt, "No mssg in queue");
#endif
								Schedule();
								if (GetTimeStamp() - lastget > timeping){
									lastget = GetTimeStamp();
//									tprintf (hmqtt, "BEFORE sending Ping...!!");
									rc = MqttPing(hmqtt);
//									tprintf (hmqtt, "AFTER sending Ping...(%d)!!", nirqs);
									if (rc < 1)  // second try
										rc = MqttPing(hmqtt);
									if (rc < 1) {
										//  It seems we are'nt receiving anymore
#ifdef DEBUG
										tprintf (hmqtt, "Sended Ping  getting %d instead of PINGRESP (%ld bytes)", rc, getHSsize());

										rc = MqttPing(hmqtt);
										rc = 1;
#endif

										if (1) {
#ifdef DEBUG
											tprintf (hmqtt, "Ready  to reconnection or reboot!!! ");
#endif

											ntries = 0;
											do {
												hmqtt = FULLNEWCONN();
												ntries++;
											} while (hmqtt <= 0);
#ifdef DEBUG
											rc = tprintf (hmqtt, "earing recovered after %d tries!!", ntries);
#endif
										}
									}
									else {
#ifdef DEBUG
										rc = tprintf (hmqtt, "Sended Ping getting %d (%s %ld bytes)", rc, strDateTime(), getHSsize ());
#endif
									}
//									cnomssg = 0;
								}

						}
			      } //
			      else {
#ifdef DEBUG
			    	  tprintf (hmqtt, "RC invalido %d", rc);
#endif
			    	  ntries = 0;
			    	  do {
			    			hmqtt = COMM_Init();
			    			ntries++;
			    	  } while (hmqtt <= 0);
#ifdef DEBUG
			    	  tprintf (hmqtt, "RECONNECTED because of communication breakdown!!!!");
#endif
			      }

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
DEV_HANDLER Device_Init(BoardType board) {
	DEV_HANDLER result = NULL;
	if (board == Board0 || board == TalkingFiberBoard){
#if defined(BUILD_M95) || defined(BUILD_BG96)
		result = DeviceGPRS_Init();
#endif
	}
	else if ((board == Board1) || (board == Board2)){
#if defined(BUILD_RM08)
		result = DeviceEth_Init();
#endif
	}
	else
		result = NULL;
	return result;
}

void Device_Reset(BoardType board) {
	if (board == Board0 || board == TalkingFiberBoard){
#if defined(BUILD_M95) || defined(BUILD_BG96)
		Common_Reset();
#endif
	}
	else {
#if defined(BUILD_RM08)
		DeviceEth_Reset();
#endif
	}
}

void	*FACTORY(BoardType board, DEV_HANDLER hconn, st_CB *cb){
	if (board == Board0 || board == TalkingFiberBoard){
#if defined(BUILD_M95) || defined(BUILD_BG96)
		return MODEMFACTORY(hconn, cb);
#endif
	}
	else {
#if defined(BUILD_RM08)
		return ETHFACTORY(hconn, cb);
#endif
	}
	return NULL;
}

// Function to order a full restart
void Device_full_reset(void) {
	// As verified, NVIC_SystemReset works by itself, so there is no need to change WDT
	NVIC_SystemReset();
}

void	EnableRXGPRG(){
	if(!bydma)
		HAL_UART_Receive_IT(&huart6, &dataByteBufferIRQ, 1); // Enabling IRQ
}


int	Publish(char *message) {
	int rc = MqttPutMessage(hmqtt, topicpub, message);
	return rc;
}

int	Log(char *message) {
	int rc = MqttPutMessage(hmqtt, topictr, message);
	return rc;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == GPRS_UART.Instance) {
		if (!bydma) { // only if not BYDMA
			nirqs++;
			allnew += Write(DataBuffer, dataByteBufferIRQ);
			if (IsFull(DataBuffer)) {
				DataBuffer->overruns++;
			}
			balnew++;
			HAL_UART_Receive_IT(huart, &dataByteBufferIRQ, 1);
		}
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	HAL_NVIC_DisableIRQ(TIM7_IRQn);
	if (htim->Instance==TIM7)
	{
	  	AddSeconds(10);
        elapsed10seconds++;
		if (elapsed10seconds%TIMING_TIMEOUT_GPRS == 0)
		{
			/// Tiempo timeoutGPRS
			timeoutGPRS=1;
		}
	}
	HAL_NVIC_EnableIRQ(TIM7_IRQn);
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
  /* User can add his own implementation to report the HAL error return state
  while(1)
  {
  }
  */
	tprintf (hmqtt, "Error from line %d in file %s", line, file);
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
