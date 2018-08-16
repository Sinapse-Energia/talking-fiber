/*
 * parametrizacion.cpp
 *
 *  Created on: 9 ago. 2017
 *      Author: juanra
 */



#include	"context.h"
#include	"Frames.h"
#include 	"Definitions.h"
#include	"NBinterface.h"
#include	"Application.h"
#include	"BLInterface.h"



//		(1) POPULATE the CONTEXT
//		This is the current way to  populate the Context :
//		Declare an static array of pointers to CVariable objects
//			and then, assign it to the static member
int	 CreateContext() {
	static		CVariable		*ALLVARS[] = {

		new CVariable ("Current time of day",		"NOW",		new CVartime () ),
		new CVariable ("Current date and time",		"DATE_TIME",new CVarstring () ),
		new CVariable ("Unix Timestamp",			"UNIXTS",	new CVarint (-1) ),  // TIMESTAMP
		new CVariable ("Time Zone",					"TZONE",	new CVarint () ),

		new CVariable ("Last Synchronization time",	"SYNCHTS",	new CVarint (0) ),	// TIMESTAMP LAST SYNCH

		new CVariable ("Current message",			"MSSG",		new CVarstring () ),
		new CVariable ("Message tag",				"TAG",		new CVarstring () ),
		new CVariable ("Node identifier",			"ID",		new CVarstring () ),
		new CVariable ("Generic result",			"RESULT",	new CVarstring () ),


		new CVariable ("List of commands",			"CMDLIST",	new CVarstring ("ohhh") ),

// DEFAULT CONNECTION PARAMETERS
		new CVariable ("Server IP address",			"IP",		new CVarstring () ),
		new CVariable ("Server TCP PORT",			"PORT",		new CVarint () ),
		new CVariable ("Username",					"USER",		new CVarstring () ),
		new CVariable ("Password",					"PSWD",		new CVarstring () ),
		new CVariable ("Server security type",		"SEC",		new CVarint () ),


#if defined(GPRS_TRANSPORT)
		new CVariable ("GPRS Transceiver",			"GPRSDEVICE",new CVarstring () ),
		new CVariable ("Access Point ",				"APN",		new CVarstring () ),
		new CVariable ("Access Point Name",			"APNNAME",	new CVarstring () ),
		new CVariable ("Access Point User",			"APNUSER",	new CVarstring () ),
		new CVariable ("Access Point Password",		"APNPSWD",	new CVarstring () ),
#endif

// CONTINGENCY CONNECTION PARAMETERS
		new CVariable ("Last server IP address",	"LIP",		new CVarstring () ),
		new CVariable ("Last server TCP PORT",		"LPORT",	new CVarint   () ),
		new CVariable ("Last username",				"LUSER",	new CVarstring () ),
		new CVariable ("Last password",				"LPSWD",	new CVarstring () ),
		new CVariable ("Last security type",		"LSEC",		new CVarint () ),
		new CVariable ("Last Access Point Name",	"LAPN",		new CVarstring () ),

// COLLECTED during connection
		new CVariable ("Local IP",					"LOCALIP",	new CVarstring () ),
#if defined(GPRS_TRANSPORT)
		new CVariable ("SIM Card Id",				"IDSIM",	new CVarstring () ),
		new CVariable ("Product SN",				"IMEI",		new CVarstring () ),
#endif

#if defined(ETH_TRANSPORT)
		new CVariable ("MAC adrdress",				"MAC",		new CVarstring () ),
#endif

// APPLICATION VARIABLES
		new CVariable ("Status",					"STAT",		new CVarint () ),
//		new CVariable ("Dimming status",			"DSTAT",	new CVarint () ),   // obsolete, now renamed to PWM

		new CVariable ("Temperature",				"TEMP",		new CVarint () ),
		new CVariable ("Voltage",					"V",		new CVarint () ),
		new CVariable ("Current",					"A",		new CVarint () ),
		new CVariable ("Active power",				"P",		new CVardec () ),
		new CVariable ("Reactive power",			"Q",		new CVardec () ),
		new CVariable ("Apparent power",			"S",		new CVardec () ),
		new CVariable ("Aggregated active energy",	"EP",		new CVardec () ),
		new CVariable ("Aggregated reactive energy","EQ",		new CVardec () ),
		new CVariable ("Aggregated apparent energy","ES",		new CVardec () ),
		new CVariable ("Frequency",					"F",		new CVardec () ),

// Periodic Measurement Context entries
		new CVariable ("Meassurement Period",		"MPERIOD",	new CVarint	 () ),
		new CVariable ("Periodic measurement begin","MPBEGIN",	new CVartime () ),
		new CVariable ("Periodic measurement end",	"MPEND",	new CVartime () ),
		new CVariable ("Periodic measurement days", "MPTIMES",	new CVarint  () ),

		new CVariable ("Profile programation","PROGR",			new CVarlist (10, new CVartuple(new CVarint(), new CVartime()) )),

// GEOLOCALIZATION
		new CVariable ("Longitude",				"LONGITUDE",	new CVarstring ("-1") ), // default value by init
		new CVariable ("Latitude",				"LATITUDE",		new CVarstring ("-1") ), // default value by init
		new CVariable ("Period of GPS sendings","LOCPERIOD",	new CVarint () ),
		new CVariable ("GPS Timestamp",			"GPSTS",		new CVarint (-1) ),  // TIMESTAMP

		new CVariable ("GPS active",			"GPS",			new CVarint () ),

// Sun relative times
		new CVariable ("Current sunrise time",	"SUNRISE",		new CVartime () ),
		new CVariable ("Current sunset time",	"SUNSET",		new CVartime () ),

// DATA ITEMS to be shared with BOOTLOADER
		new CVariable ("Firmware Name",				"FWNAME",		new CVarstring () ),
		new CVariable ("Firmware Version",			"FWVERSION",	new CVarstring () ),
		new CVariable ("Update Firmware",			"UPDFW",		new CVarint () ),
		new CVariable ("Update Firmware Counter",	"UPDFW_COUNT",	new CVarint () ),
		new CVariable ("Update Firmware Protocol",	"UPDFW_PROTO",	new CVarstring () ),
		new CVariable ("Update Firmware Server",	"UPDFW_HOST",	new CVarstring () ),
		new CVariable ("Update Firmware Port",		"UPDFW_PORT",	new CVarint () ),
		new CVariable ("Update Firmware Name",		"UPDFW_NAME",	new CVarstring () ),
		new CVariable ("Update Firmware Route",		"UPDFW_ROUTE",	new CVarstring () ),
		new CVariable ("Update Firmware User",		"UPDFW_USER",	new CVarstring () ),
		new CVariable ("Update Firmware Password",	"UPDFW_PSWD",	new CVarstring () ),

		new CVariable ("GPIO Status",				"GPIO",			new CVarstring () ),
		new CVariable ("PWM Status",				"PWM",			new CVarint () ),

		// INPUT PARAMETERS to populate DATA ITEMS
		//	(prefix p to the same name
		new CVariable ("Firmware Name",				"pFWNAME",		new CVarstring () ),
		new CVariable ("Firmware Version",			"pFWVERSION",	new CVarstring () ),
		new CVariable ("Update Firmware",			"pUPDFW",		new CVarint () ),
		new CVariable ("Update Firmware Counter",	"pUPDFW_COUNT",	new CVarint () ),
		new CVariable ("Update Firmware Protocol",	"pUPDFW_PROTO",	new CVarstring () ),
		new CVariable ("Update Firmware Server",	"pUPDFW_HOST",	new CVarstring () ),
		new CVariable ("Update Firmware Port",		"pUPDFW_PORT",	new CVarint () ),
		new CVariable ("Update Firmware Name",		"pUPDFW_NAME",	new CVarstring () ),
		new CVariable ("Update Firmware Route",		"pUPDFW_ROUTE",	new CVarstring () ),
		new CVariable ("Update Firmware User",		"pUPDFW_USER",	new CVarstring () ),
		new CVariable ("Update Firmware Password",	"pUPDFW_PSWD",	new CVarstring () ),

		new CVariable ("AT Command to test",  "ATCMD",	new CVarstring  () ),
		new CVariable ("Reply to the AT-Cmd", "ATREPLY",new CVarstring  () ),

// CAN VARIABLES
#if defined(BUILD_CANBUS)
		new CVariable ("CAN interface id",			"CAN_ID",		new CVarint () ),
		new CVariable ("CAN baud rate",				"CAN_BAUDRATE",	new CVarint () ),
		new CVariable ("CAN mode",					"CAN_MODE",		new CVarint () ),
		new CVariable ("CAN Listening addresses",	"CAN_R_ADDRESS",new CVarlist (10, new CVarstring()) ),


		new CVariable ("CAN writing address",		"CAN_W_ADDRESS",new CVarstring() ),
		new CVariable ("CAN format",				"CAN_FORMAT",	new CVarstring () ),
		new CVariable ("CAN write message",			"CAN_W_MESSAGE",new CVarstring () ),

		// CAN_R_MESSAGES is a pair (Address + payload)
		new CVariable ("CAN read message list",		"CAN_R_MESSAGES",new CVarlist (10, new CVartuple( new CVarstring(), new CVarstring()) )),
		new CVariable ("CAN read number ",			"CAN_R_NUMBER",	new CVarint()  ),

		new CVariable ("CAN result",				"CAN_RESULT",	new CVarstring () ),
#endif

// ANALOG & DIGITAL SENSORS
		new CVariable ("Analog Sensor Pin Number ",	"ASNPIN",			new CVarint () ),
		new CVariable ("Analog Sensor Value ",		"ASVALUE",			new CVardec () ),
		new CVariable ("Analog Sensor threshold flag","ASFLAG",		new CVarstring () ),

		new CVariable ("Digital Sensor Pin Number ","DSNPIN",			new CVarint () ),
		new CVariable ("Digital Sensor Value ",		"DSVALUE",		new CVarint () ),
		new CVariable ("Digital Sensor threshold flag","DSFLAG",	new CVarstring () ),



#ifdef DEBUG
//		Helpers for debug & test
		new CVariable ("MSEC POLL", 		"POLL",	new CVarint  () ),
		new CVariable ("MSEC PING", 		"PING",	new CVarint  () ),
		new CVariable ("Time Delay in hours", "TDELAY",	new CVarint  () ),
		new CVariable ("Timers stringized", "TIMERTEXT",new CVarstring  () ),

//		Helpers for "board signature"
		new CVariable ("Board Type", 		"BOARD",	new CVarint  () ),
#endif


	//	All the following variables belong to the EPD Subspace
	//	All of them have short names prefixed with EPD::
	//	The one with the same prefix and name is the primary key of the Subspace

#ifdef CMC

		new CVariable ("EDP Ident",					"EPD::EPD",	new CVarstring () ),
		new CVariable ("EDP Dimming",				"EPD::PWM",	new CVarint () ),
		new CVariable ("EDP Temperature",			"EPD::TEMP",	new CVarint () ),
		new CVariable ("EDP Voltage",				"EPD::V",		new CVarint () ),
		new CVariable ("EDP Current",				"EPD::A",	new CVarint () ),
		new CVariable ("EDP Active power",			"EPD::P",	new CVardec () ),
#endif

		// Photodiode parameters
		new CVariable ("PFM_TO_ANALOGUE voltage", 		"TFVOL",	new CVarstring  ("-1") ),
		new CVariable ("PFM_TO_ANALOGUE timestamp", 	"TFVTS",	new CVarint     (0) ),

		NULL
	};
	CVariable::Context = ALLVARS;
	CSubspace::RestoreAll();
	return 1;
}


	// (2) VARIABLE HANDLERS  
	// to declare the variables having any special handling when writen and/or read
	// Is an array of three element structure (variable nick, read handler, write handler, terminated by a NULL entry 
