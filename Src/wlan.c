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
