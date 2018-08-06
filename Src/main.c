/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>  // provisional atoi()
#include <stdarg.h>
#include <time.h>  // provionale time()

#include "main.h"
#include "stm32f2xx_hal.h"
#include "GPRS_transport.h"
#include "Definitions.h"
#include "southbound_ec.h"
#include "MQTTAPI.H"
#include "NBinterface.h"
#include "BLInterface.h"
#include "Application.h"



#include "circular.h"
#include "utils.h"

#include "dma.h"		// BYDMA
#include "CAN_Util.h"

// has to be moved to a utilities header when it should be
extern 	int		tprintf(int hcon, char *texto,...);

typedef enum
{

	Board1 = 1,  // Square..
	Board2 = 2,	 // Orange...
	Board0 = 3,	 // Classic...
} BoardType;


TIM_HandleTypeDef    TimHandle;
TIM_OC_InitTypeDef sConfig;
RTC_HandleTypeDef hrtc;

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

ADC_HandleTypeDef hadc1;

IWDG_HandleTypeDef hiwdg;

TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim7;
TIM_OC_InitTypeDef sConfig;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart4;

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart6;
DMA_HandleTypeDef hdma_usart6_rx;

UART_HandleTypeDef huartDummy;
/// It is defined a variable but it is not going to be used.

#define GPRS_UART huart6

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config		(BoardType boardtype);
static void MX_GPIO_Init	(BoardType boardtype);
static void MX_ADC1_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM7_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART6_UART_Init(void);
static void MX_UART4_UART_Init(void);
static void MX_IWDG_Init(void);
static void MX_RTC_Init(void);


void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);


extern	DEV_HANDLER		Device_Init	(BoardType);
extern	void			Device_Reset(BoardType);
extern 	void			*FACTORY	(BoardType board, DEV_HANDLER hconn, st_CB *cb);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
  int elapsed10secondsAux=0;
  uint16_t elapsed10seconds=0; 					// At beginning this is 0
  uint8_t LOG_ACTIVATED=0;				 		/// Enable to 1 if you want to show log through logUART
  uint8_t LOG_GPRS=0;  							/// For showing only GPRS information
  uint8_t timeoutGPRS=0; 						/// At beginning this is 0
  uint32_t timeout=1000;				 		/// Timeout between AT command sending is 1000 milliseconds.
  uint8_t rebootSystem=0;						 /// At beginning this is 0
  uint8_t nTimesMaximumFail_GPRS=2; 			/// For initial loop of initializing GPRS device
  uint8_t retriesGPRS=1; 						/// only one retries per AT command if something goes wrong
  uint8_t existDNS=1; 							/// The IP of main server to connect is not 4 number separated by dot. It is a DNS.
  uint8_t offsetLocalHour=0; 					/// for getting UTC time
//  uint8_t APN[SIZE_APN]; 						/// Array where is saved the provider APN (format as example in Definitions.h)
//  uint8_t IPPORT[SIZE_MAIN_SERVER]; 			/// Array where is saved main destination server to connect (IP and PORT format as example in Definitions.h)
//  uint8_t SERVER_NTP[SIZE_NTP_SERVER]; 			/// Array where is saved server NTP to get UTC time (format as example in Definitions.h)
//  uint8_t calendar[10];               			/// Array for saving all calendar parameters get from NTC server
//  uint8_t idSIM[30];                 			 /// Array where is saved ID from SIMcard
  uint8_t openFastConnection=0;      			 /// by default to 0, it is used for doing a quick connection when it is needed to call the connect function again
  uint8_t setTransparentConnection=1;  			/// 1 for transparent connection, 0 for non-transparent. Then all data flow is command AT+ data
//  uint8_t GPRSbuffer[SIZE_GPRS_BUFFER];			 /// received buffer with data from GPRS
  uint8_t dataByteBufferIRQ;  					/// Last received byte from GPRS
  uint8_t dataByteBufferETH;  					/// Last received byte from GPRS
  //uint16_t GPRSBufferReceivedBytes;     		/// Number of received data from GPRS after a cleanningReceptionBuffer() is called
  //uint16_t indexGPRSBufferReceived;
  //uint16_t indexPickingReceivedBytes=0;
  uint8_t connected=0;
  //unsigned char buffer2[SIZE_GPRS_BUFFER];
  int32_t quantityReceived=0;
  RTC_TimeTypeDef structTimeRTC;
  RTC_DateTypeDef structDateRTC;
