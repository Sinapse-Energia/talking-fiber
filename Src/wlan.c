#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "utils.h"

#include "Definitions.h"

#include "iwdg.h"
#include "usart.h"

#include "wlan.h"

#include "gpio.h"


#if defined(SELECT_COMM_WLAN) || defined(HYBRID_M95_WLAN_CODE)

// MQTT publish packet type ID
#define MQTT_PUBLISH_PACKET_ID	3

typedef enum {
	MqttReceiverStatePending = 0,
	MqttReceiverStateCtrl,
	MqttReceiverStatePub
} MqttReceiverState_t;

// Buffer for Uart receive DMA HAL function
static uint8_t wlanRecvDMABuf[WLAN_BUF_SIZE];
static volatile size_t wlanRecvReadPos = 0;
static volatile bool wlanDMAenabled = false;

/*
 * ================================ MQTT CTRL ================================
 */

// Buffer for received data
static uint8_t wlanBufMqttCtrl[WLAN_BUF_SIZE];
static volatile size_t wlanWriteIndexMqttCtrl = 0;
static volatile size_t wlanReadIndexMqttCtrl = 0;

size_t wlanGetAvailableDataMqttCtrl(void)
{
	wlanRecvExec();

	if (wlanReadIndexMqttCtrl >= wlanWriteIndexMqttCtrl)
	{ // It will never happen, but we must check and fix any way
		wlanReadIndexMqttCtrl = wlanWriteIndexMqttCtrl;
		return 0;
	}
	else
	{
		return (wlanWriteIndexMqttCtrl - wlanReadIndexMqttCtrl);
	}
}

// 1 - ok, 0 - error
static int wlanPutByteToBufMqttCtrl(uint8_t byte)
{
	// Reset buffer indexes when read reaches write
	if (wlanReadIndexMqttCtrl == wlanWriteIndexMqttCtrl)
	{
		wlanWriteIndexMqttCtrl = 0;
		wlanReadIndexMqttCtrl = 0;
	}

	// Check for overrun
	if (wlanWriteIndexMqttCtrl >= WLAN_BUF_SIZE)
	{
		// Buffer overflow
		return 0;
	}

	// Put data
	wlanBufMqttCtrl[wlanWriteIndexMqttCtrl] = byte;
	wlanWriteIndexMqttCtrl++;

	return 1;
}

// return read size
int transport_getdataMqttCtrl(unsigned char* buf, int count)
{
	size_t wlanRxWaitTime = 0;
	while ((wlanGetAvailableDataMqttCtrl() < count) &&
			(wlanRxWaitTime < WLAN_RX_TIMEO))
	{
		HAL_Delay(50);
		wlanRxWaitTime += 50;
	}

	if (wlanGetAvailableDataMqttCtrl() < count) return 0;

	memcpy(&buf[0], &wlanBufMqttCtrl[wlanReadIndexMqttCtrl], count);
	wlanReadIndexMqttCtrl += count;

	return count;
}

/*
 * ================================ MQTT PUB ================================
 */

// Buffer for received data
static uint8_t wlanBufMqttPub[WLAN_BUF_SIZE];
static volatile size_t wlanWriteIndexMqttPub = 0;
static volatile size_t wlanReadIndexMqttPub = 0;

size_t wlanGetAvailableDataMqttPub(void)
{
	wlanRecvExec();

	if (wlanReadIndexMqttPub >= wlanWriteIndexMqttPub)
	{ // It will never happen, but we must check and fix any way
		wlanReadIndexMqttPub = wlanWriteIndexMqttPub;
		return 0;
	}
	else
	{
		return (wlanWriteIndexMqttPub - wlanReadIndexMqttPub);
	}
}

// 1 - ok, 0 - error
static int wlanPutByteToBufMqttPub(uint8_t byte)
{
	// Reset buffer indexes when read reaches write
	if (wlanReadIndexMqttPub == wlanWriteIndexMqttPub)
	{
		wlanWriteIndexMqttPub = 0;
		wlanReadIndexMqttPub = 0;
	}

	// Check for overrun
	if (wlanWriteIndexMqttPub >= WLAN_BUF_SIZE)
	{
		// Buffer overflow
		return 0;
	}

	// Put data
	wlanBufMqttPub[wlanWriteIndexMqttPub] = byte;
	wlanWriteIndexMqttPub++;

	return 1;
}

