/*
 * Application.cpp
 *
 *  Created on: 26 sept. 2017
 *      Author: juanra
 */



#include "context.h"
#include "NBinterface.h"
#include "TimeEvents.h"
#include "utils.h"


#include "Application.h"

#include "southbound_ec.h"

// SENSORS
int 	ASN_READ() {
	int pin = atoi(GetVariable("ASNPIN"));
	char	strvalue[16];

	double value = read_analog_signal(pin);

	int ip = (int) value;
	int fracp = (int) ((value - ip)* 100 );

	sprintf (strvalue, "%d.%02d", ip, fracp);
	SetVariable("ASVALUE", strvalue);
	SetVariable("ASFLAG", "-1");

	itoa (GetTimeStamp(), strvalue, 10);
	SetVariable("UNIXTS", strvalue);


	return 1;
}

int 	DSN_READ() {
	int pin = atoi(GetVariable("DSNPIN"));
	char	strvalue[16];

	int  value = (int) read_digital_signal(pin);
	itoa (value, strvalue, 10);
	SetVariable("DSVALUE",  strvalue);
	SetVariable("DSFLAG", "-1");

	itoa (GetTimeStamp(), strvalue, 10);
	SetVariable("UNIXTS", strvalue);
	return 1;
}

static int traza = 0;
#if defined(BUILD_CANBUS)
// CAN ACTIONS
int		CAN_configure (){
	// Recover variables
	CVarint		*Interface =	GetIntVar("CAN_ID");
	CVarint		*Baudrate =		GetIntVar("CAN_BAUDRATE");
	CVarint		*Mode =			GetIntVar("CAN_MODE");
	CVarlist 	*Address 	= 	GetListVar ("CAN_R_ADDRESS");

	struct {
		int				value;
		EnumCANBitRate 	sym;
	} baudlist[] = {
			62500,		EnBitRate_62_5K,
			125000,		EnBitRate_125K,
			250000,		EnBitRate_250K,
			500000,		EnBitRate_500K,
			1000000,	EnBitRate_1000K
	};
	// evaluate them
	int interface =	Interface->Value();
	int baudrate =	Baudrate->Value();
	int mode =	Mode->Value();
	uint32_t	addresses[10];
	unsigned int i, j;
	unsigned n = sizeof(baudlist)/sizeof(baudlist[0]);
	for (j = 0; j < n; j++){
		if (baudrate == baudlist[j].value)
			break;
	}
	if (j == n) {
		SetVariable ("CAN_RESULT", "FALSE");
	}
	else {
		i = 0;
		while (i < Address->size()) {
			CVarstring *add = (CVarstring * )Address->Item(i);
			addresses[i] = strtol(add->Value(), NULL, 16);

			i++;
		}
		addresses[i] = 0;
		int result = config_can (baudlist[j].sym, true, addresses);

		if (result)
			SetVariable ("CAN_RESULT", "TRUE");
		else
			SetVariable ("CAN_RESULT", "FALSE");
	}

	return 1;
}
int		CAN_Send (){
	// Recover variables
	CVarint		*Interface =	GetIntVar("CAN_ID");
	CVarstring	*Address =		GetStringVar("CAN_W_ADDRESS");
	CVarstring	*Format =		GetStringVar("CAN_FORMAT");
	CVarstring 	*Message 	= 	GetStringVar ("CAN_W_MESSAGE");
	// evaluate them
	int interface =	Interface->Value();
	const char *format = Format->Value();
	uint32_t address =	strtol(Address->Value(), NULL, 16);
	uint8_t message[16] = {0};
	strcpy ((char *) message,  Message->Value());
	char pair[3] = {0};
	int i = 0;
	uint8_t xmessage[16];
	while (i < 8) {
		if (message[2*i]){
			strncpy(pair, (char *) message + 2*i, 2);
			xmessage[i] = strtol(pair, NULL, 16);
		}
		else {
			xmessage[i] = 0;
		}
		i++;
	}
	xmessage[i] =0;




	int result = write_can_message(address, xmessage);

	if (result)
		SetVariable ("CAN_RESULT", "TRUE");
	else
		SetVariable ("CAN_RESULT", "FALSE");
	return 1;
}

