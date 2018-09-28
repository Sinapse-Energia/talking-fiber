/*
 * utils.c
 *
 *  Created on: Jun 19, 2017
 *      Author: External
 */


#include 	<stdio.h>
#include 	<stdarg.h>
#include 	<string.h>
#include 	<stdbool.h>
#include 	<math.h>
#include	"utils.h"
#include	"Definitions.h"
#include	"NBinterface.h"

#ifdef TIME_WITH_RTC

#include "rtc.h"

#endif

struct tm	ECdatetime;

long		ECtimestamp;

int SetDateTime(const char *NTP) {

	sscanf (NTP+2, "%d/%d/%d,%d:%d:%d", &ECdatetime.tm_year, &ECdatetime.tm_mon,
			&ECdatetime.tm_mday, &ECdatetime.tm_hour, &ECdatetime.tm_min, &ECdatetime.tm_sec);

#ifdef TIME_WITH_RTC

	RTC_DateTypeDef dt;
	RTC_TimeTypeDef tt;

	dt.Year = ECdatetime.tm_year;
	dt.Month = ECdatetime.tm_mon;
	dt.Date = ECdatetime.tm_mday;
	dt.WeekDay = RTC_WEEKDAY_MONDAY;

	tt.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	tt.Hours = ECdatetime.tm_hour;
	tt.Minutes = ECdatetime.tm_min;
	tt.Seconds = ECdatetime.tm_sec;
	tt.StoreOperation = RTC_STOREOPERATION_RESET;
	tt.TimeFormat = RTC_FORMAT_BIN;

	HAL_RTC_SetDate(&hrtc, &dt, RTC_FORMAT_BIN);
	HAL_RTC_SetTime(&hrtc, &tt, RTC_FORMAT_BIN);

	ECdatetime.tm_year += 2000;
	ECtimestamp = mktime(&ECdatetime);

#else

	ECdatetime.tm_year += 2000;
	ECdatetime.tm_hour += UTC_OFFSET;

	if (ECdatetime.tm_sec > 5) {
		ECdatetime.tm_sec -= 5;
	}
	else {
		ECdatetime.tm_sec = 0;
	}
//	sscanf (NTP+2, "%d/%d/%d", &ECdatetime.tm_year, &ECdatetime.tm_mon, &ECdatetime.tm_mday);
	ECtimestamp = ((ECdatetime.tm_hour * 60 + ECdatetime.tm_min) * 60 ) + ECdatetime.tm_sec;

#endif

	return ECtimestamp;
};

char* GetDateTime(void) {

	static char buf[24];

#ifdef TIME_WITH_RTC

	RTC_DateTypeDef dt;
	RTC_TimeTypeDef tt;

	HAL_RTC_GetTime(&hrtc, &tt, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &dt, RTC_FORMAT_BIN);

	ECdatetime.tm_year = dt.Year + 2000;
	ECdatetime.tm_mon = dt.Month;
	ECdatetime.tm_mday = dt.Date;

	ECdatetime.tm_hour = tt.Hours;
	ECdatetime.tm_min = tt.Minutes;
	ECdatetime.tm_sec = tt.Seconds;

#endif

	sprintf(buf, "%04d/%02d/%02d,%02d:%02d:%02d",
			ECdatetime.tm_year, ECdatetime.tm_mon, ECdatetime.tm_mday,
			ECdatetime.tm_hour, ECdatetime.tm_min, ECdatetime.tm_sec);
	return &buf[0];
}

long GetTimeStamp() {

#ifdef TIME_WITH_RTC

	RTC_DateTypeDef dt;
	RTC_TimeTypeDef tt;

	HAL_RTC_GetTime(&hrtc, &tt, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &dt, RTC_FORMAT_BIN);

	ECdatetime.tm_year = dt.Year + (2000 - 1900);
	ECdatetime.tm_mon = dt.Month - 1;
	ECdatetime.tm_mday = dt.Date;

	ECdatetime.tm_hour = tt.Hours;
	ECdatetime.tm_min = tt.Minutes;
	ECdatetime.tm_sec = tt.Seconds;

	ECtimestamp = mktime(&ECdatetime);

#endif

	return ECtimestamp;
}