// return read size
int transport_getdataMqttPub(unsigned char* buf, int count)
{
	size_t wlanRxWaitTime = 0;
	while ((wlanGetAvailableDataMqttPub() < count) &&
			(wlanRxWaitTime < WLAN_RX_TIMEO))
	{
		HAL_Delay(50);
		wlanRxWaitTime += 50;
	}

	if (wlanGetAvailableDataMqttPub() < count) return 0;

	memcpy(&buf[0], &wlanBufMqttPub[wlanReadIndexMqttPub], count);
	wlanReadIndexMqttPub += count;

	return count;
}

/*
 * ================================ MAIN ======================================
 */

static volatile MqttReceiverState_t mqttRecvState = MqttReceiverStatePending;
static volatile bool mqttRecvLengthNow = false;
static volatile size_t mqttRecvDataNeed = 0;

// Indicates that UART is in text lines mode
static volatile bool lineMode = false;

#define UART_LINES_MAX_CNT	3
#define UART_LINE_MAX_LEN	64
static char lineBuf[UART_LINES_MAX_CNT][UART_LINE_MAX_LEN];
static volatile size_t linePos = 0;
static volatile size_t linesCnt = 0;
static uint8_t binBuf[UART_LINE_MAX_LEN];
static volatile size_t binPos = 0;
static volatile size_t binNeed = 0;

static void wlanRecvByte(uint8_t byte)
{
	if (binNeed > 0)
	{
		if (binPos < UART_LINE_MAX_LEN)
			binBuf[binPos++] = byte;
		binNeed--;
	}
	else if (lineMode) // Line mode
	{
		if (linesCnt >= UART_LINES_MAX_CNT) return; // reached maximum lines

		if (byte == '\n' || byte == '\r' || byte == '\0') // check end of line
		{
			if (linePos == 0) return; // skip empty lines

			lineBuf[linesCnt][linePos] = '\0'; // got an eol
			linePos = 0;
			linesCnt++;
		}
		else
		{
			if (linePos < UART_LINE_MAX_LEN - 1) // got byte, check line length
			{
				lineBuf[linesCnt][linePos++] = byte; // put byte into buffer
			}
			//else {} // line overflow, skipping bytes
		}
	}
	else // MQTT mode
	{
		// Receive first byte - packet type
		if (mqttRecvState == MqttReceiverStatePending)
		{
			// Analyze packet type
			if ((byte>>4) == MQTT_PUBLISH_PACKET_ID)
			{
				mqttRecvState = MqttReceiverStatePub;
				wlanPutByteToBufMqttPub(byte);
			}
			else
			{
				mqttRecvState = MqttReceiverStateCtrl;
				wlanPutByteToBufMqttCtrl(byte);
			}

			// Next will receive length
			mqttRecvLengthNow = true;
			mqttRecvDataNeed = 0;
		}
		// Receive next bytes
		else
		{
			// Put byte to corresponding buffer
			if (mqttRecvState == MqttReceiverStatePub)
			{
				wlanPutByteToBufMqttPub(byte);
			}
			else
			{
				wlanPutByteToBufMqttCtrl(byte);
			}

			// Receive length
			if (mqttRecvLengthNow)
			{
				mqttRecvDataNeed = (mqttRecvDataNeed << 8) | byte;

				// End receiving length when needed
				if (byte < 0x7F)
				{
					mqttRecvLengthNow = false;
				}
			}
			// Receive other bytes
			else
			{
				mqttRecvDataNeed--;
			}

			// If no data next - go to receive next packet
			if (mqttRecvDataNeed == 0)
			{
				mqttRecvState = MqttReceiverStatePending;
			}
		}
	}
}

void wlanRecvExec(void)
{ // TODO block whole function with own mutex if getting problems with receive

	if (!wlanDMAenabled) return;

	size_t bytesToRead = 0;
	size_t bytesToWrite = WLAN_UART->hdmarx->Instance->NDTR;

	size_t wlanRecvWritePos = WLAN_BUF_SIZE - bytesToWrite;
	if (wlanRecvWritePos >= wlanRecvReadPos)
		bytesToRead = wlanRecvWritePos - wlanRecvReadPos;
	else
		bytesToRead = WLAN_BUF_SIZE - wlanRecvReadPos + wlanRecvWritePos;

	while (bytesToRead > 0)
	{
		wlanRecvByte(wlanRecvDMABuf[wlanRecvReadPos]);

		if (wlanRecvReadPos == (WLAN_BUF_SIZE-1))
			wlanRecvReadPos = 0;
		else
			wlanRecvReadPos++;

		bytesToRead--;
	}
}

