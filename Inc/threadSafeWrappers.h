/*
 * threadSafeWrappers.h
 *
 *  Created on: Sep 7, 2017
 *      Author: Sviatoslav
 */

#ifndef THREADSAFEWRAPPERS_H_
#define THREADSAFEWRAPPERS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "cmsis_os.h"
#include "Shared.h"

void 	take						(xSemaphoreHandle mutex);
void 	give						(xSemaphoreHandle mutex);

void	ThreadsafeWrappersInit		(void);

int		MqttSubscribeThreadSafe		(int handle, char *topic);
int		MqttPutMessageThreadSafe	(int handle, char	*topic, char *message);
int		MqttPingThreadSafe			(int handle);
int		ReconnectThreadSafe			(int *handle);
void 	TraceThreadSafe				(int handle, char *texto, ...);
void 	TraceThreadSafeTopic		(int handle, char *topic, char *texto, ...);
void 	TraceThreadSafeFromContext	(int handle, char *texto, ...);
void 	TraceThreadSafeFromContextTopic	(int handle, char *topic, char *texto, ...);

char	*ProcessMessageThreadSafe	(char *message);
char	*GetVariableThreadSafe		(char *name);
int		SetVariableThreadSafe		(char *name, char *value);
int		SaveDevParamsToNVMThreadSafe(void);

void	ContextLock(void);
void	ContextUnlock(void);

bool ReadSharedMemory_ThreadSafe(SharedMemoryData* outData);
bool WriteSharedMemory_ThreadSafe(SharedMemoryData* inData);

#ifdef __cplusplus
}
#endif

#endif /* THREADSAFEWRAPPERS_H_ */
