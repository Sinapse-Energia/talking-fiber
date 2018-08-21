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
#include "Frames.h"


#include "Application.h"

#include "southbound_ec.h"

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

int	SendTF() {
#ifdef CONNECT_ONLY_TO_SEND
    HAL_GPIO_WritePin(EX_ENABLE_GPRS_BATTERY_GPIO_Port, EX_ENABLE_GPRS_BATTERY_Pin, GPIO_PIN_SET);
    FULLNEWCONN();
#endif
	int rc = SendFrame("L3TFR");
#ifdef CONNECT_ONLY_TO_SEND
	hmqtt = 0;
	HAL_GPIO_WritePin(EX_ENABLE_GPRS_BATTERY_GPIO_Port, EX_ENABLE_GPRS_BATTERY_Pin, GPIO_PIN_RESET);
#endif
	return rc;
}

int SendTFAl() {
#ifdef CONNECT_ONLY_TO_SEND
    HAL_GPIO_WritePin(EX_ENABLE_GPRS_BATTERY_GPIO_Port, EX_ENABLE_GPRS_BATTERY_Pin, GPIO_PIN_SET);
    FULLNEWCONN();
#endif

    int rc = -1;
    int mv = atoi(GetVariable("TFVOL"));
    if (mv < atoi(GetVariable("TFATH")))
    {
        char out[64];
        snprintf(out, 128, "L3_TF_ALERT;%s;%i;%s;",
                GetVariable("ID"), mv, GetVariable("TFVTS"));
        rc = Publish(out);
    }

#ifdef CONNECT_ONLY_TO_SEND
    hmqtt = 0;
    HAL_GPIO_WritePin(EX_ENABLE_GPRS_BATTERY_GPIO_Port, EX_ENABLE_GPRS_BATTERY_Pin, GPIO_PIN_RESET);
#endif
    return rc;
}

int SetTFPeriodic() {
#ifdef ENABLE_PERIODIC
	CTimeEvent *timer = new CTimeEvent("TFP", GetTimeStamp(), 1 , -1,
	        60*atoi(GetVariable("TFPER")), SendTF);
	int dx = CTimeEvent::Clear("TFP");
	CTimeEvent::Add(timer);
#endif
#ifdef ENABLE_ALERT
    CTimeEvent *timer2 = new CTimeEvent("TFA", GetTimeStamp(), 1 , -1,
            60*atoi(GetVariable("TFALM")), SendTFAl);
    int dx2 = CTimeEvent::Clear("TFA");
    CTimeEvent::Add(timer2);
#endif
}