size_t wlanRecvToBufExec(uint8_t* buf, size_t buf_size)
{
	if (!wlanDMAenabled) return 0;

	size_t bytesToRead = 0;
	size_t bytesToWrite = WLAN_UART->hdmarx->Instance->NDTR;
	size_t readed = 0;

	size_t wlanRecvWritePos = WLAN_BUF_SIZE - bytesToWrite;
	if (wlanRecvWritePos >= wlanRecvReadPos)
		bytesToRead = wlanRecvWritePos - wlanRecvReadPos;
	else
		bytesToRead = WLAN_BUF_SIZE - wlanRecvReadPos + wlanRecvWritePos;

	while (bytesToRead > 0 && buf_size > 0)
	{
		*buf++ = wlanRecvDMABuf[wlanRecvReadPos];

		if (wlanRecvReadPos == (WLAN_BUF_SIZE-1))
			wlanRecvReadPos = 0;
		else
			wlanRecvReadPos++;

		bytesToRead--;
		buf_size--;
		readed++;
	}

	return readed;
}

void wlanRecvStartMQTT(void)
{
	wlanDMAenabled = false;
	HAL_UART_DMAStop(WLAN_UART);

	binNeed = 0;
	lineMode = false;

	wlanReadIndexMqttCtrl = 0;
	wlanReadIndexMqttPub = 0;
	wlanWriteIndexMqttCtrl = 0;
	wlanWriteIndexMqttPub = 0;
	mqttRecvState = MqttReceiverStatePending;
	wlanRecvReadPos = 0;

	HAL_UART_Receive_DMA(WLAN_UART, &wlanRecvDMABuf[0], WLAN_BUF_SIZE);
	wlanDMAenabled = true;
}

void wlanRecvStopMQTT(void)
{
	wlanDMAenabled = false;
	HAL_UART_DMAStop(WLAN_UART);
}

#endif

bool wlan_parse_tz_json(char* json, int* out_offset)
{
	char* name = NULL;
	char* value = NULL;

	while (*json != '\0')
	{
		switch (*json)
		{
		case '{':
		case ',':
			*json = '\0';
			if (name != NULL && value != NULL)
			{
				if (strcmp("\"status\"", name) == 0 &&
						strcmp("\"OK\"", value) != 0)
				{
						return false;
				}
				else if (strcmp("\"gmtOffset\"", name) == 0)
				{
					*out_offset = atoi(value);
					return true;
				}
			}
			name = ++json;
			break;
		case ':':
			*json = '\0';
			value = ++json;
			break;
		default:
			json++;
			break;
		}
	}

	return false;
}

#ifdef SELECT_COMM_WLAN

static void wlanRecvStartAT(void)
{
	wlanDMAenabled = false;
	HAL_UART_DMAStop(WLAN_UART);

	binNeed = 0;
	lineMode = true;

	linePos = 0;
	linesCnt = 0;
	wlanRecvReadPos = 0;

	HAL_UART_Receive_DMA(WLAN_UART, &wlanRecvDMABuf[0], WLAN_BUF_SIZE);
	wlanDMAenabled = true;
}

// Wait for speciffic lines count on receiver
static bool wlanRecvWaitLines(size_t linesCount, uint32_t timeout)
{
	uint32_t i = 0;
	do
	{
		wlanRecvExec();
		if (linesCnt >= linesCount) return true;
		osDelay(100);
		i += 100;
	}
	while (i < timeout);

	return false;
}

// req must be null terminated string, without \n
// resp must be null terminated string, without \n, or NULL if not need to check
static bool wlanRequestAT(const char* req, const char* resp, uint32_t timeout)
{
	wlanRecvStartAT();
	if (!transport_sendPacketBuffer(0, (uint8_t*)req, strlen(req))) return false;
	if (!transport_sendPacketBuffer(0, (uint8_t*)"\n", 1)) return false;
	if (!wlanRecvWaitLines(2, timeout)) return false;
	if (strcmp(lineBuf[0], req) != 0) return false;
	if (resp != NULL && strcmp(lineBuf[1], resp) != 0) return false;
	return true;
}