/* USER CODE END 0 */


extern  unsigned char WDT_ENABLED;						 /// Enable for activate independent watch dog timer


extern void  Device_full_reset(void);


extern int	COMM_Init();

char	*topicpub = 0;
char	*topicsub = 0;
char	*topicbrcast = 0;
char	*topictr = 0;


UART_HandleTypeDef *eth_uart;
UART_HandleTypeDef *gprs_uart;



int		bydma = 1;


int		nirqs = 0;

int		hmqtt = 0; 			// handle to the mqtt connection. Has to be a SIGNED integer

int	timepolling = TIMEPOLLING;
int	timeping = TIMEPING;


int		connections = 0;  // this counter gives us a hint for distinguish reboot/connection

// shoots to get connection times
uint32_t	tc0, //	start connection
			tc1, // after open IP
			tc2, // broker connected
			tc3;


st_CB *DataBuffer;
st_CB *DataBufferEth;

BoardType	BaseBoard;


int		conti = 0; // provisional
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
	int 	try = 0; // 1 default, 2 contingency
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
		char *apn = GetVariable("LAPN");
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
	SetProgramm();
	SetPeriodics();
}


int daily_rearm(void) {
	EveryDayActions();
}


int main0(){
	return main2();
}


int main2(void) {

	int	x = 1;
	int y = 2;
	x = x +1;
	x = x +1;
	x = x +1;
	x = x +1;
	x = x +1;
	x = x +1;
	x = x +1;
	x = x +1;
	x = x +1;
	y = y + x;
	y = y + x;
	y = y + x;
	y = y + x;
//	HAL_Init();
//	HAL_Delay(3000);
	return main0();
}
/**/

// Palceholder function to investigate the board type
BoardType getBoard() {
	return DEFAULTBOARD;
}

