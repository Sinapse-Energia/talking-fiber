/*
 * utils.c
 *
 *  Created on: Jun 19, 2017
 *      Author: External
 */


#include 	<stdio.h>
#include 	<string.h>
#include 	<stdarg.h>
#include 	<stdlib.h>
#include	"utils.h"
#include	"NBinterface.h"		// any of them is implemented here
#include 	"southbound_ec.h"	// NVM access functions

////////////////////////  LED COLORING ////////////////////////////////
static int color;

int	getcolor(){
	return color;
}
void	_color(int col){
	switch (col) {
		case 0: (blueOFF, redOFF, greenOFF); return;
		case 1: (blueON,  redOFF, greenOFF); return;
		case 2: (blueOFF, redON,  greenOFF); return;
		case 3: (blueOFF, redOFF, greenON); return;
		case 4: (blueON,  redOFF, greenON); return;
		case 5: (blueON,  redON,  greenOFF); return;
		case 6: (blueOFF, redON,  greenON); return;
		case 7: (blueON,  redON,  greenON); return;
	}
}

void	Color(int col){
	color = col;
	_color(color);
}
/**
void	ROTATE(){
	color = (color + 1) % 8;
	Color(color);
}
**/

void	Blink() {
	static int	blink = 0;
	blink = ! blink;
	if (blink){
		_color(color);
	}
	else {
		_color (0);
	}
}

////////////////////  MISSED INET UNIX FUNCTIONS  //////////////////////////////////////

// ntohl convert from network to host endians .
// ONLY used from NTP call
uint32_t ntohl(uint32_t const net) {
    uint8_t data[4] = {};
    memcpy(&data, &net, sizeof(data));

    return ((uint32_t) data[3] << 0)
         | ((uint32_t) data[2] << 8)
         | ((uint32_t) data[1] << 16)
         | ((uint32_t) data[0] << 24);
}


////////////////////  SELF MANTAINED DATE & TIME  //////////////////////////////////////

// struct for itemized date & time
struct tm	ECdatetime;

// The Unix timestamp (seconds from epoch)
time_t		ECtimestamp;

int SetTimeStamp(uint32_t ts){
	ECtimestamp = ts;
	ECdatetime  = *localtime(&ECtimestamp);
	return 1;
}

int SetDateTimeNTP(const char *reply){
// #\r\n+QNTP: 0,"2018/01/29,20:56:09+04"\r\n#
	char	c;
	int		utcoffset = 0;
	struct tm	Temp_tm;
	time_t		TempTS;
	int n = sscanf (reply, "\r\nOK\r\n\r\n+QNTP: %c, \"%d/%d/%d,%d:%d:%d+%d\r\n\"", &c, &Temp_tm.tm_year, &Temp_tm.tm_mon, &Temp_tm.tm_mday, &Temp_tm.tm_hour, &Temp_tm.tm_min, &Temp_tm.tm_sec, &utcoffset);
	if ( n ==8 ){
		char utctxt[8];
		utcoffset = utcoffset/4;

		Temp_tm.tm_hour += utcoffset; // ??

		itoa(utcoffset, utctxt, 10);
		SetVariable ("TZONE", utctxt);

		// Adjust to real struct tm rules
		Temp_tm.tm_mon -= 1;
		Temp_tm.tm_year -= 1900;



		TempTS = mktime(&Temp_tm);
		if (TempTS >= ECtimestamp) {
			ECdatetime = Temp_tm;
			ECtimestamp = TempTS;
		}
		else {
			// Return to the past... Let it be as is
		}
		return 1;
	}
	else
		return 0;

}

// The function now can deal with the reply of CCLK command (NTP server), and QLTS command (Network time)
int SetDateTime(const char *reply){
	int			utcoffset = 0;
	char		Ttype[16];
	struct tm	Temp_tm = {0};
	time_t		TempTS = 0;
	int n = sscanf (reply+2, "+%s \"%d/%d/%d,%d:%d:%d+%d", Ttype, &Temp_tm.tm_year, &Temp_tm.tm_mon, &Temp_tm.tm_mday, &Temp_tm.tm_hour, &Temp_tm.tm_min, &Temp_tm.tm_sec, &utcoffset);
	if ( n ==8 ){
		if (!strcmp (Ttype, "CCLK:")) {
			Temp_tm.tm_year += 100;
			utcoffset = 1;
			SetVariable ("TZONE", "1");
		}
		if (!strcmp (Ttype, "QLTS:")){
			char utctxt[8];
			utcoffset = utcoffset/4;
			itoa(utcoffset, utctxt, 10);
			SetVariable ("TZONE", utctxt);
			Temp_tm.tm_year -= 1900;
		}
		// Adjust to real struct tm rules
		Temp_tm.tm_mon -= 1;
		Temp_tm.tm_hour += utcoffset;

		if (Temp_tm.tm_sec > 5) {
			Temp_tm.tm_sec -= 5;
		}
		else {
			Temp_tm.tm_sec = 0;
		}

		TempTS = mktime(&Temp_tm);
		if (TempTS >= ECtimestamp) {
			ECdatetime = Temp_tm;
			ECtimestamp = TempTS;
		}
		else {
			// Return to the past... Let it be as is
		}
		return 1;
	}
	else
		return 0;
};