static bool wlanReadAT(const char* req, char* out_resp, size_t resp_max_size,
		uint32_t timeout)
{
	wlanRecvStartAT();
	if (!transport_sendPacketBuffer(0, (uint8_t*)req, strlen(req))) return false;
	if (!transport_sendPacketBuffer(0, (uint8_t*)"\n", 1)) return false;
	if (!wlanRecvWaitLines(2, timeout)) return false;
	if (strcmp(lineBuf[0], req) != 0) return false;
	if (out_resp != NULL)
		strncpy(out_resp, lineBuf[1], resp_max_size);
	return true;
}

static bool wlanSwitchToAT(void)
{
	volatile int i;
	uint8_t ansBuf[1];

	wlanDMAenabled = false;
	HAL_UART_DMAStop(WLAN_UART);
	HAL_UART_DeInit(WLAN_UART);
	MX_USART6_UART_Init();

	for (i = 0; i < 10; i++)
	{
		// Send +++ sequence and get response
		ansBuf[0] = '\0';
		if (!transport_sendPacketBuffer(0, (uint8_t*)"+++", 3)) return 0;
		HAL_UART_Receive(WLAN_UART, ansBuf, 1, 2000);
		if (ansBuf[0] == 'a')
		{
			wlanRecvStartAT();
			if (!transport_sendPacketBuffer(0, (uint8_t*)"a", 1)) return 0;
			if (wlanRecvWaitLines(1, 1000) &&
					(strcmp(lineBuf[0], "+ok") == 0)) break;
		}
		else if (ansBuf[0] == '+')
		{
			break;
		}
	}
	if (i == 10) return false;
	return true;
}

static bool wlanSwitchToAT57600(void)
{
	volatile int i;
	uint8_t ansBuf[1];

	wlanDMAenabled = false;
	HAL_UART_DMAStop(WLAN_UART);
	HAL_UART_DeInit(WLAN_UART);
	//WLAN_UART->Instance = USART6;
	WLAN_UART->Init.BaudRate = 57600;
	WLAN_UART->Init.WordLength = UART_WORDLENGTH_8B;
	WLAN_UART->Init.StopBits = UART_STOPBITS_1;
	WLAN_UART->Init.Parity = UART_PARITY_NONE;
	WLAN_UART->Init.Mode = UART_MODE_TX_RX;
	WLAN_UART->Init.HwFlowCtl = UART_HWCONTROL_NONE;
	WLAN_UART->Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(WLAN_UART);

	for (i = 0; i < 10; i++)
	{
		// Send +++ sequence and get response
		ansBuf[0] = '\0';
		if (!transport_sendPacketBuffer(0, (uint8_t*)"+++", 3)) return 0;
		HAL_UART_Receive(WLAN_UART, ansBuf, 1, 2000);
		if (ansBuf[0] == 'a')
		{
			wlanRecvStartAT();
			if (!transport_sendPacketBuffer(0, (uint8_t*)"a", 1)) return 0;
			if (wlanRecvWaitLines(1, 1000) &&
					(strcmp(lineBuf[0], "+ok") == 0)) break;
		}
		else if (ansBuf[0] == '+')
		{
			break;
		}
	}
	if (i == 10) return false;
	return true;
}

// 1 - ok, 0 - error
int transport_sendPacketBuffer(int sock, unsigned char* buf, int buflen)
{
	return (HAL_UART_Transmit(WLAN_UART, &buf[0], buflen, WLAN_TX_TIMEO) == HAL_OK);
}


