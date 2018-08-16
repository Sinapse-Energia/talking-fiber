/*
 * Apllication.h
 *
 *  Created on: 26 sept. 2017
 *      Author: juanra
 */

#ifndef APLLICATION_H_
#define APLLICATION_H_


#ifdef __cplusplus
extern "C" {
#endif

	int		TestSynchonization (void);	// Provisional placement

	int		SendMeassurement(void);		//	helper function to trigger a Meassurement message sending
	int		SendCoordinates (void);		//	helper function to trigger a LLocalization message sending
	int		SetProgramm		(void);		//	action to be called when a 2; command is processed
	int		SetDimming		(void);		//	action to be called when a 3; command is processed
	int		SetPeriodics	(void);		//	action to be called when a 4; command is processed
	int		SetPeriodicsGPS	(void);		//	action to be called when a 401; command is processed
	int		ApplyDimming	(void);		//	action to be used for set the timmer to the programmed value 
	int		GetDimming		(long t);	//	helper function to get the dimming value corresponding to any time

	int 	SetTFPeriodic	(void);

	int 	EvalSunTimes 	(void);		//  action to be call on Sun times request

	int		QueryPosition	();			// provisional... ir hacia SENDCOMMAND....
	int		ExecCmd			();			// provisional... ir hacia SENDCOMMAND....

#if defined(GPRS_TRANSPORT)
	int		ManageAPNS		();
#endif

#if defined(BUILD_CANBUS)
// CAN ACTIONS
	int		CAN_configure	();
	int		CAN_Send		();
	int		CAN_Read		();
#endif


// SENSORS
	int 	ASN_READ		();
	int 	DSN_READ		();


#ifdef __cplusplus
}
#endif



#endif /* APLLICATION_H_ */
