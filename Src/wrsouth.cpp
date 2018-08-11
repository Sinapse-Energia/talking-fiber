/*
 * wrsouth.cpp
 *
 *  Created on: 2 jun. 2017
 *      Author: juanra
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "wrsouth.h"
#include "context.h"
#include "southbound_ec.h"
#include "Definitions.h"
#include "utils.h"
#include "NBinterface.h"
#include "Frames.h"
#include "threadSafeWrappers.h"
#include "tim.h"
#include "gpio.h"
#include "usart.h"
#include "Shared.h"
#include "adc.h"

/************************************************
			PROVISIONAL
 A helper function to simulate a INTEGER variable
	ranging within  MIN and MAX
  ************************************/

// Parse HEX from text, return position to char after parsed or NULL if fail
char* parseHexByte(char* in_text, uint8_t* out_byte)
{
	// Skip spaces
	while (*in_text == ' ') in_text++;
	// Check for end of text
	if (*in_text == '\0') return NULL;
	// Normalize symbol
	char sym = (char)toupper(*in_text);
	// Check format
	if (sym < '0' || (sym > '9' && sym < 'A') || sym > 'F') return NULL;
	// Decode symbol
	uint8_t byte = ((sym < 'A') ? (sym - '0') : (sym - 'A' + 10)) & 0x0F;
	byte = byte << 4;

	// Go to next symbol
	in_text++;

	// Find and parse low 4 bits value

	// Skip spaces
	while (*in_text == ' ') in_text++;
	// Check for end of payload
	if (*in_text == '\0') return NULL;
	// Normalize symbol
	sym = (char)toupper(*in_text);
	// Check format
	if (sym < '0' || (sym > '9' && sym < 'A') || sym > 'F') return NULL;
	// Decode symbol
	byte |= (((sym < 'A') ? (sym - '0') : (sym - 'A' + 10)) & 0x0F);

	// Go to next symbol
	in_text++;

	*out_byte = byte;

	return in_text;
}

char	*VariableInt(int min, int max) {
	static char result[16];
	int x =  min + rand()%(max-min);
	itoa(x, result, 10);
	return result;
}

// Id for DECIMAL VARIABLES (one decimal figure output)
char	*VariableDec(double min, double max) {
	static char result[16];
	float r = (float)rand()/(float)RAND_MAX;
	double x = min + r * (max-min);	
	int intp = (int) x;
	int fracp = (int) ((r - x)* 10 );
	sprintf(result, "%d.%1d", intp, fracp);
	return result;
}

int randInt(int min, int max)
{
	return (min + rand() * (max - min) / RAND_MAX);
}

float randFloat(float min, float max)
{
	return (min + rand() * (max - min) / RAND_MAX);
}

/************************************************
	PLACEHOLDERS for the real READING wrappers
*************************************************/

char *Read_TFVOL(const char *value)
{
	//HAL_GPIO_WritePin(EX_RESET_PHOTODIODE_GPIO_Port, EX_RESET_PHOTODIODE_Pin, GPIO_PIN_SET);
	//HAL_Delay(100);
	// Read ADC diode voltage
	float pfm_to_analogue = adc_read_val(ADC_CHANNEL_6);
	// PFM_TO_ANALOGUE = (VDD_PHOTODIODE/2)-R_23_1*Iphotodiode
	float Iphotodiode = (VDD_PHOTODIODE/2 - pfm_to_analogue) / R_23_1;
	//HAL_GPIO_WritePin(EX_RESET_PHOTODIODE_GPIO_Port, EX_RESET_PHOTODIODE_Pin, GPIO_PIN_RESET);

	// Set timestamp
	char tsbuf[16];
	long ts = GetTimeStamp();
	snprintf(tsbuf, 16, "%li", ts);
	SetVariable((char*)"TFVTS", tsbuf);

	// Set voltage value
	static char tfvoltbuf[16];
	snprintf(tfvoltbuf, 16, "%d.%02d", (int)Iphotodiode, abs((int)(Iphotodiode*100.0f))%100);
	return tfvoltbuf;
}

char *Read_LTIME(const char *value)
{
	return GetDateTime();
}

char *Read_GPSSST(const char *value)
{
	static char timeSSbuf[6];
	int timeSSHour = atoi(GetVariable((char*)"GPSSSH"));
	int timeSSMinute = atoi(GetVariable((char*)"GPSSSM"));
	sprintf(timeSSbuf, "%02i:%02i", timeSSHour, timeSSMinute);
	return &timeSSbuf[0];
}