bool wlan_first_config(void)
{
	// Try switching to AT with current speed
	if (!wlanSwitchToAT())
	{
		// Try switching to AT with 57600
		if (!wlanSwitchToAT57600()) return false;
	}
	RGB_Color_Blink(RGB_COLOR_YELLOW);

	// Disable ETH1
	if (!wlanRequestAT("AT+EPHYA=off", "+ok", 2000));// return false;
	RGB_Color_Blink(RGB_COLOR_YELLOW);

	// Set ETH2 to WAN
	if (!wlanRequestAT("AT+FVEW=enable", "+ok", 5000)) return false;
	RGB_Color_Blink(RGB_COLOR_YELLOW);

	// Set UART parameters
	if (!wlanRequestAT("AT+UART=115200,8,1,NONE,NFC", "+ok", 2000)) return false;
	RGB_Color_Blink(RGB_COLOR_YELLOW);

	// Disable DHCP server
	if (!wlanRequestAT("AT+DHCPDEN=off", "+ok", 2000)) return false;
	RGB_Color_Blink(RGB_COLOR_YELLOW);

	// Disable SocketB
	if (!wlanRequestAT("AT+TCPB=off", "+ok", 2000)) return false;
	RGB_Color_Blink(RGB_COLOR_YELLOW);

	// Set AP IP - must be in different subnet from STA address
	if (!wlanRequestAT(WLAN_SERVER_CONFIG, "+ok", 2000)) return false;
	RGB_Color_Blink(RGB_COLOR_YELLOW);

	// Set STA/Client/Wan IP - must be in different subnet from AP address
	// Disable DHCP and set static IP
	if (!wlanRequestAT(WLAN_CLIENT_CONFIG, "+ok", 2000)) return false;
	RGB_Color_Blink(RGB_COLOR_YELLOW);

	if (!wlanRequestAT("AT+Z", "+ok", 2000)) return false;
	RGB_Color_Blink(RGB_COLOR_YELLOW);

	return true;
}

bool wlan_update_time(void)
{
	char buf[256];
	uint32_t i, j;

	for (j = 0; j < 3; j++)
	{
		// Go to command mode
		if (!wlanSwitchToAT()) return false;
		RGB_Color_Blink(RGB_COLOR_YELLOW);

		// Remove commands garbage
		wlanRequestAT("AT", NULL, 1000);
		RGB_Color_Blink(RGB_COLOR_YELLOW);

		// Pass host and port using AT commands to the WIFI module
		sprintf(buf, "AT+NETP=TCP,CLIENT,%i,%s",
				TIME_SERVER_PORT, TIME_SERVER_NAME);
		if (!wlanRequestAT(buf, "+ok", 2000)) return 0;
		RGB_Color_Blink(RGB_COLOR_YELLOW);

		// Disable socket B
		if (!wlanRequestAT("AT+TCPB=off", "+ok", 2000)) return 0;
		RGB_Color_Blink(RGB_COLOR_YELLOW);

		// Reboot module and receive time
		if (!wlanRequestAT("AT+Z", "+ok", 2000)) return 0;
		RGB_Color_Blink(RGB_COLOR_YELLOW);
		binPos = 0;
		binNeed = 4;
		for(i = 0; i < 30000 && binNeed > 0; i += 1000)
		{
			wlanRecvExec();
			osDelay(1000);
		}
		if (binNeed > 0) continue;
		RGB_Color_Blink(RGB_COLOR_YELLOW);

		// Parse time
		uint64_t time_bin = binBuf[0];
		time_bin = (time_bin << 8) | binBuf[1];
		time_bin = (time_bin << 8) | binBuf[2];
		time_bin = (time_bin << 8) | binBuf[3];

		time_t t = time_bin - 2208988800ul;
		struct tm *lt = gmtime(&t);
		strftime(buf, 256, "%Y/%m/%d,%H:%M:%S", lt);
		SetDateTime(buf);
		RGB_Color_Blink(RGB_COLOR_YELLOW);

		return true;
	}

	return false;
}

