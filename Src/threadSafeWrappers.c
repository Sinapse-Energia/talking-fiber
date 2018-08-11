/*
 * threadSafeWrappers.c
 *
 *  Created on: Sep 7, 2017
 *      Author: Sviatoslav
 */

#include "threadSafeWrappers.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "cmsis_os.h"

#include "Definitions.h"
#include "MQTTAPI.H"
#include "NBinterface.h"
#include "utils.h"

#include "Shared.h"

#ifdef DISP_TIMING
#include	"stm32f4xx_hal.h"
#endif

static xSemaphoreHandle mqttMutex = NULL;
static xSemaphoreHandle contextMutex = NULL;
static xSemaphoreHandle rfMutex = NULL;
static xSemaphoreHandle epdMutex = NULL;
static xSemaphoreHandle sharedMemMutex = NULL;


#ifdef DISP_TIMING
extern int hmqtt;
#endif

void 	take						(xSemaphoreHandle mutex)
{
	if (mutex == NULL) return;
	while (xSemaphoreTake(mutex, portMAX_DELAY) != pdTRUE) osDelay(10);
}

void 	give						(xSemaphoreHandle mutex)
{
	if (mutex == NULL) return;
	xSemaphoreGive(mutex);
}

void	ThreadsafeWrappersInit		(void)
{
	mqttMutex = xSemaphoreCreateMutex();
	contextMutex = xSemaphoreCreateMutex();
	rfMutex = xSemaphoreCreateMutex();
	epdMutex = xSemaphoreCreateMutex();
	sharedMemMutex = xSemaphoreCreateMutex();
}

int		MqttSubscribeThreadSafe		(int handle, char *topic)
{
	int ret = 0;

	take(mqttMutex);
	ret = MqttSubscribe(handle, topic);
	give(mqttMutex);

	return ret;
}

int		MqttPutMessageThreadSafe	(int handle, char	*topic, char *message)
{
	int ret = 0;

	take(mqttMutex);
	ret = MqttPutMessage(handle, topic, message);
	give(mqttMutex);

	return ret;
}

int		MqttPingThreadSafe			(int handle)
{
	int ret = 0;

	take(mqttMutex);
	ret = MqttPing(handle);
	give(mqttMutex);

	return ret;
}

char	*ProcessMessageThreadSafe	(char *message)
{
	char* ret = NULL;

	take(contextMutex);

#ifdef DISP_TIMING
	uint32_t startTick = HAL_GetTick();
#endif
	ret = ProcessMessage(message);
#ifdef DISP_TIMING
	uint32_t endTick = HAL_GetTick();
	TraceThreadSafeFromContext(hmqtt, (char*)"ProcessMessage %lu msec",
			(endTick - startTick));
#endif

	give(contextMutex);

	return ret;
}

char	*GetVariableThreadSafe		(char *name)
{
	char* ret = NULL;

	take(contextMutex);
	ret = GetVariable(name);
	give(contextMutex);

	return ret;
}

int		SetVariableThreadSafe		(char *name, char *value)
{
	int ret = 0;

	take(contextMutex);
	ret = SetVariable(name, value);
	give(contextMutex);

	return ret;
}

int		ReconnectThreadSafe			(int *handle)
{
	take(mqttMutex);

	if (*handle > 0) MqttDisconnect(*handle);
	int ntries = 0;
	do {
		*handle = COMM_Init();
		ntries++;
	} while (*handle <= 0);

	give(mqttMutex);

	// Connection succeed - update shared UPDFW
	take(sharedMemMutex);

	if (!UpdateSharedClientConnected())
	{
#if defined(DEBUG)
		TraceThreadSafe(*handle, "Update shared memory failed!");
#endif
	}

	give(sharedMemMutex);

	return ntries;
}

int		SaveDevParamsToNVMThreadSafe(void)
{
	int ret = 0;

	take(contextMutex);
	ret = SaveDevParamsToNVM();
	give(contextMutex);

	return ret;
}

void 	TraceThreadSafe				(int handle, char *texto, ...)
{
#ifdef DEBUG
	va_list	  ap;
	char	  salida[512];

	va_start	  (ap, texto);

	if (handle <= 0) return;

	sprintf (salida, "Node %s <%s> ", GetVariableThreadSafe("ID"),
			strDateTime());
	vsprintf (salida+strlen(salida), texto, ap);

	take(mqttMutex);
	MqttPutMessage(handle, topictr, salida);
	give(mqttMutex);
#endif
}

void 	TraceThreadSafeTopic		(int handle, char *topic, char *texto, ...)
{
#ifdef DEBUG
	va_list	  ap;
	char	  salida[512];

	va_start	  (ap, texto);

	if (handle <= 0) return;

	sprintf (salida, "Node %s <%s> ", GetVariableThreadSafe("ID"),
			strDateTime());
	vsprintf (salida+strlen(salida), texto, ap);

	take(mqttMutex);
	MqttPutMessage(handle, topic, salida);
	give(mqttMutex);
#endif
}

void 	TraceThreadSafeFromContext	(int handle, char *texto, ...)
{
#ifdef DEBUG
	va_list	  ap;
	char	  salida[512];

	va_start	  (ap, texto);

	if (handle <= 0) return;

	sprintf (salida, "Node %s <%s> ", GetVariable("ID"), strDateTime());
	vsprintf (salida+strlen(salida), texto, ap);

	take(mqttMutex);
	MqttPutMessage(handle, topictr, salida);
	give(mqttMutex);
#endif
}

void 	TraceThreadSafeFromContextTopic	(int handle, char *topic, char *texto, ...)
{
#ifdef DEBUG
	va_list	  ap;
	char	  salida[512];

	va_start	  (ap, texto);

	if (handle <= 0) return;

	sprintf (salida, "Node %s <%s> ", GetVariable("ID"), strDateTime());
	vsprintf (salida+strlen(salida), texto, ap);

	take(mqttMutex);
	MqttPutMessage(handle, topic, salida);
	give(mqttMutex);
#endif
}

void	ContextLock(void)
{
	take(contextMutex);
}

void	ContextUnlock(void)
{
	give(contextMutex);
}

bool ReadSharedMemory_ThreadSafe(SharedMemoryData* outData)
{
	take(sharedMemMutex);
	bool ret = ReadSharedMemory(outData);
	give(sharedMemMutex);
	return ret;
}

bool WriteSharedMemory_ThreadSafe(SharedMemoryData* inData)
{
	take(sharedMemMutex);
	bool ret = WriteSharedMemory(inData);
	give(sharedMemMutex);
	return ret;
}
