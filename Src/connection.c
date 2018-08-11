/*
 * connection.c
 *
 *  Created on: Sep 13, 2017
 *      Author: Sviatoslav
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "NBinterface.h"
#include "threadSafeWrappers.h"
#include "Definitions.h"
#include "main.h"
#include "MQTTAPI.H"

extern int tprintf(int hcon, char *texto,...);

static int	connections = 0;  // this counter gives us a hint for distinguish reboot/connection

/*************************************************************************
 * Connect (ip, port, user, password)
 * Function to encapsulate the complete connection
 * and factor it to be called with default and contingency parameters
 *************************************************************************/
int	Connect(char *host, unsigned int port, char *user, char *password)
{
	/// mqttMutex need to be locked when call these function
	/// contextMutex must to be unlocked when call these function

	int		hconn;
	bool 	ok;

 	hconn = MqttConnect(host, port, user, password);
	if (hconn > 0) {
		int n = 3 ;
		while (--n) {
			char topic[256];
			ok = true;

			// Subscribe to AP actuation topic
			sprintf(topic, "%s/CMC/ACT/%s", GetVariableThreadSafe("MTPRT"),
					GetVariableThreadSafe("ID"));
			if (MqttSubscribe(hconn, topic) < 0) ok = false;

			// Subscribe to Global AP actuation topic
			sprintf(topic, "%s/CMC/ACT/%s", GetVariableThreadSafe("MTPRT"),
					GetVariableThreadSafe("DGRID"));
			if (MqttSubscribe(hconn, topic) < 0) ok = false;

			if (ok)
			{
				return hconn;
			}
		}
		if (!ok)
			return -1;
	}
	return hconn;
}

//	First, calls the MqttConnection function, and it get connected, Subscribes to the (single) topic
//	Returns 1 on completion and -1 if the initialization fails

/////////////////////////////////////////////////////////////////////////////////////////////
//	Initialization of communications
//	The function doesn't take parameters (get all of then from the Context)
//	Makes all calling the Connection() function
//  First: try to connest using the 'default' broker parameters
//  	if succeds, stores those default values as 'last successful connection'
//		if fails, makes a second try with the 'last successful connection' (contingency)
/////////////////////////////////////////////////////////////////////////////////////////////
int	COMM_Init()
{
	/// mqttMutex need to be locked when call these function
	/// contextMutex must to be unlocked when call these function

	int		hconn;
	char	*id = GetVariableThreadSafe("ID");

	// Default broker parameters
	char *h1 = GetVariableThreadSafe("MURI");
	unsigned int p1 = atoi(GetVariableThreadSafe("MPORT"));
	char *u1 = GetVariableThreadSafe("MUSER");
	char *k1 = GetVariableThreadSafe("MPSWD");

 	hconn = Connect(h1, p1, u1, k1);
	if (hconn > 0) {
		tprintf (hconn,
			"%s Connected(%d) to %s, successfully subscripted to All topics",
				id, connections++, h1);
		// Update the LAST SUCCESSFUL CONNECTION CONFIG VALUES
		// FIXME temporally disabled because Wlan issue
//		SetVariableThreadSafe("BMURI", GetVariableThreadSafe("MURI"));
//		SetVariableThreadSafe("BMPRT", GetVariableThreadSafe("MPORT"));
//		SetVariableThreadSafe("BMUSR", GetVariableThreadSafe("MUSER"));
//		SetVariableThreadSafe("BMPSW", GetVariableThreadSafe("MPSWD"));
		return hconn;
	}
	else {
		// CONTINGENCY: Try with the LAST SUCCESSFUL CONNECTION CONFIG VALUES
		char *h2 = GetVariableThreadSafe("BMURI");
		unsigned int p2 = atoi(GetVariableThreadSafe("BMPRT"));
		char *u2 = GetVariableThreadSafe("BMUSR");
		char *k2 = GetVariableThreadSafe("BMPSW");
	 	hconn = Connect(h2, p2, u2, k2);
		if (hconn > 0) {
			tprintf (hconn,
"%s Connected(%d) (CONTINGENCY) to %s, successfully subscripted to All topics",
					id, connections++, h1);
		}
		return hconn;
	}
}