const struct tm 	*GetTimeStructure(void){
	return &ECdatetime;
}

time_t GetTimeStamp(){
	return ECtimestamp;
}

time_t GetTimeStampAt(unsigned int h, unsigned int m, unsigned int s){
	// make a copy of the ECsatetime
	struct tm tx =  ECdatetime;
	tx.tm_hour = h;
	tx.tm_min = m;
	tx.tm_sec = s;
	time_t result = mktime(&tx);
	return result;
}


void AddSeconds(int n){
	UpdateTimeStamp(n);
}

void UpdateTimeStamp(int n){
//	struct tm work;
	ECtimestamp += n;
	ECdatetime  = *gmtime(&ECtimestamp);
}

//  Returns the tm year
unsigned int  GetYear(void){
	return ECdatetime.tm_year + 1900;
}
//  Returns the tm month
unsigned int  GetMonth(void){
	return ECdatetime.tm_mon + 1;
}
//  Returns the tm month
unsigned int  GetDay(void){
	return ECdatetime.tm_mday;
}


char	*UpdateSR(const char *x) {
	EvalSunTimes();
	return GetVariable("SUNRISE");
}

char	*UpdateSS(const char *x) {
	EvalSunTimes();
	return GetVariable("SUNSET");
}

// pre-action for "NOW" variable
char	*UpdateNow(const char *x) {
	static	char	TS[32];
	sprintf (TS, "%02d:%02d:%02d", ECdatetime.tm_hour, ECdatetime.tm_min, ECdatetime.tm_sec);
	return TS;
	;
}

// pre-action for "DATE_TIME" variable
char	*UpdateTS(const char *x) {
	return strDateTime()	;
}


// pre-action for "UNIXTS" variable
char	*UpdateUTS(const char *x) {
	static char strTS[20];
	itoa (GetTimeStamp(), strTS, 10);
	return strTS;
}


char	*strDateTime() {
	static	char	DT[64];
	sprintf (DT, "%d-%d-%d %02d:%02d:%02d",
					ECdatetime.tm_mday, ECdatetime.tm_mon+1, ECdatetime.tm_year+1900,
					ECdatetime.tm_hour, ECdatetime.tm_min, ECdatetime.tm_sec);
	return DT;
}


////////////////////  MPU Real Time Clock DATE & TIME  //////////////////////////////////////

extern RTC_TimeTypeDef structTimeRTC;
extern RTC_DateTypeDef structDateRTC;


int	UpdateRTC(void *ph){
	RTC_HandleTypeDef *phrtc = (RTC_HandleTypeDef  *)ph;

	structTimeRTC.Hours = ECdatetime.tm_hour;
	structTimeRTC.Minutes = ECdatetime.tm_min;
	structTimeRTC.Seconds = ECdatetime.tm_sec;

	structDateRTC.Date = ECdatetime.tm_mday;
	structDateRTC.Month = ECdatetime.tm_mon;
	structDateRTC.Year = ECdatetime.tm_year;

	MIC_Set_RTC (phrtc, &structTimeRTC, &structDateRTC, RTC_FORMAT_BIN);

	return 1;
}

int GetRTC(void *ph){
	RTC_HandleTypeDef *phrtc = (RTC_HandleTypeDef  *)ph;

	MIC_Get_RTC (phrtc, &structTimeRTC, &structDateRTC, RTC_FORMAT_BIN);

	ECdatetime.tm_hour = structTimeRTC.Hours;
	ECdatetime.tm_min = structTimeRTC.Minutes;
	ECdatetime.tm_sec = structTimeRTC.Seconds;

	ECdatetime.tm_mday = structDateRTC.Date;
	ECdatetime.tm_mon = structDateRTC.Month;
	ECdatetime.tm_year =  structDateRTC.Year;

	ECtimestamp = mktime(&ECdatetime);
	return 1;
}


