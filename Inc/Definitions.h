#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

// Value to be used when discovery fails
// Values are:
//	Board0 -> Classic M2M (Vodafone)
//  Board1 -> Square, piggyback M2M
//  Board2 -> Pretty box (Orange)
#define	DEFAULTBOARD	TalkingFiberBoard


/* ========================= TF RELATED SETTINGS ============================ */

// Enable periodic TF measurement and set default publish period in minutes
#define ENABLE_PERIODIC
#define DEFAULT_PERIOD  1

/* ========================================================================== */

//#define BUILD_ALL_MODEMS
#define BUILD_M95
//#define BUILD_BG96
//#define BUILD_RM08

#if defined(BUILD_ALL_MODEMS)
	#define BUILD_M95
	#define BUILD_BG96
	#define BUILD_RM08
#endif

#if defined(BUILD_M95) || defined(BUILD_BG96)
#define GPRS_TRANSPORT
#undef DEFAULTBOARD
#define	DEFAULTBOARD	TalkingFiberBoard
#endif

#if defined(BUILD_RM08)
#define ETH_TRANSPORT
#endif


// CAUTION:  BUILD_CANBUS here has to be used together with
//		#define HAL_CAN_MODULE_ENABLED
// in file stm32f2xx_hal_conf.h
//	#define BUILD_CANBUS


/**
#if defined(BUILD_CANBUS)
#define HAL_CAN_MODULE_ENABLED
#else
#undef HAL_CAN_MODULE_ENABLED
#endif
**/


#define DEBUG
#define TIME_NTP
// #define TIME_GPRS




#ifdef __cplusplus
 extern "C" {
#endif

extern int	timepolling;
extern int	timeping;

// OBSOLETE
#define TIMING_TIMEOUT_GPRS 2



// seconds to wait when polling, before giving up
#define	TIMEPOLLING  2

// seconds to wait after last message, to send a Ping
#define	TIMEPING  120					// in seconds

// hours from last time, to order a synchronize
#define	TIMESYNCHRONIZE		12  		// In Hours

// seconds after submit HLK configuration
#define	TIME_HLKCONFIG		10  		// In seconds

// seconds after submit HLK connection
#define	TIME_HLKCONNECT		50  		// In seconds



 // Maximum number ot connection tries before resetting the device
#define	MAX_CONNECT_TRIES	5
 // Maximum size for the datablock to save & restore from NVM
#define STORESIZE 512

#define RTC_CLOCK_SOURCE_LSI
//#define RTC_CLOCK_SOURCE_LSE  //Se usa el externo 32768Hz.
//#define RTC_CLOCK_SOURCE_HSE  //Se usa el externo 32768Hz.

#ifdef RTC_CLOCK_SOURCE_LSI
#define RTC_ASYNCH_PREDIV    0x7F
#define RTC_SYNCH_PREDIV     0x137
#endif

#ifdef RTC_CLOCK_SOURCE_LSE
#define RTC_ASYNCH_PREDIV  0x7F
#define RTC_SYNCH_PREDIV   0x00FF
#endif

#ifdef RTC_CLOCK_SOURCE_HSE
#define RTC_ASYNCH_PREDIV  100
#define RTC_SYNCH_PREDIV   8125
#endif

extern unsigned char  WDT_ENABLED;		/// Enable for activate independent watch dog timer

extern 	int			ethmode;
extern	const char	*wifi_ssid;
extern	const char	*wifi_password;


extern	char	*const_APN;
extern	char	*const_SERVER_NTP;
extern	char	*const_MAIN_SERVER;


extern	char	*user;
extern	char	*password;
extern	char	*host;
extern	unsigned int port;
extern	int 	security;

extern	char	*id0;

// Global, so far
// has to be hideen in a  utilities file when it should be
extern 	char	*topicroot;
extern 	char	*topictr0;
extern	char	*topicpub0;
extern	char	*broadcast;
extern	char	*topicsub0;




typedef	 int  (* Action) (void);


#ifdef __cplusplus
}
#endif


#endif
