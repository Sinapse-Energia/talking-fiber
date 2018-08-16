/*****************************************************************************
 * Module name: CQ_Gen.c
 *
 * First written on Mar 10, 2018 by Owais.
 *
 * Module Description:
 * Generalized circular queue
 *
 * Updates:
 *****************************************************************************/
#include "CQ_Gen.h"

#if defined(BUILD_CANBUS)
/*****************************************************************************
 * Function name    : CQ_Init
 *    returns       : void
 *    arg1          : CircularQueue* cq, the queue to initialize
 *    arg2          : void* PtrArray, the array supplier by client module to insert and remove data from
 *    arg3          : uint16_t ArrayElemCount, Element count which can be successfully stored in CQ, it is one less than array size used to hold items
 *    arg4          : uint16_t Data_type_size, the data type of the items which are inserted in queue
 * Created by       : Owais
 * Date created     : 11-Mar-2018
 * Description      : Initialize CQ
 * Notes            :
 *****************************************************************************/
void CQ_Init(CircularQueue* cq, void* PtrArray, uint16_t ArrayElemCount, uint16_t Data_type_size)
{
	cq->InIndex = 0;
	cq->OutIndex = 0;
	cq->IsFull = false;
	cq->IsEmpty = true;
	cq->Size = ArrayElemCount + 1;
	cq->Capacity = ArrayElemCount;
	cq->PtrData = PtrArray;
	cq->DT = Data_type_size;
}
/*****************************************************************************
 * Function name    : CQ_Enqueue
 *    returns       : EnumCQErrors, the error code for call
 *    arg1          : CircularQueue* cq, the queue to initialize
 *    arg2          : void* datum, pointer to the data item which is to be enqueued
 *                    the actual number of bytes to be enqueued depends on initialized DT field of queue
 * Created by       : Owais
 * Date created     : 11-Mar-2018
 * Description      : Enqueue datum in queue cq data array
 * Notes            :
 *****************************************************************************/
EnumCQErrors CQ_Enqueue(CircularQueue* cq, void* datum)
{
	uint32_t prim;

	if (cq->IsFull)
		return CQ_ERROR_FULL;

	prim = __get_PRIMASK();
	__disable_irq();

	memcpy(cq->PtrData + (cq->InIndex * cq->DT), datum, (size_t)cq->DT); /* Translate into address as per data type size */
	cq->InIndex++;

	cq->IsEmpty = false; //no longer empty when u enqueue
	cq->InIndex = cq->InIndex % cq->Size;

	if (cq->InIndex > cq->OutIndex)
		if ((cq->InIndex - cq->OutIndex) == (cq->Size - 1))
			cq->IsFull = true;

	if (cq->InIndex < cq->OutIndex)
		if ((cq->OutIndex - cq->InIndex) == 1)
			cq->IsFull = true;

	if (!prim)
	{
		__enable_irq();
	}

	return CQ_ERROR_OK;
}
/*****************************************************************************
 * Function name    : CQ_AvailableData
 *    returns       : uint16_t, available data units in queue pending to be removed
 *    arg1          : CircularQueue* cq, the queue to use
 * Created by       : Owais
 * Date created     : 11-Mar-2018
 * Description      : The data items which can be removed from queue
 * Notes            :
 *****************************************************************************/
uint16_t CQ_AvailableData(CircularQueue* cq)
{
	uint16_t result;
	uint32_t prim;

	prim = __get_PRIMASK();
	__disable_irq();

	if (cq->InIndex == cq->OutIndex)
		result = 0;
	else if ((cq->InIndex == (cq->Size - 1) && cq->OutIndex == 0) || (cq->InIndex == (cq->OutIndex - 1)))
		result = cq->Size - 1;
	else if (cq->InIndex < cq->OutIndex)
		result = (cq->InIndex) + (cq->Size - cq->OutIndex);
	else
		result = cq->InIndex - cq->OutIndex;

	if (!prim)
	{
		__enable_irq();
	}

	return result;
}
/*****************************************************************************
 * Function name    : CQ_AvailableEmptySpace
 *    returns       : uint16_t, available empty slots in queue
 *    arg1          : CircularQueue* cq, the queue to use
 * Created by       : Owais
 * Date created     : 11-Mar-2018
 * Description      : The empty slots count which can be filled by datum
 * Notes            :
 *****************************************************************************/
uint16_t CQ_AvailableEmptySpace(CircularQueue* cq)
{
	uint16_t result;

	result = CQ_AvailableData(cq);
	result = cq->Capacity - result;
	return result;
}
/*****************************************************************************
 * Function name    : CQ_Dequeue
 *    returns       : void*, pointer to the data item removed from queue
 *    arg1          : CircularQueue* cq, the queue to dequeue data item from
 *    arg2          : EnumCQErrors *errorcode, out parameter showing the error code of call
 * Created by       : Owais
 * Date created     : 11-Mar-2018
 * Description      : Dequeue an item form the queue
 * Notes            :
 *****************************************************************************/
void* CQ_Dequeue(CircularQueue* cq, EnumCQErrors *errorcode)
{
	void* pDatum = NULL;
	uint32_t prim;

	if (cq->IsEmpty == true)
	{
		*errorcode = CQ_ERROR_EMPTY;
		return 0;
	}

	prim = __get_PRIMASK();
	__disable_irq();

	pDatum = cq->PtrData + (cq->OutIndex * cq->DT);
	cq->IsFull = false;
	cq->OutIndex++;
	cq->OutIndex = cq->OutIndex % cq->Size;

	if (cq->InIndex == cq->OutIndex)
		cq->IsEmpty = true;

	if (!prim)
	{
		__enable_irq();
	}

	*errorcode = CQ_ERROR_OK;
	return pDatum;
}
#endif