#ifdef DEBUG
char	*Set_TimeOffset(const char *val) {
	int valor = atoi (val);
	UpdateTimeStamp(valor*60*60);
	return val;
}
char	*Set_TimePing(const char *val) {
	int value = atoi (val);
	if (value > 0)
		timeping = value;
	return val;
}
char	*Set_TimePoll(const char *val) {
	int value = atoi (val);
	if (value > 0)
		timepolling = value;
	return val;
}
#endif



// Helper funcyion to analyze if a host is a DNS or a IP address
// It returns 0 if the host is an IP address (999.999.999.999) and 1 otherwise
// ONLY used so far in M95
int isdns(unsigned char * host){
	int n1, n2, n3, n4;
	int n = sscanf((const char *)host, "%d.%d.%d.%d", &n1, &n2, &n3, &n4);
	if (n == 4)
		return 0;
	else
		return 1;
}

// Callback to parse the reply to AT+GMM Command and get the transceiver model
// ONLY used in MODEMFACTORY
int	 SetGPRSDevice(const char *txt){
	if (!strcmp(txt, "\r\nBG96\r\n\r\nOK\r\n"))
		SetVariable ("GPRSDEVICE", "BG96");
	else if (!strcmp(txt, "\r\nQuectel_M95\r\n\r\nOK\r\n"))
		SetVariable ("GPRSDEVICE", "M95");
	else
		SetVariable ("GPRSDEVICE", "M95");
	return 1;
}

// Helper function to parse the BG96 GPS data reply
// ONLY used from QueryPosition
int		GetPositioning (const char *cpgga){
//	char *cpgga ="+QGPSLOC: 114035.0,4032.2101N,00355.7221W,0.0,656.0,2,13.37,0.0,0.0,280118,00";
	static char	strUTS[20];
	char latitude[10], longitude[10];
	float f0;

//	int x = sscanf (cpgga, "\r\n+QGPSLOC: %f,%10s,%11s,", &f0, latitude, longitude);
// 	Provisional shortcut to avoid problems with the first match
// Mode 2 (decimal degrees..), for NW only so far
	int x = sscanf (cpgga+21, "%8s,%8s,", latitude, longitude);
//	int x = sscanf(cpgga, "+QGPSLOC: %[^,],%[^,],%[^,]", strUTS, latitude, longitude );
	if (x ==2) {
		// Update Coordenates
		SetVariable ("LATITUDE", latitude);
		SetVariable ("LONGITUDE", longitude);
		// Update Flag
		SetVariable ("GPS", "1");
		// mark timestamp
		itoa (GetTimeStamp(), strUTS, 10);
		SetVariable("GPSTS", strUTS);
		// Set proper Color
		Color(COL_CONNECTEDGPS);
		return 1;
	}
	else {
//		SetVariable ("LATITUDE", "-1");
//		SetVariable ("LONGITUDE", "-1");
//		SetVariable("GPSTS", strUTS);
//		itoa (GetTimeStamp(), strUTS, 10);
//		Color(COL_CONNECTEDGPS);

		// Update Flag
		SetVariable ("GPS", "0");
		// Set proper Color
		Color(COL_CONNECTED);
		// has to return 1 anyway because otherwise will disable to send de reply message
		return 1;  //
	}
}


#if defined(BUILD_M95)
// To handle the reply for M95...
int	 SetLocalIP(const char *txt){
	char	myIP[20];
	strcpy (myIP, txt+2);
	char *p = strchr(myIP, '\r');
	if (p)
		*p = 0;
	SetVariable ("LOCALIP", myIP);
	return 1;
}

// To handle the reply for M95...
int	SetIdSIM(const char *txt){
	char	SIM[40];
	strcpy (SIM, txt+2);
	char *p = strchr(SIM, '\r');
	if (p)
		*p = 0;
	SetVariable ("IDSIM", SIM);
	return 1;
}

#endif


#if defined(BUILD_BG96)
// To handle the reply for BG96...
int	 SetLocalIP2(const char *reply){
	char	myIP[20];
	int x = sscanf (reply+2, "+QIACT: 1,1,1,\"%s", myIP );
	if (x == 1) {
		char *p = strchr(myIP, '"');
		if (p)
			*p = 0;
		SetVariable ("LOCALIP", myIP);
	}
	return 1;
}

