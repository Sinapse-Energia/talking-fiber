/*
 * utils.h
 *
 *  Created on: Jun 19, 2017
 *      Author: External
 */

#ifndef UTILS_H_
#define UTILS_H_

#include	<time.h>
#include	<stdint.h>
#include 	<stdbool.h>

#ifdef __cplusplus
 extern "C" {
#endif

extern	long 	GetTimeStamp(void);
extern	char	*strDateTime();
extern	void 	AddSeconds(int num);

char	*getHour(void);
char	*getMinute(void);
void	getDate(int* outDay, int* outMonth, int* outYear);

// Handlers for AT-COMMANDS whose output deserves to be managed
extern	int		SetDateTime(const char *NTP);
extern	int		SetLocalIP(const char *iptx);
extern	int		SetIdSIM(const char *simtx);
extern	int		SetIMEI(const char *txt);

extern	int		ValidateReg(const char *regreply);

extern	int		pretrace(char *texto,...);

char	*GetLocalMessage(int h, char *buffer, int maxsize);
char	*GetDateTime(void);

uint32_t getMsecTimestamp(void);
uint32_t howManyMsecPassed(uint32_t since);

bool CalculateSunRiseSetTimes(double lat, double lon,
                              int year, int month, int day,
                              int* outRiseHour, int* outRiseMinute,
                              int* outSetHour, int* outSetMinute);

extern	int		SetGeneric(const char *reply);

uint8_t ConvertArraytoInteger(char *array, uint8_t tam);
float TemperatureMeasure (char *array1, char *array2);
float RHMeasure (char *array1, char *array2, float TA);

#ifdef __cplusplus
}
#endif

#endif /* UTILS_H_ */