int main(void) {
	int		ntries = 0;		// Counter of the number of attempts to connect
	int 	nreset = 0;		// Counter of the number of soft-resets
	int		lomio = 1;
	char	strint[8];		// string to convert integers to text
	DEV_HANDLER		hdevice = NULL;
	uint8_t dataFromFlash[128];
	int 	okFlash=0;
	char	sintegers[8];

	int	a = 1;
	int b = 2;


	a = a +1;
	a = a +1;
	a = a +1;
	a = a +1;
	a = a +1;
	a = a +1;
	a = a +1;
	a = a +1;
	a = a +1;
	b = b + a;
	b = b + a;
	b = b + a;
	b = b + a;



	/* MCU Configuration----------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	// Create de Variable's Context
	CreateContext();

	RecConnParams();

	BaseBoard  = atoi(GetVariable("BOARD"));

	if (BaseBoard == 0){
		BaseBoard = getBoard();
		itoa (BaseBoard, sintegers, 10);
		SetVariable("BOARD", sintegers);
	}


	/* Configure the system clock */
	SystemClock_Config(BaseBoard);


	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init(BaseBoard);

	Color(COL_OFFLINE);
//	HAL_Delay(3000);

	MX_ADC1_Init();
	//MX_TIM3_Init();// francis
	MX_TIM7_Init();

	MX_RTC_Init();
	GetRTC (&hrtc);

	 



	if (BaseBoard == Board0)  {
#if defined(BUILD_M95) || defined(BUILD_BG96)
		MX_USART6_UART_Init();
#endif
	}
	else {
#if defined(BUILD_RM08)
		if (BaseBoard == Board2){
			MX_UART4_UART_Init();
			eth_uart = &huart4;
		}
		else if (BaseBoard == Board1)  {
			MX_USART1_UART_Init();
			eth_uart = &huart1;
		}
		else { // defaults to Board1

		}
#endif
	}



	//MX_IWDG_Init();

	//MX_I2C2_Init();
	initializePWM();
	//dimming(50);
	//dimming(70);
	//dimming(100);
	/* USER CODE BEGIN 2 */
	HAL_Delay(30);



	HAL_TIM_Base_Start_IT(&htim7); 	//Activate IRQ for Timer7
	if (WDT_ENABLED==1) {
			 MX_IWDG_Init();
			__HAL_IWDG_START(&hiwdg);
			  HAL_IWDG_Refresh(&hiwdg);
	}





	//  BY NOW there is this switch to select Board0 (GPRS) or Board1/2 (Ethernet)
	if (BaseBoard == Board0) {  // GPRS
#if defined (GPRS_TRANSPORT)
		uint32_t ta, tb;

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

		// relayActivation(GPIOX2_GPIO_Port,GPIOX2_Pin);



		// (4) C Language wrapper to the Modem Abstract Factory
		do {
			gtransceiver = FACTORY(BaseBoard, hdevice, DataBuffer);
		} while (!gtransceiver);
#endif
	}  //  GPRS
	else {  // ETH
#if defined (ETH_TRANSPORT)
		// 1 Gets the initialized device handler
		do {
			hdevice = Device_Init(BaseBoard);
		} while (!hdevice);


		// 2 Allocates a circular buffer to work with
		DataBufferEth	= CircularBuffer (256, NULL);

		// 3 Enables the interrupt to get the characters into the callback
		HAL_UART_Receive_IT(eth_uart, &dataByteBufferETH, 1); // Enabling IRQ

		// 4 Creates the modem object (so far only issues a single command)
		do {
			gtransceiver = FACTORY(BaseBoard, hdevice, DataBufferEth);
			if (!gtransceiver)
				Device_Reset(BaseBoard);
		} while (!gtransceiver);

///		TestSynchonization();
#endif
	}


	/// ALL THAT FOLLOWS is COMMON all transceivers


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
	if (0) {
		


		if (updfw_protocol)
			SetVariable ("UPDFW_PROTO", updfw_protocol);

		if (updfw_server)
			SetVariable ("UPDFW_HOST", updfw_server);

//		int	updfw_port 				= 0;
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


	// Set the version compilation signature
	if (fw_version)
		SetVariable("FWVERSION",fw_version);

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


	/**
	///////////  CAN1  BEGIN   ////////////////////////////////////
	// ADDRESSES
	uint32_t ArrIDs[] = {0x1111,0x1000,0x2222,0x2000,0x3333, 0x0000};
	// MESSAGE to be sent
	char	Data[8];
	for(int i=0; i<8; i++)
		 Data[i] = 40 + i*2;

	int init_can = config_can (EnBitRate_250K, true, ArrIDs);
	write_can_message(0x1111, Data);
	int quit = 0;
	while (quit != 1) {
		quit = read_can_bus(Data);
		if (quit == 0)
			tprintf ("CAN read message %8.8s", Data);
	 }

	///////////  CAN1  END ////////////////////////////////////
	**/
	if (hmqtt) {
			extern char	pretrbuf[];
			long lastget = GetTimeStamp();

			int rc = 1;
			int cnomssg = 0;
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

			  /* Infinite loop */
			  /* USER CODE BEGIN WHILE */

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
								cnomssg = 0;
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
									cnomssg = 0;
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
				 //  not connected ... retry or exit?
		}






  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

}


DEV_HANDLER Device_Init(BoardType board) {
	DEV_HANDLER result;
	if (board == Board0){
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
	if (board == Board0){
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
	if (board == Board0){
#if defined(BUILD_M95) || defined(BUILD_BG96)
		return MODEMFACTORY(hconn, cb);
#endif
	}
	else {
#if defined(BUILD_RM08)
		return ETHFACTORY(hconn, cb);
#endif
	}

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



/** System Clock Configuration
*/
void SystemClock_Config(BoardType boardtype)
{
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
//	if (1) {
	if ((boardtype == Board1) || (boardtype == Board2)) {

		    /**Initializes the CPU, AHB and APB busses clocks
		    */
		  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI
		                              |RCC_OSCILLATORTYPE_HSE;
		  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
		  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
		  RCC_OscInitStruct.HSICalibrationValue = 16;
		  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
		  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
		  //RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
		 // RCC_OscInitStruct.PLL.PLLM = 16;
		 // RCC_OscInitStruct.PLL.PLLN = 192;
		  //RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
		  //RCC_OscInitStruct.PLL.PLLQ = 4;
		  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
		  {
		    _Error_Handler(__FILE__, __LINE__);
		  }

		    /**Initializes the CPU, AHB and APB busses clocks
		    */
		  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
		                              |RCC_CLOCKTYPE_PCLK1;//|RCC_CLOCKTYPE_PCLK2;
		  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
		  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
		  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
		 // RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

		  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
		  {
		    _Error_Handler(__FILE__, __LINE__);
		  }

		  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
		  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
		  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
		  {
		    _Error_Handler(__FILE__, __LINE__);
		  }
	}
	else {
		/**Initializes the CPU, AHB and APB busses clocks
		*/
		RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
		RCC_OscInitStruct.HSEState = RCC_HSE_ON;
		RCC_OscInitStruct.LSIState = RCC_LSI_ON;
		RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
		if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
		{
		_Error_Handler(__FILE__, __LINE__);
		}

		/**Initializes the CPU, AHB and APB busses clocks
		*/
		RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
								  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
		RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
		RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
		RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
		RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

		if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
		{
		_Error_Handler(__FILE__, __LINE__);
		}
	}
	// COMMON
    /**Configure the Systick interrupt time */
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick */
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* ADC1 init function */
static void MX_ADC1_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
    */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* IWDG init function */
static void MX_IWDG_Init(void)
{

  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* RTC init function */
static void MX_RTC_Init(void)
{

  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;



    /**Initialize RTC Only
    */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
  hrtc.Init.SynchPrediv = RTC_SYNCH_PREDIV;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
  //HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR0,0x0);
    /**Initialize RTC and set the Time and Date
    */
  if(HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0) != 0x32F2){
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.TimeFormat = RTC_HOURFORMAT12_AM;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sDate.WeekDay = RTC_WEEKDAY_THURSDAY;
  sDate.Month = RTC_MONTH_MAY;
  sDate.Date = 0x0;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR0,0x32F2);
  }

}
/* TIM3 init function */
static void MX_TIM3_Init(void)
{

  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = (SystemCoreClock/1000)-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 10; //10ms
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;


  if (HAL_TIM_Base_Init(&htim3) != HAL_OK) //francis
    {
      _Error_Handler(__FILE__, __LINE__);
    }

  /*
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

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim3);
  */

}

/* TIM7 init function */
static void MX_TIM7_Init(void)
{

  TIM_MasterConfigTypeDef sMasterConfig;

  htim7.Instance = TIM7;
  //htim7.Init.Prescaler = 0;
  htim7.Init.Prescaler = (SystemCoreClock/1000)-1;;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 10000;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

#if defined(BUILD_RM08)
/* USART1 init function */
static void MX_USART1_UART_Init(void) {

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
//  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
//  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* USART4 init function */
static void MX_UART4_UART_Init(void)
{

	huart4.Instance = UART4;
	huart4.Init.BaudRate = 115200;
	huart4.Init.WordLength = UART_WORDLENGTH_8B;
	huart4.Init.StopBits = UART_STOPBITS_1;
	huart4.Init.Parity = UART_PARITY_NONE;
	huart4.Init.Mode = UART_MODE_TX_RX;
	huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart4.Init.OverSampling = UART_OVERSAMPLING_16;
//  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
//  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

#endif


/* USART2 init function */
static void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 19200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}



#if defined(BUILD_M95) || defined(BUILD_BG96)
/* USART6 init function */
static void MX_USART6_UART_Init(void)
{

  huart6.Instance = USART6;
  huart6.Init.BaudRate = 19200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}
#endif



/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(BoardType boardtype)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  if (boardtype == Board0){
#if defined (BUILD_M95) || defined (BUILD_BG96)

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();  // seems unused...
  __HAL_RCC_GPIOA_CLK_ENABLE();
#if !defined (BUILD_CANBUS)
  __HAL_RCC_GPIOB_CLK_ENABLE();   // conflicts with CAN init
#endif
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, blueRGB_Pin|redRGB_Pin|greenRGB_Pin|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
                          |GPIO_PIN_7|txDBG_3G_Pin|GPIO_PIN_9|GPIO_PIN_10
                          |emerg_3G_Pin|pwrKey_3G_Pin, GPIO_PIN_RESET);

  #if ! defined  (BUILD_CANBUS)
  /*Configure GPIO pin Output Level */
  	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10
                          |GPIO_PIN_11|Relay1_Pin|GPIO_PIN_14|GPIO_PIN_15
                          |GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
                          |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);
  #endif
  #if 1
  /*Configure GPIO pin Output Level */
     HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
  #endif
  /*Configure GPIO pins : blueRGB_Pin redRGB_Pin greenRGB_Pin */
  GPIO_InitStruct.Pin = blueRGB_Pin|redRGB_Pin|greenRGB_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : status_3G_Pin netlight_3G_Pin */
  GPIO_InitStruct.Pin = status_3G_Pin|netlight_3G_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PC3 PC4 PC5 PC8
                           PC10 PC11 PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8
                          |GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA4 PA5 PA6
                           PA7 txDBG_3G_Pin PA9 PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
                          |GPIO_PIN_7|txDBG_3G_Pin|GPIO_PIN_9|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  #if !defined (BUILD_CANBUS)
  /*Configure GPIO pins : PB0 PB1 PB2 PB10
                           PB11 Relay1_Pin PB14 PB15
                           PB3 PB4 PB5 PB6
                           PB7 PB8 PB9 */
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10
                          |GPIO_PIN_11|Relay1_Pin|GPIO_PIN_14|GPIO_PIN_15
                          |GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
                          |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  #endif
  /*Configure GPIO pin : PWM_sim_Pin */
  GPIO_InitStruct.Pin = PWM_sim_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(PWM_sim_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : emerg_3G_Pin pwrKey_3G_Pin */
  GPIO_InitStruct.Pin = emerg_3G_Pin|pwrKey_3G_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : rxDBG_3G_Pin */
  GPIO_InitStruct.Pin = rxDBG_3G_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(rxDBG_3G_GPIO_Port, &GPIO_InitStruct);

  #if 1
  /*Configure GPIO pin : PD2 */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  #endif
#endif
  }
  else {
#if defined (BUILD_RM08)
		/* GPIO Ports Clock Enable */
		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOH_CLK_ENABLE();  // seems unused...
		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();   // conflicts with CAN init
		__HAL_RCC_GPIOD_CLK_ENABLE();   // leads to CAN_Init timeout

		if (boardtype == Board2) {

		  /*Configure GPIOC pin Output Level */
		  HAL_GPIO_WritePin(GPIOC, blueRGB_Pin|redRGB_Pin|greenRGB_Pin, GPIO_PIN_SET);

		  /*Configure GPIOA pin Output Level */
		  // NOTHING

		  /*Configure GPIOB pin Output Level */
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10 |GPIO_PIN_11, GPIO_PIN_SET);

		  /*Configure GPIOD pin Output Level */
		  // NOTHING

		  /*Configure GPIO pins : blueRGB_Pin redRGB_Pin greenRGB_Pin */
		  GPIO_InitStruct.Pin = blueRGB_Pin|redRGB_Pin|greenRGB_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		  /*Configure GPIO pins : PB0 PB1 PB2 PB10
								   PB11 Relay1_Pin PB14 PB15
								   PB3 PB4 PB5 PB6
								   PB7 PB8 PB9 */
		  GPIO_InitStruct.Pin = GPIO_PIN_10 |GPIO_PIN_11;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		}
		else {

		  /*Configure GPIOC pin Output Level */
		  HAL_GPIO_WritePin(GPIOC, blueRGB_Pin|redRGB_Pin|greenRGB_Pin|GPIO_PIN_3
								  |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8|GPIO_PIN_10
								  |GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_SET);

		  /*Configure GPIOA pin Output Level */
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
								  |GPIO_PIN_7|txDBG_3G_Pin|GPIO_PIN_9|GPIO_PIN_10
								  |emerg_3G_Pin|pwrKey_3G_Pin, GPIO_PIN_RESET);


		  /*Configure GPIOB pin Output Level */
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|//GPIO_PIN_1|
								  GPIO_PIN_2 // |GPIO_PIN_10 |GPIO_PIN_11 removed
								  |Relay1_Pin|GPIO_PIN_14|GPIO_PIN_15
								  |GPIO_PIN_3|GPIO_PIN_4|//GPIO_PIN_5|
								  GPIO_PIN_6
								  |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10 |GPIO_PIN_11, GPIO_PIN_SET);


		  /*Configure GPIOD pin Output Level */
		  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);


		  /*Configure GPIO pins : blueRGB_Pin redRGB_Pin greenRGB_Pin */
		  GPIO_InitStruct.Pin = blueRGB_Pin|redRGB_Pin|greenRGB_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		  /*Configure GPIO pins : status_3G_Pin netlight_3G_Pin */
		  GPIO_InitStruct.Pin = status_3G_Pin|netlight_3G_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		  /*Configure GPIO pins : PC3 PC4 PC5 PC8
								   PC10 PC11 PC12 */
		  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8
								  |GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		  /*Configure GPIO pins : PA0 PA4 PA5 PA6
								   PA7 txDBG_3G_Pin PA9 PA10 */
		  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
								  |GPIO_PIN_7|txDBG_3G_Pin;//GPIO_PIN_9|GPIO_PIN_10;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_2|GPIO_PIN_10
									|GPIO_PIN_11|Relay1_Pin|GPIO_PIN_14|GPIO_PIN_15
									|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_6
								  |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		  /*Configure GPIO pin : PWM_sim_Pin */
		  GPIO_InitStruct.Pin = PWM_sim_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  HAL_GPIO_Init(PWM_sim_GPIO_Port, &GPIO_InitStruct);

		  /*Configure GPIO pins : emerg_3G_Pin pwrKey_3G_Pin */
		  GPIO_InitStruct.Pin = emerg_3G_Pin|pwrKey_3G_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		  /*Configure GPIO pin : rxDBG_3G_Pin */
		  GPIO_InitStruct.Pin = rxDBG_3G_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  HAL_GPIO_Init(rxDBG_3G_GPIO_Port, &GPIO_InitStruct);

		  /*Configure GPIO pin : PD2 */
		  GPIO_InitStruct.Pin = GPIO_PIN_2;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

		}

#endif

  }
}