// To handle the reply for BG96...
int	SetIdSIM2(const char *txt){
	char	SIM[40];
	char *p;
	strcpy (SIM, txt+10);
	p = strchr(SIM, '\r');
	if (p)
		*p = 0;
	SetVariable ("IDSIM", SIM);
	return 1;
}

#endif


#if defined(BUILD_M95) || defined (BUILD_BG96)

int	SetIMEI(const char *txt){
	char	IMEI[32];  // supossed to have 16 chars
	strncpy (IMEI, txt+2, 20);  // provisional.... sometimes comes longer to null terminator
	char *p = strchr(IMEI, '\r');
	if (p)
		*p = 0;
	SetVariable ("IMEI", IMEI);
	SetVariable ("ID", IMEI);

	return 1;
}

int		ValidateReg(const char *reply){
	int	n;
	int stat;
	char	cmd[10];
	int x = sscanf (reply, "\r\n+%s %d,%d\r\n\r\nOK\r\n", cmd, &n, &stat );
	printf ("tengo %s, %d, %d", reply, n, stat);
	if (x == 3) {
		if ( (n == 0) && ((stat == 1) || (stat == 5)))
				return 1;
		if ( (n == 0) && (stat == 2))
			return -1; //it lasts.... retry again
		else
			return 0;
	}
	else
		return 0;

	return 1;
}
#endif



int		SetGeneric(const char *txt){
	const char *tmp = txt;

	return 1;
}
int		SetState(const char *txt){
	const char *tmp = txt;

	return 1;
}

#if defined(BUILD_RM08)
int		SetMAC (const char *txt){
	char	mac[40];
	strcpy (mac, txt+8);  // provisional.... sometimes comes longer to null terminator
	char *p = strchr(mac, ',');
	if (p) {
		*p = 0;
		SetVariable ("MAC", mac);
		SetVariable ("ID", mac);
	}
	return 1;
}

/** not used anymore
int		ValidateStat(const char *reply){
	int n, stat;
	int x = sscanf (reply, "at+RNStat%d=%d\r\n", &n, &stat );
	if (x == 2) {
		if ( stat == 4 )
				return 1;
		else
			return -1; //it lasts.... retry again
	}
	else
		return 0;
}
**/
#endif

// Helper function to accumulate trace until it can be shown
// Small buffer allocation until verify how much memory is available
char	pretrbuf[500] = "";
int		pretrace(char *texto,...) {
	va_list	  ap;

	va_start	  (ap, texto);
	vsprintf (pretrbuf+ strlen(pretrbuf), texto, ap);
	return 1;
}





#define STORESIZE 512
char	store[STORESIZE];


int	SaveALL(){
	const char	*listofvars[] = {
// CONNECTION
			"IP","PORT", "USER", "PSWD", "SEC", "LIP", "LPORT","LUSER", "LPSWD","LSEC",
// TRANSPORT
#if defined (GPRS_TRANSPORT)
			"APN", "LAPN",
#endif
// LIGHTING
			"PWM",
			"MPERIOD", "MPTIMES", "MPBEGIN", "MPEND",
// GEO
			"LONGITUDE", "LATITUDE",
// BL
			"ID", "UPDFW", "UPDFW_COUNT", "FWNAME", "FWVERSION", "UPDFW_PROTO", "UPDFW_HOST", "UPDFW_PORT",	 "UPDFW_NAME", "UPDFW_ROUTE", "UPDFW_USER", "UPDFW_PSWD",
// MISC
			"SYNCHTS", "BOARD",
			NULL
	};
	int i = 0;
	store[0] = 0;
	while (listofvars[i]) {
		char item[128];
		const char *name = listofvars[i];
		char *value = GetVariable(name);
		if (value){
			sprintf (item, "%s=%s;",  name, value );
			if (strlen(store) + strlen(item) < STORESIZE){
				strcat(store, item);
				i++;
			}
			else {
				break;
			}
		}
		else
			i++;
	}
	MIC_Flash_Memory_Write((const uint8_t *) store, (uint32_t)strlen(store)+ 1);
	return 1;
}


int	SaveConnParams(){
	return SaveALL();
}

int	SaveAppParams(){
	return SaveALL();
}

int	SaveBLParams(){
	return SaveALL();
}


int RecConnParams(){
	MIC_Flash_Memory_Read( (const uint8_t *) store, sizeof(store));
	WriteData((char *) store);
}

int RecAppParams(){
	MIC_Flash_Memory_Read( (const uint8_t *) store, sizeof(store));
	WriteData((char *) store);
}


