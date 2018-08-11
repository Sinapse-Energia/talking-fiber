/*
 * wlan.h
 *
 *  Created on: Sep 3, 2017
 *      Author: Sviatoslav
 */

#ifndef WLAN_H_
#define WLAN_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "usart.h"
#include "Definitions.h"

#define WLAN_BUF_SIZE 	4096
#ifdef SELECT_COMM_WLAN
#define WLAN_UART 		(&HUART_WIFI)
#endif
#define WLAN_TX_TIMEO	1000
#define WLAN_RX_TIMEO	10000

void wlanRecvExec(void);

// 1 - ok, 0 - error
int transport_sendPacketBuffer(int sock, unsigned char* buf, int buflen);

// return read size
int transport_getdataMqttPub(unsigned char* buf, int count);
int transport_getdataMqttCtrl(unsigned char* buf, int count);

int transport_open(const char* host, int port);
int transport_close(int sock);

#ifdef HYBRID_M95_WLAN_CODE
#define WLAN_UART 		(&HUART_GPRS)
void wlanRecvStartMQTT(void);
void wlanRecvStopMQTT(void);
#endif

bool wlan_parse_tz_json(char* json, int* out_offset);

size_t wlanGetAvailableDataMqttPub(void);
size_t wlanGetAvailableDataMqttCtrl(void);

#ifdef __cplusplus
}
#endif

#endif /* WLAN_H_ */