#include "wrsouth.h"
#include "utils.h"

#include "TimeEvents.h"
struct	st_wrapper	dispatch[] = {
	"NOW",		UpdateNow,	NULL,
	"DATE_TIME",UpdateTS,	NULL,
	"UNIXTS",	UpdateUTS,	NULL,
//	"TEMP", 	Read_Temp,		NULL,
//	"A",		Read_I,			NULL,
//	"V",		Read_V,			NULL,
//	"P",		Read_P,			NULL,
//	"Q",		Read_Q,			NULL,
//	"S",		Read_S,			NULL,
//	"EP",		Read_EP,		NULL,
//	"EQ",		Read_EQ,		NULL,
//	"ES",		Read_ES,		NULL,
	"SUNRISE",	UpdateSR,		NULL,
	"SUNSET",	UpdateSS,		NULL,
#ifdef DEBUG
	"TDELAY",	NULL,			Set_TimeOffset,
	"POLL",		NULL,			Set_TimePoll,
	"PING",		NULL,			Set_TimePing,
	"TIMERTEXT", ShowTimers, 	NULL,
#endif
	"TFVOL",    Read_TFVOL,    NULL,

	NULL
};



//	(3) MESSAGE METADATA DEFINITIONS
// This is the current way of populating the input and output Framesets
//	IMPORTANT:	the OutAPI array has to be populated BEFORE,
//				because input frames can eventually reference output frames as replay
//
//	The string  have some PARTS, separated by % character
//
//		FRAMEID % TEMPLATE % DESCRIPTION % TOPIC-ID % REPLY
//			FRAMEID:	Optional.Is only necesary for Request/Replay pairs
//			TEMPLATE	The template of the message (bolerplate text and variables)
//			DESCRIPTION	Optional, as aditional information
//			TOPIC-ID		(not used by now)
//			REPLAY		Optional. Only for INPUT messages which have to trigger a response.