void AddSeconds(int n) {

#ifndef TIME_WITH_RTC

	ECtimestamp += n;
	ECdatetime.tm_sec += n;
	if (ECdatetime.tm_sec > 59) {
		ECdatetime.tm_min += ECdatetime.tm_sec / 60;
		ECdatetime.tm_sec = ECdatetime.tm_sec % 60;
		if (ECdatetime.tm_min > 59) {
			ECdatetime.tm_hour += ECdatetime.tm_min / 60;
			ECdatetime.tm_min = ECdatetime.tm_min % 60;
			if (ECdatetime.tm_hour > 23) {
				ECdatetime.tm_mday += ECdatetime.tm_hour / 24;
				ECdatetime.tm_hour = ECdatetime.tm_hour % 24;
			}
		}
	}

#endif

}

char	*strDateTime() {

	static	char	DT[64];

#ifdef TIME_WITH_RTC

	RTC_DateTypeDef dt;
	RTC_TimeTypeDef tt;

	HAL_RTC_GetTime(&hrtc, &tt, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &dt, RTC_FORMAT_BIN);

	ECdatetime.tm_year = dt.Year + 2000;
	ECdatetime.tm_mon = dt.Month;
	ECdatetime.tm_mday = dt.Date;

	ECdatetime.tm_hour = tt.Hours;
	ECdatetime.tm_min = tt.Minutes;
	ECdatetime.tm_sec = tt.Seconds;

#endif

	sprintf (DT, "%d-%d-%d %02d:%02d:%02d",
					ECdatetime.tm_mday, ECdatetime.tm_mon, ECdatetime.tm_year,
					ECdatetime.tm_hour, ECdatetime.tm_min, ECdatetime.tm_sec);
	return DT;
}

char	*getHour(void) {

	static char hourBuf[8];

#ifdef TIME_WITH_RTC

	RTC_DateTypeDef dt;
	RTC_TimeTypeDef tt;

	HAL_RTC_GetTime(&hrtc, &tt, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &dt, RTC_FORMAT_BIN);

	ECdatetime.tm_year = dt.Year + 2000;
	ECdatetime.tm_mon = dt.Month;
	ECdatetime.tm_mday = dt.Date;

	ECdatetime.tm_hour = tt.Hours;
	ECdatetime.tm_min = tt.Minutes;
	ECdatetime.tm_sec = tt.Seconds;

#endif

	sprintf(hourBuf, "%d", ECdatetime.tm_hour);
	return hourBuf;
}

char	*getMinute(void) {

	static char minuteBuf[8];

#ifdef TIME_WITH_RTC

	RTC_DateTypeDef dt;
	RTC_TimeTypeDef tt;

	HAL_RTC_GetTime(&hrtc, &tt, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &dt, RTC_FORMAT_BIN);

	ECdatetime.tm_year = dt.Year + 2000;
	ECdatetime.tm_mon = dt.Month;
	ECdatetime.tm_mday = dt.Date;

	ECdatetime.tm_hour = tt.Hours;
	ECdatetime.tm_min = tt.Minutes;
	ECdatetime.tm_sec = tt.Seconds;

#endif

	sprintf(minuteBuf, "%d", ECdatetime.tm_min);
	return minuteBuf;
}

void getDate(int* outDay, int* outMonth, int* outYear) {

#ifdef TIME_WITH_RTC

	RTC_DateTypeDef dt;
	RTC_TimeTypeDef tt;

	HAL_RTC_GetTime(&hrtc, &tt, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &dt, RTC_FORMAT_BIN);

	ECdatetime.tm_year = dt.Year + 2000;
	ECdatetime.tm_mon = dt.Month;
	ECdatetime.tm_mday = dt.Date;

	ECdatetime.tm_hour = tt.Hours;
	ECdatetime.tm_min = tt.Minutes;
	ECdatetime.tm_sec = tt.Seconds;

#endif

	if (outDay != NULL) *outDay = ECdatetime.tm_mday;
	if (outMonth != NULL) *outMonth = ECdatetime.tm_mon;
	if (outYear != NULL) *outYear = ECdatetime.tm_year;
}

// Pending : to validate the IP format is correct
int	 SetLocalIP(const char *txt){
	char	myIP[20];
	strcpy (myIP, txt+2);
	char *p = strchr(myIP, '\r');
	if (p)
		*p = 0;
	SetVariable ("LOCALIP", myIP);
	return 1;
}

// Pending : to validate the SIM format is correct
int	SetIdSIM(const char *txt){
	char	SIM[40];
	strcpy (SIM, txt+2);
	char *p = strchr(SIM, '\r');
	if (p)
		*p = 0;
	SetVariable ("IDSIM", SIM);
	return 1;
}