bool wlan_update_tz(void)
{
	char buf[256];
	uint32_t i;
	char rbuf[1024];
	char* rbufptr = rbuf;
	size_t rbufleft = 1024;

	// Go to command mode
	if (!wlanSwitchToAT()) return false;
	RGB_Color_Blink(RGB_COLOR_YELLOW);

	// Remove commands garbage
	wlanRequestAT("AT", NULL, 1000);
	RGB_Color_Blink(RGB_COLOR_YELLOW);

	// Pass host and port using AT commands to the WIFI module
	sprintf(buf, "AT+NETP=TCP,CLIENT,%i,%s",
			TZ_SERVER_PORT, TZ_SERVER_NAME);
	if (!wlanRequestAT(buf, "+ok", 2000)) return 0;
	RGB_Color_Blink(RGB_COLOR_YELLOW);

	// Disable socket B
	if (!wlanRequestAT("AT+TCPB=off", "+ok", 2000)) return 0;
	RGB_Color_Blink(RGB_COLOR_YELLOW);

	// Reboot module
	if (!wlanRequestAT("AT+Z", "+ok", 2000)) return 0;
	RGB_Color_Blink(RGB_COLOR_YELLOW);

	// Wait till connected
	osDelay(15000);

	// Get lat and lon
	char* lat = GetVariableThreadSafe("GPSLAT");
	char* lon = GetVariableThreadSafe("GPSLON");

	// Send request
	sprintf(buf, "GET /v2/get-time-zone?key=%s&format=json&by=position&lat=%s&lng=%s HTTP/1.1\r\nHost: %s\r\n\r\n",
			TZ_API_KEY, lat, lon, TZ_SERVER_NAME);
	transport_sendPacketBuffer(0, (uint8_t*)buf, strlen(buf));

	// Get response
	for(i = 0; i < 5000; i += 1000)
	{
		size_t bytes_read = wlanRecvToBufExec((uint8_t*)rbufptr, rbufleft);
		rbufptr += bytes_read;
		rbufleft -= bytes_read;
		osDelay(1000);
		RGB_Color_Blink(RGB_COLOR_YELLOW);
	}

	// Search for JSON start
	size_t resp_size = 1024 - rbufleft;
	char* json = NULL;
	for (i = 0; i < resp_size && json == NULL; i++)
		if (rbuf[i] == '{')
			json = &rbuf[i];

	if (json == NULL) return false;

	// Parse JSON
	int offset = 0;
	if (!wlan_parse_tz_json(json, &offset)) return false;
	sprintf(buf, "%i", (offset/3600));

	SetVariableThreadSafe("UTCOF", buf);

	return true;
}

int transport_open(const char* host, int port)
{
	char buf[256];

	// Give module some time to start up
	osDelay(5000);

	// Init Uart
	wlanDMAenabled = false;
	HAL_UART_DMAStop(WLAN_UART);
	HAL_UART_DeInit(WLAN_UART);
	MX_USART6_UART_Init();
	RGB_Color_Blink(RGB_COLOR_YELLOW);

#ifdef PERFORM_WLAN_FIRST_TIME_CONFIG
	// Config WiFi module disabling DHCP etc
	if (!wlan_first_config()) return 0;
#endif

#ifdef WLAN_USE_TIME_SERVER
	// Get time from server
	if (GetVariableThreadSafe("TIMEFL")[0] == '0') wlan_update_time();
#endif

	wlan_update_tz();

	// Connect to MQTT

	// Go to command mode
	if (!wlanSwitchToAT()) return 0;
	RGB_Color_Blink(RGB_COLOR_YELLOW);

	// Get IMEI(MAC)
	char IMEIbuf[32] = { 0 };
	wlanReadAT("AT+WMAC", IMEIbuf, 32, 1000);
	SetIMEI(&IMEIbuf[2]);
	RGB_Color_Blink(RGB_COLOR_YELLOW);

	// Remove commands garbage
	wlanRequestAT("AT", NULL, 1000);
	RGB_Color_Blink(RGB_COLOR_YELLOW);

	// Pass host and port using AT commands to the WIFI module
	sprintf(buf, "AT+NETP=TCP,CLIENT,%i,%s", port, host);
	if (!wlanRequestAT(buf, "+ok", 2000)) return 0;
	RGB_Color_Blink(RGB_COLOR_YELLOW);

	// Disable socket B
	if (!wlanRequestAT("AT+TCPB=off", "+ok", 2000)) return 0;
	RGB_Color_Blink(RGB_COLOR_YELLOW);

	// Reboot module
	if (!wlanRequestAT("AT+Z", "+ok", 2000)) return 0;
	RGB_Color_Blink(RGB_COLOR_YELLOW);

	// Wait for connect
	osDelay(WLAN_CONNECT_TIME); // Increase this time if using slow connection

	wlanRecvStartMQTT();
	RGB_Color_Blink(RGB_COLOR_YELLOW);
	return 1;
}

int transport_close(int sock)
{
	wlanDMAenabled = false;
	HAL_UART_DMAStop(WLAN_UART);
	HAL_UART_DeInit(WLAN_UART);

	wlanReadIndexMqttCtrl = 0;
	wlanReadIndexMqttPub = 0;
	wlanWriteIndexMqttCtrl = 0;
	wlanWriteIndexMqttPub = 0;
	mqttRecvState = MqttReceiverStatePending;
	wlanRecvReadPos = 0;

	return 1;
}

#endif