int		CAN_Read (){
	// Recover variables
	CVarint		*Interface =	GetIntVar("CAN_ID");
	CVarstring	*Format =		GetStringVar("CAN_FORMAT");
	CVarint 	*Nmessages 	= 	GetIntVar ("CAN_R_NUMBER");
	// evaluate them
	int interface =	Interface->Value();
	const char *format = Format->Value();
	int nmessages = Nmessages->Value();


	CVarlist 	*Messages 	= 	GetListVar ("CAN_R_MESSAGES");
	uint32_t	address;
	uint8_t	payload [9];
	char messages[512] = "";  // for having all the stuff
	int i = 0;
		while (i < nmessages) {
		if (traza)
			tprintf (hmqtt, "Before call read_can_bus %d-sima", i);

		int result  = read_can_bus(&address, payload);
		if (traza)
			tprintf (hmqtt, "Coming from read_can_bus with result %d", result);

		if (result == CQ_ERROR_OK){
			if (0) {
				int trama = payload[0];
				double value = ieee2real(payload+1);
				int ip = (int) value;
				int fp = (value - ip) * 1000;
				if (0)
					tprintf (hmqtt, "Read %d: Value %d.%03d  ", i , ip, fp);
				sprintf  (messages+strlen(messages), "%04lX;%d.%03d;", address, ip, fp);

			}
			else {
				payload[8] = 0;
				if (traza)
					tprintf (hmqtt, "Read %d: address %04lX, message %02X%02X%02X... with rc %d", i, address, payload[0],payload[1],payload[2], result);
				sprintf  (messages+strlen(messages), "%04lX;%02X%02X%02X%02X%02X%02X%02X%02X;", address, payload[0],payload[1],payload[2],payload[3],payload[4],payload[5],payload[6],payload[7]);
			}
			i++;
		}
		else {
			if (traza)
				tprintf (hmqtt, "Quiting read with %d", i);
			break;
		}
	}
	Messages->write((const char *)messages);

	return 1;
}
#endif


#if defined(GPRS_TRANSPORT)
int		ManageAPNS(){
	char	result[64];
	char *apnname = GetVariable("APNNAME");
	char *apnuser = GetVariable("APNUSER");
	char *apnpswd = GetVariable("APNPSWD");

	sprintf (result, "\"%s\",\"%s\",\"%s\"", apnname, apnuser, apnpswd);

	SetVariable("APN", result);
	return 1;
}

#endif

int		SetProgramm() {
	CVarlist *all = GetListVar ("PROGR"); 
	unsigned int i = 0;
	CTimeEvent::Clear("PR");
	while (i < all->size()) {
		CVartuple *step = (CVartuple * )all->Item(i);
		CVarint  *t1 = (CVarint *) step->Item(0);
		CVartime *t2 = (CVartime *) step->Item(1);

		if (t1->Value() >= 0) { // skips -1 legacy padding
			unsigned int secondsaday = 24 * 60 * 60;	// number of seconds of a day
			unsigned int maxtimes = (unsigned int) -1;  // a lot of days... maybe could be enough a month?


			CTimeEvent *timer = new CTimeEvent("PR", t2->Timestamp(), 1 , maxtimes, secondsaday, ApplyDimming);
			CTimeEvent::Add(timer);
		}
		i++;
	}
	return i;
}


int		SetDimming() {
	// reset any active range, i.e.  "PR" timers
	CTimeEvent::Clear("PR");
	SetVariable("PROGR", "");
	// trigger the funct to save in NVM
	SaveConnParams();
	return 1;

}


int		SetPeriodicsGPS() {
	// Recover commmand parameter
	CVarint		*Periodo =	GetIntVar("LOCPERIOD");
	// evaluate them
	int periodo =	Periodo->Value();
	long now = 		GetTimeStamp();



	if (periodo >0) {
		CTimeEvent *timer = new CTimeEvent("LOC", now, 1 , (int ) -1 , periodo, SendCoordinates);
		int dx = CTimeEvent::Clear("LOC");
		CTimeEvent::Add(timer);
		return 1;

	}
	else
		return 0;
}


int		SetPeriodics() {
	// Recover commmand parameters
	CVarint		*Periodo =	GetIntVar("MPERIOD");
	CVarint		*Veces =	GetIntVar("MPTIMES");
	CVartime	*Fecha1 =	GetTimeVar("MPBEGIN");
	CVartime	*Fecha2 =	GetTimeVar("MPEND");

	// evaluate them
	int periodo =	Periodo->Value();
	int veces =		Veces->Value();
	unsigned long TS1 =		Fecha1->Timestamp();
	unsigned long TS2 =		Fecha2->Timestamp();  //.....//
	unsigned long now = 		GetTimeStamp();

	int number1; 	// number of periods, after HM1 and HM2
	unsigned int number;	// final number of periods
	if (periodo && veces){
		if (TS2 < TS1){
			// First Time bigger than second....
			// IGNORE COMMAND:  return 0;
			return -1;
		}

		SaveAppParams();


		if (TS1 < now) { // if the time slice is in progress
			//  TRUNCATE  TS1 = now ;
			TS1 = now;
		}
		// Calculate number of periods
		number1 = (TS2 -TS1)/(periodo * 60);

		if (number1 > 0) {
			if (veces > 0 && veces < number1) {
				number = veces;
			}
			else {
				number = number1;
			}
		//	number = ((veces < -0) || number1 < veces)? number1: veces;


			CTimeEvent *timer = new CTimeEvent("PM", TS1, 1 , number, periodo * 60, SendMeassurement);
			int dx = CTimeEvent::Clear("PM");
			CTimeEvent::Add(timer);
			return 1;
		}
		else
			return 0;
	}
	else
		return -1;
}


