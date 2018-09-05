#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

#define DEBUG

#define TIME_TX_RF_FRAME_2000BPS  64   //always in ms.

#ifdef __cplusplus
 extern "C" {
#endif

// FIXME change to STM32F2 related
#define VREFINT_CAL		(*(uint16_t*)0x1FFF7A2A) 	// reference calibration register
#define AVDD_EXPECTED	3.3f 						// expected voltage on avdd
#define ADC_RES			12 							// ADC resolution
#define ADC_VREF		1.21f 						// typical ADC reference voltage

#define VDD_PHOTODIODE	5.0f
#define R_23_1			120000l 					//120KOm

#define USE_STOPMODE

#define SIZE_APN 60
#define SIZE_MAIN_SERVER 60
#define SIZE_NTP_SERVER 60
#define SIZE_GPRS_BUFFER 256
#define TIMING_TIMEOUT_GPRS 2

/* ========================= GPRS RELATED SETTINGS ========================== */

#define COMMUNICATION_M95
#define CONST_SERVER_NTP "\"0.europe.pool.ntp.org\"\r\0"

// Enable this define in order to use DMA MQTT management from USR in M95
#define HYBRID_M95_WLAN_CODE

#define HUART_GPRS huart6

/* ========================================================================== */

//#define DISP_TIMING

//#define DISP_THREAD_DEBUG

#define I2C_RETRIES 5
#define SD_RETRIES  5

// Maximun ammount of minutes to wait task response before reboot the device
#define TASK_WAIT_MAX_MINUTES		5

// TODO change to F2
#define FLASH_BANKB_START_SECTOR	FLASH_SECTOR_0
#define FLASH_BANKB_SECTORS			2
// Application
#define FLASH_BANKA_START_SECTOR	FLASH_SECTOR_2
#define FLASH_BANKA_SECTORS			6
// Application copy
#define FLASH_BANKC_START_SECTOR	FLASH_SECTOR_8
#define FLASH_BANKC_SECTORS			3
// Shared memory address
#define FLASH_BANKD_START_SECTOR	FLASH_SECTOR_11
#define FLASH_BANKD_SECTORS			1

#define UPDFW_PRIMITIVE_VALUE 	0
#define UPDFW_COUNT_DEFAULT		9

// MQTT Queue debug
//#define DEBUG_ENABLE_MQTT_QUEUE_PUB
// MQTT Queue debug messages publish period in seconds
#define DEBUG_MQTT_QUEUE_PUB_PERIOD 15

// This buffer size limits MQTT response size
#define MQTT_RESPONSE_BUF_SIZE 2048

/* ========================= TIME RELATED SETTINGS ========================== */

// Uncomment this define to correct time with TIME server every hour
//#define TIME_CORRECT_PERIODICALLY

// Period in hours to execute time correction
#define TIME_CORRECT_PERIOD_HOURS	6

// Comment to disable receiving time from TIME server in WLAN mode
#define WLAN_USE_TIME_SERVER

// TIME server parameters
#define TIME_SERVER_PORT 37
#define TIME_SERVER_NAME "time.nist.gov"

// TODO Please go to api.timezonedb.com and register your API key to put here
#define TZ_SERVER_PORT   80
#define TZ_SERVER_NAME   "api.timezonedb.com"
#define TZ_API_KEY       "0OICAP1DG2AU" // TODO this API key should be changed

// Use RTC for time operations
#define TIME_WITH_RTC

 /* ========================= TF RELATED SETTINGS ========================== */

#define CONNECT_ONLY_TO_SEND

// When this workaround is used, period of periodic (if enabled) and alerts
// will be the same: lesser of two DEFAULT_PERIOD and DEFAULT_ALERT_M
//#define CONNECT_ONLY_TO_SEND_WORKAROUND

// Enable periodic TF measurement and set default publish period in minutes
#define ENABLE_PERIODIC
#define DEFAULT_PERIOD  "5"

//#define USE_SD_CARD

 // Enable alarm TF and set default period (minutes) and TH (mV)
#define ENABLE_ALERT
#define DEFAULT_ALERT_M "10"
#define DEFAULT_ALERT_TH "2700"

/* ========================================================================== */

#ifdef __cplusplus
}
#endif


#endif
