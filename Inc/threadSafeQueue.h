/*
 * threadSafeQueue.h
 *
 *  Created on: Aug 21, 2017
 *      Author: Sviatoslav
 */

#ifndef THREADSAFEQUEUE_H_
#define THREADSAFEQUEUE_H_

// Includes and defines
#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* threadSafeQueueHandler;

// Prototypes
threadSafeQueueHandler threadSafeQueueInit(void);
void threadSafeQueueEnqueue(threadSafeQueueHandler qhandler, const char* msg);
bool threadSafeQueueDequeue(threadSafeQueueHandler qhandler, char* buf, size_t bufSize);
size_t threadSafeQueueGetSize(threadSafeQueueHandler qhandler);

#ifdef __cplusplus
}
#endif

#endif /* THREADSAFEQUEUE_H_ */