// Utility function to convert 4 chars in IEEE-754 Floating Point to a double
// ONLY used in CAN reads
double 	ieee2real (unsigned char data[]){
	int sign;
	int mantissa;
	unsigned char exp;
	double result;

	sign = data[3] >> 7;
	exp = ((data[3] & 0x7F) << 1 ) |  ((data[2] & 0x80) >> 7);
	mantissa = data[0] | data[1] << 8 | (data[2] & 0x7F) << 16;

	result = (sign?-1:1) * (double)(mantissa + 0x7FFFFF)/(double) 0x7FFFFF   * pow (2, exp - 127);

	return result;
}


#include <stdlib.h>
long	getHSsize (void){
	char	stack = 0;
	char	*heap = malloc(100);
	unsigned long size =   &stack - heap;
	free	(heap);
	return size;
}


char *textCert =
"-----BEGIN CERTIFICATE-----\n"
"MIIEATCCAumgAwIBAgIJALFjqVBg1LHlMA0GCSqGSIb3DQEBCwUAMIGWMQswCQYD"
"VQQGEwJFUzEPMA0GA1UECAwGTWFkcmlkMQ8wDQYDVQQHDAZNYWRyaWQxEDAOBgNV"
"BAoMB1NpbmFwc2UxFDASBgNVBAsMC0RldmVsb3BtZW50MQ0wCwYDVQQDDARNUVRU"
"MS4wLAYJKoZIhvcNAQkBFh9yYWZhLmFsY2FpZGVAc2luYXBzZWVuZXJnaWEuY29t"
"MB4XDTE3MDkxNDExMjY0MloXDTIyMDkxNDExMjY0MlowgZYxCzAJBgNVBAYTAkVT"
"MQ8wDQYDVQQIDAZNYWRyaWQxDzANBgNVBAcMBk1hZHJpZDEQMA4GA1UECgwHU2lu"
"YXBzZTEUMBIGA1UECwwLRGV2ZWxvcG1lbnQxDTALBgNVBAMMBE1RVFQxLjAsBgkq"
"hkiG9w0BCQEWH3JhZmEuYWxjYWlkZUBzaW5hcHNlZW5lcmdpYS5jb20wggEiMA0G"
"CSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDCfuQkv6LrQoyynZMi4jsmGgQ32geK"
"5v7fYWCH3dB7f6KVDozRraFW4a9vqkhQEpwmMcXZ6+aI/NaFW0ifc16BKdXXMslw"
"aJmQIBbBgrpXCYCmED7v4h8bBYNoA+/yIqo+EAYfLSYwMvM/9D7n2x28LiytuNsf"
"Nsga6tToN2rIIfsMrCRxBT4Ex8NKlpyRF0EO29jbZFrlUXp0wqowZFobgM5mgNjG"
"MSYzPJFvlF/Hlp5XL2MM5nbgWWYgGk6w2Ep79gR4a32Np0Gq8C3r7avv29num6Mu"
"3pKp+wiBUtZzafNHGqbRClcQEUZk7E6K/yXRqEfOdBO1RZE99/orO7vTAgMBAAGj"
"UDBOMB0GA1UdDgQWBBT4A15SVpWhKsWWWD9nWnn91bPVDDAfBgNVHSMEGDAWgBT4"
"A15SVpWhKsWWWD9nWnn91bPVDDAMBgNVHRMEBTADAQH/MA0GCSqGSIb3DQEBCwUA"
"A4IBAQBdkrJkQ5f6wA+FoofKy19GewXWtn6UTnE1U/dVehJmB8WXxnLZAqH7aPq8"
"kFN+MsAXa6qaLe9C52ne75bop8rzLE73fcWjRtfbJr5fovk32jF20VL0QhjRfwko"
"pRqqrYBoDMzpvyzPaJnPEcqyVf+MHVPqaRdnXz/9Qqumu9yz09XVBPL6KkskpQgj"
"oZs2jpCPSDhAavzNTJzQuJmVYj5eYTBBMmg8y9UENqvmQsUPFx8lAcWF/BcKLBH5"
"BMCnZ9MYAwA4kwMpv4/yeKfDShF9RbG2U6rSbW0Rv4bpS5IuKBgIyWdfJf3Qcb1c"
"IsqfjZ5GlW+Gih34ZxlzNYEAjm/j\n"
"-----END CERTIFICATE-----";


unsigned char	*getCertificateTxt(size_t *lcert){
	*lcert = strlen(textCert);
	return textCert;
}




