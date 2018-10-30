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
#include "tim.h"
#include "gpio.h"
#include "usart.h"
#include "Shared.h"
#include "adc.h"
#include "Shared.h"

/************************************************
	PLACEHOLDERS for the real READING wrappers
*************************************************/

char *Read_TFVOL(const char *value)
{
    // Enable Photodiode power
    HAL_GPIO_WritePin(EX_RESET_PHOTODIODE_GPIO_Port, EX_RESET_PHOTODIODE_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(nSHDN_GPIO_Port, nSHDN_Pin, GPIO_PIN_SET);
    HAL_Delay(TF_PHOTO_POWER_DELAY_MSEC);

    // Read ADC diode voltage
    float samples[TF_PHOTO_SAMPLES];
    int minInd = 0, maxInd = 0;

    samples[0] = adc_read_val(ADC_CHANNEL_6);
	for (int i = 1; i < TF_PHOTO_SAMPLES; i++)
	{
	    samples[i] = adc_read_val(ADC_CHANNEL_6);
	    if (samples[i] > samples[maxInd])
	        maxInd = i;
	    if (samples[i] < samples[minInd])
	        minInd = i;
	}

	// Calculate average
	float sum = 0;
	int n = 0;
	for (int i = 0; i < TF_PHOTO_SAMPLES; i++)
	{
	    if (i == minInd || i == maxInd) continue;
	    sum += samples[i];
	    n++;
	}

    float pfm_to_analogue = (sum / n) * 1000; // in mV
	// PFM_TO_ANALOGUE = (VDD_PHOTODIODE/2)-R_23_1*Iphotodiode
	//float Iphotodiode = (VDD_PHOTODIODE/2 - pfm_to_analogue) / R_23_1;

    // Disable Photodiode power
    HAL_GPIO_WritePin(nSHDN_GPIO_Port, nSHDN_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(EX_RESET_PHOTODIODE_GPIO_Port, EX_RESET_PHOTODIODE_Pin, GPIO_PIN_RESET);

	// Set timestamp
	char tsbuf[16];
	long ts = GetTimeStamp();
	snprintf(tsbuf, 16, "%li", ts);
	SetVariable((char*)"TFVTS", tsbuf);

	// Set voltage value
	static char tfvoltbuf[16];
	snprintf(tfvoltbuf, 16, "%d.%02d", (int)pfm_to_analogue, abs((int)(pfm_to_analogue*100.0f))%100);
	return tfvoltbuf;
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

char *Write_TLSTH(const char *value)
{
    return Write_HourMinute(value, "TLSTM");
}

int Post_BTCFG()
{
	SharedMemoryData sharedData;
	ReadSharedMemory(&sharedData);

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
	if (WriteSharedMemory(&sharedData))
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
	ReadSharedMemory(&sharedData);
	sharedData.variables.UPDFW = 1;
	if (WriteSharedMemory(&sharedData))
		SetVariable((char*)"BTLRES", (char*)"0");
	else
		SetVariable((char*)"BTLRES", (char*)"5");

	return 1;
}

int Post_CONLS()
{
    int LISEN = atoi(GetVariable((char*)"LISEN"));
    int TLSTH = atoi(GetVariable((char*)"TLSTH"));
    int TLSTM = atoi(GetVariable((char*)"TLSTM"));
    int TLDUR = atoi(GetVariable((char*)"TLDUR"));
    int TLRTN = atoi(GetVariable((char*)"TLRTN"));

    if (LISEN < 0 || LISEN > 1 || TLSTH < 0 || TLSTH > 23 ||
            TLSTM < 0 || TLSTM > 59 || TLDUR < 1 || TLDUR > 60 ||
            TLRTN < 1 || TLRTN > 10)
    {
        // Error TODO need mechanism in context to restore variables
        SetVariable((char*)"TLRES", (char*)"0");
    }
    else
    {
        SetVariable((char*)"TLRES", (char*)"1");
    }

    return 1;
}

int Post_CONSN()
{
    int TSPER = atoi(GetVariable((char*)"TSPER"));
    int TSLDR = atoi(GetVariable((char*)"TSLDR"));
    int TSRTN = atoi(GetVariable((char*)"TSRTN"));

    if (TSPER < 0 || TSLDR < 0 || TSLDR > 60 ||
            TSRTN < 1 || TSRTN > 10)
    {
        // Error TODO need mechanism in context to restore variables
        SetVariable((char*)"TSRES", (char*)"0");
    }
    else
    {
        SetVariable((char*)"TSRES", (char*)"1");
    }

    return 1;
}

int Post_CONAL()
{
    int TAPER = atoi(GetVariable((char*)"TAPER"));
    int TATHL = atoi(GetVariable((char*)"TATHL"));
    int TATHH = atoi(GetVariable((char*)"TATHH"));

    if (TAPER < 0 || TATHL < 0 || TATHH < 0 || TATHL > TATHH)
    {
        // Error TODO need mechanism in context to restore variables
        SetVariable((char*)"TARES", (char*)"0");
    }
    else
    {
        SetVariable((char*)"TARES", (char*)"1");
    }

    return 1;
}

// HOOK of functions to be called with a context variable is modified
struct	st_wrapper	dispatch[] = {

	"TFVOL",		Read_TFVOL,		NULL,
	"TLSTH",        NULL,           Write_TLSTH,

	NULL
};

// HOOK of functions to be called when a frame is processed. The functions should be developed in this file or in an extra file containing only the frame actions
struct	st_fractions
	actions[] = {
	"CCOMM", 	NULL,  			NEWCONN,
	"BTCFG",	NULL,			Post_BTCFG,
	"BTACT",	NULL,			Post_BTACT,
	"CHAPN",    NULL,           NEWCONN,
	"CONLS",    NULL,           Post_CONLS,
	"CONSN",    NULL,           Post_CONSN,
	"CONAL",    NULL,           Post_CONAL,

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