int	 ReadMetadata(char *dominioIn, char *dominioOut){
	const char	*linesOut[] = {
// System messages
		"ERROR%999;$MSSG;Unknown message%Error%%",
		"BAD%$MSSG;Ignored: Incorrect parameters%%",
		"EXECMD%700;$ID;$ATREPLY;%AT-COMMAND reply%%",
		"CMDLIST%721;$CMDLIST;%List of supported commands%%",

// Application replies
//		"MEASU%$ID;$TEMP;$STAT;$PWM;$A;$V;$P;$Q;$S;$EP;$EQ;$ES;$F;%periodic measurement%%",
//		"TALRT%$ID;1;$TEMP%temperature alert%%",
//		"VALRT%$ID;2;$V%voltage alert%%",
//		"IALRT%$ID;3;$A%current alert%%",
//		"RSTAT%107;$ID;$GPIO;$PWM;$UNIXTS%%",

// BootLoader share	replies
		"FWCNF%607;$ID;$RESULT;$FWVERSION;$UPDFW_HOST;$UPDFW_PORT;$UPDFW_PROTO;$UPDFW_ROUTE;$UPDFW_USER;$UPDFW_PSWD;$UPDFW_COUNT;%Bootloader parameters%%",
//	

// Show Firmware Version 
		"APIVER%609;$ID;$FWVERSION;%Show Firmware version%%",

// Show LOCALIZATION 
		"SHLOC%500;$ID;$LATITUDE;$LONGITUDE;$GPSTS;%GPS Localization%%",

// Sunrise-Sunset times
		"SHSUNS%604;$ID;$LATITUDE;$LONGITUDE;$SUNRISE;$SUNSET%Get Current sunset and sunrise times%%",

#if defined(BUILD_CANBUS)
// CAN output interface
		"CANCONFR%L1_CONF_CAN_R;$ID;$CAN_ID;$UNIXTS;$CAN_RESULT;%Reply to CAN configuration%%",
		"CANWRITR%L1_WRITE_CAN_R;$ID;$CAN_ID;$UNIXTS;$CAN_RESULT;%Reply to CAN send%%",
		"CANREADR%L1_READ_OD_CAN_R;$ID;$CAN_ID;$CAN_FORMAT;$CAN_R_MESSAGES;$UNIXTS%Reply to CAN read%%",

#endif

// ANALOG & DIGITAL SENSORS
//		"DSNOUT%702;$ID;$DSNPIN;$DSVALUE;$DSFLAG;$UNIXTS;%Digital Sensor Reply%%",
//		"ASNOUT%705;$ID;$ASNPIN;$ASVALUE;$ASFLAG;$UNIXTS;%Analog Sensor Reply%%",



#ifdef DEBUG
		"SHSYNCH%LAST=;$SYNCHTS;ahora=;$NOW;AHORA=;$DATE_TIME%Show Synchronization status%%",
		"SHPROG%$PROGR%Show Program%%",
		"SHTIM%$TIMERTEXT%Show Timmers%%",
		"SHPER%$MPERIOD;$MPBEGIN;$MPEND;$MPTIMES;%Show Peridocs%%",
		"SHDT%01;$DATE_TIME;%Showme date and time%%",
  #if defined (GPRS_TRANSPORT)
		  "SHAPN%APNparts=;$APNNAME;$APNUSER;$APNPSWD;Default=>;$APN;LastAPN=;$LAPN%Show APN%%",
  #endif
#endif

#ifdef CMC
		// TEXT* are Test ConEXTension Frames 
		// (test Frames for CMC/EPD Context Extension)
		//  713 command (response to TEXT7, 313 comman
		"TEXT3%713;@EPD($EPD::EPD,$EPD::P)%List of EPDs and power values%%",
		// INVENTED FRAME for get on demand EPD values (response to 315 command)
		"TEXT9%$EPD::EPD;$EPD::PWM;$EPD::TEMP;$EPD::V;$EPD::A;$EPD::P;%EPD meassurement%%",
#endif
		
		"L3TFR%L3_TF_STATUS_R;$ID;$TFVOL;$TFVTS;%return photodiode voltage%%",

		NULL
	};



	const char	*linesIn[] = {
// System commands
		"ATRUN%300;$ATCMD%AT Command On demand%%EXECMD%",
		"GETCMDS%321;%Get list of commands%%CMDLIST%",

// Application commands
//		"PULLM%1;%pull measurement%xx%MEASU%",
//		"LPROF%2;$PROGR% lighting profile programation%%",
//		"SDIMM%3;$PWM;% set ligthing status%%",
//		"PERMS%4;$MPERIOD;$MPBEGIN;$MPEND;$MPTIMES;%Periodoc meassurement parameters%%",
		"CCOMM%5;$IP;$PORT;$USER;$PSWD;$SEC;%configure communications%%",
//		"GSTAT%7;%Get Status%%RSTAT%",
		//		"CTHRS%6;$TX;$AX;$VX;% configure theresolds%%",

//	Bootloader commands
#if defined (GPRS_TRANSPORT)
		"APN%175;$APN;% Change APN%%",
		"CHAPN%127;$APNNAME;$APNUSER;$APNPSWD;% Change APN%%",
#endif
		"FWREQ%207;$pUPDFW_HOST;$pUPDFW_PORT;$pUPDFW_PROTO;$pUPDFW_ROUTE;$pUPDFW_NAME;$pUPDFW_USER;$pUPDFW_PSWD;$pUPDFW_COUNT%firmware updates config params%%FWCNF%",
		"FWTRG%208;$pUPDFW_USER;$pUPDFW_PSWD;%Trigger Bootloader%%FWCNF%",
		"APIREQ%209;%Retrieve API version%%APIVER%",


// Localization
		"LOCOND%400;%Localization on demand%xx%SHLOC%",
		"PERLOC%401;$LOCPERIOD%Localization periodic%xx%%",
		"SETCOOR%121;$LATITUDE;$LONGITUDE;%Set GPS Coordinates%%",

// Sunrise/Sunset times
		"SUNSGET%204;%Get Current sunset and sunrise times%%SHSUNS%",

#if defined(BUILD_CANBUS)
// CAN Interface input commands
		"CANCONF%L1_CONF_CAN;$CAN_ID;$CAN_BAUDRATE;$CAN_MODE;$CAN_R_ADDRESS;%CAN Configuration%%CANCONFR%",
		"CANWRIT%L1_WRITE_CAN;$CAN_ID;$CAN_W_ADDRESS;$CAN_FORMAT;$CAN_W_MESSAGE%CAN send message to one address%%CANWRITR%",
		"CANREAD%L1_READ_OD_CAN;$CAN_ID;$CAN_FORMAT;$CAN_R_NUMBER%CAN read message(s)%%CANREADR%",
#endif

// ANALOG & DIGITAL SENSORS
//		"DSNIN%302;$DSNPIN;%Digital Sensor Request%%DSNOUT",
//		"ASNIN%305;$ASNPIN;%Analog Sensor Request%%ASNOUT",

#ifdef DEBUG
		"QRSYNCH%197;%Get Synch state%%SHSYNCH",
		"RQTIM%198;%Show Timers %%SHTIM",
		"RQPROG%199;%Request PROGRAM %%SHPROG",
		"VPER%41;% VER PERIODICS%%SHPER",
		"DST%00;$TDELAY% ADELANTO DE HORA%%SHDT",
		"TIMESETS%298;$POLL;$PING%Bootloader Display%%",
		"FWSHW%299;;%Bootloader Display%%FWCNF%",
#if defined (GPRS_TRANSPORT)
		"APN1%128;% Request APNs%%SHAPN",
#endif
#endif


#ifdef CMC
		// 316; command : PRIVILEGED to CREATE EPDs
		"TEXT1%316;$$EPD::EPD;$EPD::P;%Set EPD Active power%%",
		// INVENTED: 317; command for setting EPD Dimming value
		"TEXT2%317;$EPD::EPD;$EPD::PWM;%Set EPD Dimming%%",
		// 313; command, asking for TEXT3 (713; ) replay
		"TEXT7%313;%On demand list of EDPS%yy%TEXT3%",
		// INVENTED: 313; command for requesting EPD mesaaurement (TEXT9)
		"TEXT8%315;$EPD::EPD;%On demand get EPD%yy%TEXT9%",
#endif

		"L3TFD%L3_TF_STATUS_OD;%request diode voltage%xx%L3TFR%",

		NULL
	};


	CFrame::OutAPI = ReadFrames2(linesOut);

	CFrame::InAPI = ReadFrames2(linesIn);
	return 1;
}


	// (4) MESSAGE HANDLERS 
	// to declare the messages having any special handling before and/or after processing
	// Is an array of three element structure (message id, pre handler, post handler, terminated by a NULL entry 


