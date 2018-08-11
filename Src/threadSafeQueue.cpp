/*
 * threadSafeQueue.cpp
 *
 *  Created on: Aug 21, 2017
 *      Author: Sviatoslav
 */

#include "threadSafeQueue.h"
#include "threadSafeWrappers.h"
#include <queue>
#include <string>
#include "string.h"
#include <stdbool.h>
#include "cmsis_os.h"

using namespace std;

typedef struct {
	queue<string>* qptr;
	xSemaphoreHandle mutex;
} threadSafeQueue_t;

threadSafeQueueHandler threadSafeQueueInit(void)
{
	threadSafeQueue_t* ret = new threadSafeQueue_t;
	if (ret == NULL)
	{
		return NULL;
	}

	ret->qptr = new queue<string>();
	if (ret->qptr == NULL)
	{
		free(ret);
		return NULL;
	}

	ret->mutex = xSemaphoreCreateMutex();
	if (ret->mutex == NULL)
	{
		free(ret->qptr);
		free(ret);
		return NULL;
	}

	return ret;
}

void threadSafeQueueEnqueue(threadSafeQueueHandler qhandler, const char* msg)
{
	threadSafeQueue_t* ptr = (threadSafeQueue_t*) qhandler;
	take(ptr->mutex);
	ptr->qptr->push(string(msg));
	give(ptr->mutex);
}

bool threadSafeQueueDequeue(threadSafeQueueHandler qhandler, char* buf, size_t bufSize)
{
	threadSafeQueue_t* ptr = (threadSafeQueue_t*) qhandler;
	bool ret = false;
	take(ptr->mutex);
	if (ptr->qptr->size() > 0)
	{
		const char* str = ptr->qptr->front().c_str();
		size_t sz = strlen(str);
		if (sz > (bufSize - 1)) sz = bufSize - 1;
		memcpy(buf, str, sz);
		buf[sz] = '\0';
		ptr->qptr->pop();
		ret = true;
	}
	give(ptr->mutex);
	return ret;
}

size_t threadSafeQueueGetSize(threadSafeQueueHandler qhandler)
{
	threadSafeQueue_t* ptr = (threadSafeQueue_t*) qhandler;
	take(ptr->mutex);
	size_t ret = ptr->qptr->size();
	give(ptr->mutex);
	return ret;
}