// Helper Function to get the Dimm at t timestamp
int GetDimming (long t) {
	CVarlist *progr = GetListVar("PROGR");
	unsigned short i = 0;
	unsigned int lastvalue = -1;
	while (i < progr->size()) {
		CVartuple *step = (CVartuple * ) progr->Item(i);
		CVarint  *st1 = (CVarint *) step->Item(0);
		CVartime *st2 = (CVartime *) step->Item(1);
		if (st2->Timestamp() > t){
			break;
		}
		else {
			lastvalue = st1->Value();
			i++;
		}
	}			
	return lastvalue;
}

//	action to be used for set the timmer to the programmed value 
int		ApplyDimming	(void) {
	// get the Dimming for rigth now
	int value = GetDimming(GetTimeStamp());
	tprintf (hmqtt, "Ready to apply Dimming of %d", value);
	char	txt[8];
	itoa(value, txt, 10);
	SetVariable("PWM", txt);
	return value;
}



#include "Transceiver.h"

int		QueryPosition() {
	Transceiver *trx = (Transceiver *) gtransceiver;
	int x = trx->ExecuteCommand("AT+QGPSLOC=2\r", NULL, GetPositioning );
	return x;
}



int		ExecCmd() {
	char	cmd[128];
	char	reply[128];
	char	*p = GetVariable("ATCMD");
	Transceiver *trx = (Transceiver *) gtransceiver;
	if (p) {
		sprintf (cmd, "%s\r", p);

		int x = trx->ExecuteCommand(cmd, reply, NULL);
		SetVariable("ATREPLY", reply);
		printf ("EL cmd %s responde %s \n", cmd, reply);
		return 1;
	}
	else
		return 0;
}

extern RTC_HandleTypeDef hrtc;

int	SynchronizeTime () {
	Transceiver *trx = (Transceiver *) gtransceiver;
	int i = trx->SynchronizeTime();

	if (i > 0) { // Got time from external source..
		UpdateRTC( &hrtc); // Adjust RTC
		time_t	now = GetTimeStamp();
		char strTS[20];
		itoa (now, strTS, 10);
		SetVariable("SYNCHTS", strTS); // last Synch stamp
#ifdef DEBUG
		tprintf (hmqtt, "Synchronized at %s", strDateTime());
#endif
		SaveAppParams();
		return 1;
	}
	else {
		return -1;
	}
}

int	TestSynchonization () {
	long t1 = atol(GetVariable("SYNCHTS"));
	long t2 = GetTimeStamp();
	if ( t1 == 0 ||		// first time..
		(t2 > t1 + TIMESYNCHRONIZE*3600)  	// Is in hours
		|| (t2 < t1)){ 						// just if the RTC is outdated
		return SynchronizeTime();
}
	else {
#ifdef DEBUG
		tprintf (hmqtt, "Nothing to do yet %ld ", (t2-t1));
#endif
		return 0;
	}
}



/****************************************************************************************
	Provisional functions to get lat/long form notational representation to signed degrees
	By now to support only mode 2 in QGPSLOC command
*****************************************************************************************/

double getLatitude (char *Lon){
	double val;
	val = atof(Lon);
	return val;
}

double getLongitude (char *Lon){
	double val;
	val = atof(Lon);
	return val;
}

#include "SunSet.h"


/************************************************
 Action to call to evaluate SUNRISE & SUNSET (604;)
*************************************************/
int EvalSunTimes() {
	double latitude = getLatitude(GetVariable("LATITUDE"));
	double longitude = getLongitude(GetVariable("LONGITUDE"));
	int	timezone = atoi(GetVariable("TZONE"));
	char	fmttime[16];

	if (longitude < 0.0)
		longitude = -longitude;

	SunSet here(latitude, longitude, timezone);

	here.setCurrentDate(GetYear(), GetMonth(), GetDay());

	int sunrise = here.calcSunrise();
	int sunset = here.calcSunset();

	sprintf (fmttime, "%02d:%02d", sunrise / 60, sunrise % 60 );
	SetVariable("SUNRISE", fmttime);

	sprintf (fmttime, "%02d:%02d", sunset / 60, sunset % 60 );
	SetVariable("SUNSET", fmttime);
	return 1;
}