struct	st_fractions
	actions[] = {
//	"SDIMM", 	NULL,			SetDimming,			// Command 3; post-action to save
	"CCOMM", 	NULL,  			FULLNEWCONN,			// Command 5; post-action to reconnect
//	"PERMS",	NULL,			SetPeriodics,		// Command 4; post-action to update timers
//	"LPROF",	NULL,			SetProgramm,		// Command 2; post-action to update timers
	"APN", 		NULL,			SaveConnParams,		// Command 175; post-action to save
	"FWREQ",	NULL,			ValidateFwParams,	// Command 207; post action to validate and etc..
	"FWTRG",	NULL,			ValidateFwAuth,		// Command 208; post action to validate and etc..
	"PERLOC",	NULL,			SetPeriodicsGPS,	// Command 401; post-action to update timers
	"ATRUN",	NULL,			ExecCmd,			// Message 300; Post, to run the AT-CMD on the fly
	"CMDLIST",	BuildALL,		NULL,				// Message 500; pre action to get coordinates.
	"SHLOC",	QueryPosition,	NULL,				// Message 500; pre action to get coordinates.

#if defined(GPRS_TRANSPORT)
	"CHAPN",	NULL,			ManageAPNS,			// Command 127; post-action to concatenate APN parts
#endif

#if defined(BUILD_CANBUS)
// CAN input message Post-Actions
	"CANCONF",	NULL,			CAN_configure,		// Message L1_CONF_CAN post action
	"CANWRIT",	NULL,			CAN_Send,			// Message L1_WRITE_CAN post action
	"CANREAD",	NULL,			CAN_Read,			// Message L1_READ_OD_CAN  post action
#endif
// Analog & Digital sensors
//	"ASNOUT", 	ASN_READ,			NULL,
//	"DSNOUT", 	DSN_READ,			NULL,




#ifdef DEBUG
// Let's see if it does already work
	"SHSUNS",	EvalSunTimes,	NULL,				// Message
#endif
	NULL
};





