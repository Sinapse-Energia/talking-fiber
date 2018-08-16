/*
 * utils.h
 *
 *  Created on: Jun 19, 2017
 *      Author: External
 */

#ifndef UTILS_H_
#define UTILS_H_


  // ALIAS for colors
 #define BLACK		0
 #define BLUE		1
 #define RED		2
 #define GREEN		3
 #define CYAN		4
 #define MAGENTA	5
 #define ORANGE		6
 #define WHITE		7

#define	COL_OFFLINE			RED
#define	COL_CONNECTING		ORANGE
#define	COL_CONNECTED		GREEN
#define	COL_CONNECTEDGPS	BLUE
#define	COL_EXECUTE			CYAN



#include	<time.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int hmqtt;


extern void	Color	(int col);
extern int	getcolor();
extern void	Blink	(void);

// printf over mqtt topic
extern int	tprintf	(int hcon, char *texto, ...);



// Inet functions not available from STM
uint32_t ntohl(uint32_t const net);

// Functions to handle timestamp
//	get the scalar timestamp now
extern	time_t 			GetTimeStamp(void);
//	get the projected timestamp at h:m:s of today
extern	time_t 			GetTimeStampAt(unsigned int h, unsigned int m, unsigned int s);
//  get the Time Structure
extern	const struct tm *GetTimeStructure(void);
//  get tm year
extern	unsigned int  	GetYear(void);
//  get tm month
extern	unsigned int  	GetMonth(void);
//  get tm day
extern	unsigned int  	GetDay(void);

// get the stringized timestamp
extern	char			*strDateTime();
// Update timestamp adding N seconds
extern	void 			AddSeconds(int num);
extern	void 			UpdateTimeStamp(int num);

//	Update the H:M:S timestamp ctx variable
extern	char			*UpdateNow(const char *x);
//  Update the Y:M:D:H:M:S ctx var
extern	char			*UpdateTS(const char *x);
//  Convert Unix TimeStamp to char
extern char				*UpdateUTS(const char *x);

// Functions to copy RTC back and forth to internal variables
extern int				UpdateRTC(void  *hrtc);
extern int 				GetRTC(void *ph);



//	Update SunRise SunSet (to be call on read..)
extern	char			*UpdateSR(const char *x);
extern	char			*UpdateSS(const char *x);


extern	int 			isdns(unsigned char * host);

// to initialize the scalar tiemstamp from an absolute value
extern	int 			SetTimeStamp(uint32_t ts);

// Handlers for AT-COMMANDS whose return values deserves to be managed as scalar
// Handlers for AT-COMMANDS whose output deserves to be managed as 3-values cond (returns +1,0,-1)
extern	int				SetDateTime(const char *txt);
extern	int				SetDateTimeNTP(const char *txt);

extern	int				SetGPRSDevice(const char *txt);

extern	int				SetLocalIP(const char *iptx);
extern	int	 			SetLocalIP2(const char *iptx);

extern	int				SetIdSIM(const char *simtx);
extern	int				SetIdSIM2(const char *simtx);

extern	int				SetIMEI(const char *txt);

extern	int				ValidateReg(const char *regreply);

extern	int				GetPositioning (const char *cpgga);

extern	int				SetGeneric(const char *reply);
extern	int				SetState(const char *reply);

// HLK Specifics
extern	int				SetMAC(const char *reply);
//extern	int		ValidateStat(const char *regreply);


extern	int				pretrace(char *texto,...);

extern void				WriteData(char *p); // this function is in Context.cpp but has to be moved to utils

extern	long			getHSsize (void);

extern  double 			ieee2real (unsigned char dataset[]);

extern	unsigned char	*getCertificate(size_t *lcert);
extern	unsigned char	*getCertificateTxt(size_t *lcert);

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
}
#endif


#endif /* UTILS_H_ */
