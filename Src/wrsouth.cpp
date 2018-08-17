/*
 * wrsouth.cpp
 *
 *  Created on: 2 jun. 2017
 *      Author: juanra
 */


#include <stdio.h>
#include <stdlib.h>
#include "wrsouth.h"
#include "context.h"
#include "southbound_ec.h"
#include "NBinterface.h"
#include "Definitions.h"
#include "utils.h"


/************************************************
			PROVISIONAL
 A helper function to simulate a INTEGER variable
	ranging within  MIN and MAX
  ************************************/
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


/************************************************
	PLACEHOLDERS for the real READING wrappers
*************************************************/

char *Read_TFVOL(const char *value)
{
  // Enable Photodiode power
  HAL_GPIO_WritePin(EX_RESET_PHOTODIODE_GPIO_Port, EX_RESET_PHOTODIODE_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(nSHDN_GPIO_Port, nSHDN_Pin, GPIO_PIN_RESET);
  HAL_Delay(100);

  float pfm_to_analogue = adc_read_val(ADC_CHANNEL_6) * 1000;

  // Disable Photodiode power
  HAL_GPIO_WritePin(EX_RESET_PHOTODIODE_GPIO_Port, EX_RESET_PHOTODIODE_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(nSHDN_GPIO_Port, nSHDN_Pin, GPIO_PIN_SET);

  // Set timestamp
  char tsbuf[16];
  long ts = GetTimeStamp();
  snprintf(tsbuf, 16, "%li", ts);
  SetVariable((char*)"TFVTS", tsbuf);

  // Set voltage value
  static char tfvoltbuf[16];
  snprintf(tfvoltbuf, 16, "%d.%02d", (int)pfm_to_analogue, abs((int)(pfm_to_analogue*100.0f))%100);
  SetVariable((char*)"TFVOL", tfvoltbuf);
  return tfvoltbuf;
}


char	*Read_Temp(const char *) {
	return VariableInt(28, 47);
}



char	*Read_V(const char *) {
	return VariableDec(210, 240);
}



char	*Read_I(const char *) {
	return VariableInt(300, 500);
}


char	*Read_P(const char *) {
	return VariableDec(120, 180);
}


char	*Read_Q(const char *) {
	return VariableDec(0, 0);
}


char	*Read_S(const char *) {
	return VariableDec(110, 160);
}



char	*Read_EP(const char *) {
	return VariableDec(705, 720);
}


char	*Read_EQ(const char *) {
	return VariableDec(0, 0);
}


char	*Read_ES(const char *) {
	return VariableDec(688, 692);
}


char	*Read_DSTAT(const char *) {
	return "50";
}


/************************************************
	PLACEHOLDERS for the real WRITING wrappers
*************************************************/

////////////////////////////////////////////////////////////////
//	Provisional way of calling-by-name the variable eval method
//	Looks up the method, and calls it
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