//////////////////////////  MISCELANEA /////////////////////////////////////

// Provisional placement for this

int	SendMeassurement() {
	return SendFrame("MEASU");
}

int	SendCoordinates() {
	return SendFrame("SHLOC");
}



// SOME I PLEMENTATION SAMPLES of NVM save and restore  Subspace records  

#define RAM



#ifdef	DISCO

const char	*filename = "flashfile.txt";
int		GenericReadElement(unsigned int n, unsigned char *to) {
	unsigned int base = CELLSIZE * n;  // por el CRLF
	int rc; 
	FILE *Flash = fopen(filename, "r");
	fseek (Flash, base, SEEK_SET);
//	printf ("Leyendo %d a %d sale %s\n", pos, pos+s, p);
	fgetc(Flash);
	rc = fread (to, CELLSIZE - 1, 1, Flash);
	fclose (Flash);
	return rc;
}

int		GenericWriteElement(unsigned int n, unsigned char *to) {
	unsigned int base = CELLSIZE * n;
	FILE *Flash = fopen(filename, "r+");
	if (Flash == NULL) 
		FILE *Flash = fopen(filename, "w+");
	fseek (Flash, base, SEEK_SET);
	fputc('\n', Flash); fwrite (to, CELLSIZE - 1 , 1, Flash);
	fclose (Flash);
	return 1;
}

#endif


#ifdef	RAM
static	char	swaparea[CELLSIZE*MAX_TABLE_SIZE];
int		GenericReadElement(unsigned int n, unsigned char *to) {
	unsigned int base = CELLSIZE * n;
	memcpy (to, swaparea+base, CELLSIZE);
	if (strchr((char *) to, '='))
	return 1;
	else
		return 0;

}

int		GenericWriteElement(unsigned int n, unsigned char *to) {
	unsigned int base = CELLSIZE * n;
	memcpy (swaparea+base, to, CELLSIZE);
	return 1;
}
#endif