int	SetIMEI(const char *txt){
	char	IMEI[32];  // supossed to have 16 chars
	strncpy (IMEI, txt+2, 20);  // provisional.... sometimes comes longer to null terminator
	char *p = strchr(IMEI, '\r');
	if (p)
		*p = 0;
	SetVariable ("IMEI", IMEI);

	// Set ID to last 6 IMEI digits
	size_t lenIMEI = strlen(IMEI);
	if (lenIMEI >= 6)
	{
		char* last6IMEIdigits = &IMEI[lenIMEI-6];
		SetVariable("ID", last6IMEIdigits);
	}

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



int		SetGeneric(const char *txt){
	//const char *tmp = txt;

	return 1;
}

// Helper function to accumulate trace until it can be shown
// Small buffer allocation until verify how much memory is available
char	pretrbuf[500] = "";
int		pretrace(char *texto,...) {
	va_list	  ap;

	va_start	  (ap, texto);
	vsprintf (pretrbuf+ strlen(pretrbuf), texto, ap);
	return 1;
}

//
//
// 	This is a PROVISIONAL function intended to verify the message manipulation, without having to depend on message reception
//	Uses a arbitrary sequence of commands in a circular list....
//	Each tiem de function is called, returns the next message form the list

char	*GetLocalMessage(int h, char *buffer, int maxsize){
	static  	char 	*messages[] = {
	  			"1;",
				"3;20;",
				"1;",
				"3;60;",
				"1;",
				"3;90;",
				"1;",
				"3;0;"
	  	};
	static int i = 0;
	char	*result = messages[i];
	i = (i+1)%(sizeof(messages)/sizeof(*messages));
	if (strlen(result) < maxsize)
		strcpy (buffer, result);
	else
		strncpy(buffer, result, maxsize);
	// HAL_Delay(1000);
	return buffer;
}


#include "NBinterface.h"
#include "southbound_ec.h"


struct st_cparams {
	char		ip[128];
	char		port[128];
	char		user[128];
	char		pswd[128];
	char		lip[128];
	char		lport[128];
	char		luser[128];
	char		lpswd[128];
	char		dstat[128];
} 	connection_params;



int	SaveConnParams(){
//	char *p;
//	p = GetVariable("IP");
	strcpy (connection_params.ip , GetVariable("IP"));
	strcpy (connection_params.port , GetVariable("PORT"));
	strcpy (connection_params.user , GetVariable("USER"));
	strcpy (connection_params.pswd , GetVariable("PSWD"));
	strcpy (connection_params.lip , GetVariable("LIP"));
	strcpy (connection_params.lport , GetVariable("LPORT"));
	strcpy (connection_params.luser , GetVariable("LUSER"));
	strcpy (connection_params.lpswd , GetVariable("LPSWD"));
	strcpy (connection_params.dstat , GetVariable("DSTAT"));

	MIC_Flash_Memory_Write((const uint8_t *) &connection_params, (uint32_t)sizeof(connection_params));
	return 1;
}

int RecConnParams(){
	MIC_Flash_Memory_Read( (const uint8_t *) &connection_params, sizeof(connection_params));

	if ( (*(connection_params.ip) != 0xFF) ){
		SetVariable("IP", connection_params.ip);
		SetVariable("PORT", connection_params.port);
		SetVariable("USER", connection_params.user);
		SetVariable("PSWD", connection_params.pswd);
		SetVariable("LIP", connection_params.lip);
		SetVariable("LPORT", connection_params.lport);
		SetVariable("LUSER", connection_params.luser);
		SetVariable("LPSWD", connection_params.lpswd);
		if ( (*(connection_params.dstat) != 0xFF) ){
			SetVariable("DSTAT", connection_params.dstat);
		}
	}
	return 1;
}

void wait_milliseconds(unsigned int miliseg);

void TimingDelayMicro(unsigned int tick)
{
	wait_milliseconds(tick);
}

uint32_t getMsecTimestamp(void)
{
	return  HAL_GetTick();
}

uint32_t howManyMsecPassed(uint32_t since)
{
	uint32_t now = HAL_GetTick();

	if (now < since) return (UINT32_MAX - since + now);

	return (now - since);

}

static const double mDR = M_PI / 180;
static const double mK1 = 15 * (M_PI / 180) * 1.0027379;

static double mRightAscentionArr[3] = { 0.0, 0.0, 0.0 };
static double mDecensionArr[3] = { 0.0, 0.0, 0.0 };
static double mVHzArr[3] = { 0.0, 0.0, 0.0 };

static int mRiseTimeArr[2] = { 0, 0 };
static int mSetTimeArr[2] = { 0, 0 };
static double mRizeAzimuth = 0.0;
static double mSetAzimuth = 0.0;

static bool mIsSunrise = false;
static bool mIsSunset = false;

static double GetJulianDay(int year, int month, int day)
{
    bool gregorian = (year < 1583) ? false : true;

    if ((month == 1) || (month == 2))
    {
        year = year - 1;
        month = month + 12;
    }

    double a = floor((double)year / 100);
    double b = 0;

    if (gregorian)
        b = 2 - a + floor(a / 4);
    else
        b = 0.0;

    double jd = floor(365.25 * (year + 4716))
               + floor(30.6001 * (month + 1))
               + day + b - 1524.5;

    return jd;
}

static double LocalSiderealTimeForTimeZone(double lon, double jd, double z)
{
    double s = 24110.5 + 8640184.812999999 * jd / 36525 + 86636.6 * z + 86400 * lon;
    s = s / 86400;
    s = s - floor(s);
    return s * 360 * mDR;
}

static void CalculateSunPosition(double jd, double ct,
                                 double* outSunPositionInSky0,
                                 double* outSunPositionInSky1)
{
    double g, lo, s, u, v, w;

    lo = 0.779072 + 0.00273790931 * jd;
    lo = lo - floor(lo);
    lo = lo * 2 * M_PI;

    g = 0.993126 + 0.0027377785 * jd;
    g = g - floor(g);
    g = g * 2 * M_PI;

    v = 0.39785 * sin(lo);
    v = v - 0.01 * sin(lo - g);
    v = v + 0.00333 * sin(lo + g);
    v = v - 0.00021 * ct * sin(lo);

    u = 1 - 0.03349 * cos(g);
    u = u - 0.00014 * cos(2 * lo);
    u = u + 0.00008 * cos(lo);

    w = -0.0001 - 0.04129 * sin(2 * lo);
    w = w + 0.03211 * sin(g);
    w = w + 0.00104 * sin(2 * lo - g);
    w = w - 0.00035 * sin(2 * lo + g);
    w = w - 0.00008 * ct * sin(g);

    // compute sun's right ascension
    s = w / sqrt(u - v * v);
    *outSunPositionInSky0 = lo + atan(s / sqrt(1 - s * s));

    // ...and declination
    s = v / sqrt(u);
    *outSunPositionInSky1 = atan(s / sqrt(1 - s * s));
}

static int Sign(double value)
{
    int rv = 0;

    if (value > 0.0) rv = 1;
    else if (value < 0.0) rv = -1;
    else rv = 0;

    return rv;
}

static double TestHour(int k, double zone, double t0, double lat)
{
    double ha[3];
    double a, b, c, d, e, s, z;
    double time;
    int hr, min;
    double az, dz, hz, nz;

    ha[0] = t0 - mRightAscentionArr[0] + k * mK1;
    ha[2] = t0 - mRightAscentionArr[2] + k * mK1 + mK1;

    ha[1] = (ha[2] + ha[0]) / 2;    // hour angle at half hour
    mDecensionArr[1] = (mDecensionArr[2] + mDecensionArr[0]) / 2;  // declination at half hour

    s = sin(lat * mDR);
    c = cos(lat * mDR);
    z = cos(90.833 * mDR);    // refraction + sun semidiameter at horizon

    if (k <= 0)
        mVHzArr[0] = s * sin(mDecensionArr[0]) + c * cos(mDecensionArr[0]) * cos(ha[0]) - z;

    mVHzArr[2] = s * sin(mDecensionArr[2]) + c * cos(mDecensionArr[2]) * cos(ha[2]) - z;

    if (Sign(mVHzArr[0]) == Sign(mVHzArr[2]))
        return mVHzArr[2];  // no event this hour

    mVHzArr[1] = s * sin(mDecensionArr[1]) + c * cos(mDecensionArr[1]) * cos(ha[1]) - z;

    a = 2 * mVHzArr[0] - 4 * mVHzArr[1] + 2 * mVHzArr[2];
    b = -3 * mVHzArr[0] + 4 * mVHzArr[1] - mVHzArr[2];
    d = b * b - 4 * a * mVHzArr[0];

    if (d < 0)
        return mVHzArr[2];  // no event this hour

    d = sqrt(d);
    e = (-b + d) / (2 * a);

    if ((e > 1) || (e < 0))
        e = (-b - d) / (2 * a);

    time = (double)k + e + (double)1 / (double)120; // time of an event

    hr = (int)floor(time);
    min = (int)floor((time - hr) * 60);

    hz = ha[0] + e * (ha[2] - ha[0]);                 // azimuth of the sun at the event
    nz = -cos(mDecensionArr[1]) * sin(hz);
    dz = c * sin(mDecensionArr[1]) - s * cos(mDecensionArr[1]) * cos(hz);
    az = atan2(nz, dz) / mDR;
    if (az < 0) az = az + 360;

    if ((mVHzArr[0] < 0) && (mVHzArr[2] > 0))
    {
        mRiseTimeArr[0] = hr;
        mRiseTimeArr[1] = min;
        mRizeAzimuth = az;
        mIsSunrise = true;
    }

    if ((mVHzArr[0] > 0) && (mVHzArr[2] < 0))
    {
        mSetTimeArr[0] = hr;
        mSetTimeArr[1] = min;
        mSetAzimuth = az;
        mIsSunset = true;
    }

    return mVHzArr[2];
}

bool CalculateSunRiseSetTimes(double lat, double lon,
                              int year, int month, int day,
                              int* outRiseHour, int* outRiseMinute,
                              int* outSetHour, int* outSetMinute)
{
    mRiseTimeArr[0] = 0;
    mRiseTimeArr[1] = 0;
    mSetTimeArr[0] = 0;
    mSetTimeArr[1] = 0;
    mRizeAzimuth = 0.0;
    mSetAzimuth = 0.0;

    double zone = 0;
    double jd = GetJulianDay(year, month, day) - 2451545;  // Julian day relative to Jan 1.5, 2000

    if ((Sign(zone) == Sign(lon)) && (zone != 0)) return false;

    lon = lon / 360;
    double tz = zone / 24;
    double ct = jd / 36525 + 1;                                 // centuries since 1900.0
    double t0 = LocalSiderealTimeForTimeZone(lon, jd, tz);      // local sidereal time

    // get sun position at start of day
    jd += tz;
    double ra0, dec0;
    CalculateSunPosition(jd, ct, &ra0, &dec0);

    // get sun position at end of day
    jd += 1;
    double ra1, dec1;
    CalculateSunPosition(jd, ct, &ra1, &dec1);

    // make continuous
    if (ra1 < ra0)
        ra1 += 2 * M_PI;

    // initialize
    mIsSunrise = false;
    mIsSunset = false;

    mRightAscentionArr[0] = ra0;
    mDecensionArr[0] = dec0;

    // check each hour of this day
    for (int k = 0; k < 24; k++)
    {
        mRightAscentionArr[2] = ra0 + (k + 1) * (ra1 - ra0) / 24;
        mDecensionArr[2] = dec0 + (k + 1) * (dec1 - dec0) / 24;
        mVHzArr[2] = TestHour(k, zone, t0, lat);

        // advance to next hour
        mRightAscentionArr[0] = mRightAscentionArr[2];
        mDecensionArr[0] = mDecensionArr[2];
        mVHzArr[0] = mVHzArr[2];
    }

    *outRiseHour = mRiseTimeArr[0];
    *outRiseMinute = mRiseTimeArr[1];
    *outSetHour = mSetTimeArr[0];
    *outSetMinute = mSetTimeArr[1];

    /*
    isSunset = true;
    isSunrise = true;

    // neither sunrise nor sunset
    if ((!mIsSunrise) && (!mIsSunset))
    {
        if (mVHzArr[2] < 0)
            isSunrise = false; // Sun down all day
        else
            isSunset = false; // Sun up all day
    }
    // sunrise or sunset
    else
    {
        if (!mIsSunrise)
            // No sunrise this date
            isSunrise = false;
        else if (!mIsSunset)
            // No sunset this date
            isSunset = false;
    }
    */

    return true;
}

/*****************************************************************************
 * Function name    : ConvertArraytoInteger
 * 	  @brief		: Auxiliary function to convert a boolean array to decimal value
 *
 *    @param1       : array1: bytes array 1/0
 *    @param2       : tam: size array
 *
 * 	  @retval       : Returns a decimal value
 *
 * 	  Notes         : This function is only used for DEBUG purpose
 *****************************************************************************/
uint8_t ConvertArraytoInteger(char *array, uint8_t tam)
{

	uint8_t ret = 0;
	uint8_t tmp;

	for (uint8_t i = 0; i < tam; i++) {
		tmp = array[i];
		ret |= tmp << (tam - i - 1);
	}
	return ret;
}