/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

	if (huart->Instance==eth_uart->Instance) {
 		 Write(DataBufferEth, dataByteBufferETH);
 		 HAL_UART_Receive_IT(huart,&dataByteBufferETH,1);

	}
	else if (huart->Instance==GPRS_UART.Instance) {
	  if (! bydma) { // only if not BYDMA
	  	 nirqs++;
	  		 allnew += Write(DataBuffer, dataByteBufferIRQ);
	  		 if (IsFull(DataBuffer)) {
	  			DataBuffer->overruns++;
	  		}
//	  		{
//	  			int nextw = (write_offset + 1) % bufsize;  // next position to be written
//	  			Cbuffer[write_offset++] =  dataByteBufferIRQ;
//	  			write_offset = nextw;
//	  	 	 	allnew += Write(dataByteBufferIRQ);
	  			balnew++;
//	  		}

//			 (huart,&dataByteBufferIRQ,1);
//	  	 }
//	  	 {
	  			/**
			  GPRSBufferReceivedBytes++;
			  GPRSbuffer[indexGPRSBufferReceived]=dataByteBufferIRQ;
			  indexGPRSBufferReceived=(indexGPRSBufferReceived+1)%SIZE_GPRS_BUFFER;
	  		  allold++;
	  		  balold++;
	  		  **/
//	  	 }
	  		  HAL_UART_Receive_IT(huart,&dataByteBufferIRQ,1);
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
		if (elapsed10seconds%TIMING_TIMEOUT_GPRS==0)

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
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
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

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