char *Read_GPSSRT(const char *value)
{
	static char timeSRbuf[6];
	int timeSRHour = atoi(GetVariable((char*)"GPSSRH"));
	int timeSRMinute = atoi(GetVariable((char*)"GPSSRM"));
	sprintf(timeSRbuf, "%02i:%02i", timeSRHour, timeSRMinute);
	return &timeSRbuf[0];
}

char* Write_HourMinute(const char *value, const char *minuteName)
{
	static char hour[3];
	static char minute[3];

	if (strlen(value) < 4) return NULL;

	// Check for SR/SS format
	if (value[0] == 'S')
	{
		// Get the base time, Sunrise or Sunset
		int baseTime = 0;
		if (value[1] == 'R')
		{
			// Sunrise as base
			baseTime = atoi(GetVariable((char*)"GPSSRH"))*60 +
					atoi(GetVariable((char*)"GPSSRM"));
		}
		else if (value[1] == 'S')
		{
			// Sunset as base
			baseTime = atoi(GetVariable((char*)"GPSSSH"))*60 +
					atoi(GetVariable((char*)"GPSSSM"));
		}
		else
		{
			// Bad format
			return NULL;
		}

		// Get the offset
		int offset = atoi(&value[3]);

		// Perform calculations
		if (value[2] == '+')
		{
			baseTime += offset;
		}
		else if (value[2] == '-')
		{
			baseTime -= offset;
		}
		else
		{
			// Bad format
			return NULL;
		}

		// Normalization
		if (baseTime < 0) baseTime = baseTime + (24*60);
		baseTime = baseTime % (24*60);

		sprintf(hour, "%02i", (baseTime/60));
		sprintf(minute, "%02i", (baseTime%60));
		SetVariable((char*)minuteName, minute);
		return hour;
	}

	// Usual time parsing (HH:MM)
	if (strlen(value) == 5)
	{
        hour[0]   = value[0];
        hour[1]   = value[1];
        hour[2]   = '\0';
        minute[0] = value[3];
        minute[1] = value[4];
        minute[2] = '\0';

        SetVariable((char*)minuteName, minute);
        return hour;
	}

	return (char*)value;
}

char *Write_NEWRT(const char *value)
{
	if (strcmp("-1", value) != 0)
		SetVariable((char*)"MTPRT", (char*)value);

	return (char*)value;
}

char *Write_NEWID(const char *value)
{
	if (strcmp("-1", value) != 0)
		SetVariable((char*)"ID", (char*)value);

	return (char*)value;
}

int Post_STGPS()
{
	// Get coordinates
	double lat = atof(GetVariable((char*)"GPSLAT"));
	double lon = atof(GetVariable((char*)"GPSLON"));

	// Get date
	int nowDay = 0, nowMonth = 0, nowYear = 0;
	getDate(&nowDay, &nowMonth, &nowYear);

	// Calculate SR/SS
	int srHour = 6, srMin = 0, ssHour = 21, ssMin = 0;
	CalculateSunRiseSetTimes(lat, lon,
			nowYear, nowMonth, nowDay,
			&srHour, &srMin, &ssHour, &ssMin);

	char buf[8];

	// Set SR/SS variables
	sprintf(buf, "%i", srHour);
	SetVariable((char*)"GPSSRH", buf);
	sprintf(buf, "%i", srMin);
	SetVariable((char*)"GPSSRM", buf);
	sprintf(buf, "%i", ssHour);
	SetVariable((char*)"GPSSSH", buf);
	sprintf(buf, "%i", ssMin);
	SetVariable((char*)"GPSSSM", buf);

	// Reboot to calculate UTC offset
	REBOOT();

	return 1;
}

