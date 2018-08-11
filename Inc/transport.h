/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *    Sergio R. Caprile - "commonalization" from prior samples and/or documentation extension
 *******************************************************************************/

#include "Definitions.h"

#if defined(SELECT_COMM_WLAN) || defined(HYBRID_M95_WLAN_CODE)

int transport_getdataMqttPub(unsigned char* buf, int count);
int transport_getdataMqttCtrl(unsigned char* buf, int count);

#else

int transport_getdata(unsigned char* buf, int count);

#define transport_getdataMqttPub transport_getdata
#define transport_getdataMqttCtrl transport_getdata

#endif

int transport_sendPacketBuffer(int sock, unsigned char* buf, int buflen);
int transport_getdataMqttPub(unsigned char* buf, int count);
int transport_getdataMqttCtrl(unsigned char* buf, int count);
int transport_getdatanb(void *sck, unsigned char* buf, int count);
int transport_open(const char* host, int port);
int transport_close(int sock);