int Post_BTCFG()
{
	SharedMemoryData sharedData;
	ReadSharedMemory_ThreadSafe(&sharedData);

	// TODO change string sizes from hardcoded to defined

	// Fill struct with params
	if (strcmp("-1", GetVariable((char*)"BTLSER")) != 0)
		strncpy(sharedData.variables.FW_SERVER_URI, GetVariable((char*)"BTLSER"), 63);
	if (strcmp("-1", GetVariable((char*)"BTLPRT")) != 0)
		sharedData.variables.PORT = atoi(GetVariable((char*)"BTLPRT"));
	if (strcmp("-1", GetVariable((char*)"BTLPRO")) != 0)
		strncpy(sharedData.variables.PROTOCOL, GetVariable((char*)"BTLPRO"), 7);
	if (strcmp("-1", GetVariable((char*)"BTLPAT")) != 0)
		strncpy(sharedData.variables.PATH, GetVariable((char*)"BTLPAT"), 63);
	if (strcmp("-1", GetVariable((char*)"BTLNAM")) != 0)
		strncpy(sharedData.variables.FW_NAME, GetVariable((char*)"BTLNAM"), 63);
	if (strcmp("-1", GetVariable((char*)"BTLUSR")) != 0)
		strncpy(sharedData.variables.USER, GetVariable((char*)"BTLUSR"), 63);
	if (strcmp("-1", GetVariable((char*)"BTLPAS")) != 0)
		strncpy(sharedData.variables.PASSWORD, GetVariable((char*)"BTLPAS"), 63);
	if (strcmp("-1", GetVariable((char*)"BTLCNT")) != 0)
		sharedData.variables.UPDFW_COUNT = atoi(GetVariable((char*)"BTLCNT"));

	// TODO Check FW_SERVER_URI
	// Need to have specified pattern to check

	// Check PROTOCOL (now support only HTTP)
	if (strcmp("HTTP", sharedData.variables.PROTOCOL) != 0)
	{
		SetVariable((char*)"BTLRES", (char*)"3");
		return 1;
	}

	// TODO Check PATH
	// Need to have specified pattern to check

	// Write parameters
	if (WriteSharedMemory_ThreadSafe(&sharedData))
		SetVariable((char*)"BTLRES", (char*)"0");
	else
		SetVariable((char*)"BTLRES", (char*)"5");

	return 1;
}

int Post_BTACT()
{
	// TODO check BTLAUS and BTLAPS

	// Set UPDFW to 1
	SharedMemoryData sharedData;
	ReadSharedMemory_ThreadSafe(&sharedData);
	sharedData.variables.UPDFW = 1;
	if (WriteSharedMemory_ThreadSafe(&sharedData))
		SetVariable((char*)"BTLRES", (char*)"0");
	else
		SetVariable((char*)"BTLRES", (char*)"5");

	return 1;
}

// HOOK of functions to be called with a context variable is modified
struct	st_wrapper	dispatch[] = {

	"LTIME",		Read_LTIME,		NULL,
	"GPSSST",		Read_GPSSST,	NULL,
	"GPSSRT",		Read_GPSSRT,	NULL,
	"NEWRT",		NULL,			Write_NEWRT,
	"NEWID",		NULL,			Write_NEWID,

	"TFVOL",		Read_TFVOL,		NULL,

	NULL
};

// HOOK of functions to be called when a frame is processed. The functions should be developed in this file or in an extra file containing only the frame actions
struct	st_fractions
	actions[] = {
	"CCOMM", 	NULL,  			NEWCONN,
	"STGPS",	NULL,			Post_STGPS,
	"BTCFG",	NULL,			Post_BTCFG,
	"BTACT",	NULL,			Post_BTACT,
	"CUSAP",	NULL,			REBOOT,

	NULL
};

////////////////////////////////////////////////////////////////
//	Provisional way of calling-by-name the variable eval method
//	Looksup the method, and calls it
////////////////////////////////////////////////////////////////

const char	*GenericREAD(CVariable *v) {
	int i = 0;
	const char	*n1 = v->Id();

	while (dispatch[i].nombre ) {
		if (!strcmp(n1, dispatch[i].nombre))
			if (dispatch[i].reader)
				return dispatch[i].reader(v->read(false));
			else
				return NULL;
		else
			i++;
	}
		return NULL;
}
/**/
////////////////////////////////////////////////////////////////
//	Provisional way of calling-by-name the variable update method
//	Looksup the method, and calls it
////////////////////////////////////////////////////////////////
const char	*GenericWRITE(CVariable *v, const char *value) {
	int i = 0;
	const char	*n1 = v->Id();
	const char *n2;
	while ((n2 = dispatch[i].nombre) != NULL ) {
		if (!strcmp(n1, n2)) 
			if (dispatch[i].writer)
				return dispatch[i].writer(value);
			else
				return NULL;
		else
			i++;
	}
		return NULL;
}
/**/
