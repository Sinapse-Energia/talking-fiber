/*****************************************************************************
 * Module name: CQ_Gen.h
 *
 * First written on Mar 10, 2018 by Owais.
 *
 * Module Description:
 * Generalized circular queue header
 *
 * Updates:
 *****************************************************************************/
#ifndef __CQ_GEN_H
#define __CQ_GEN_H
/*****************************************************************************
 *  Include section
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "stm32f2xx_hal.h"
#include "Definitions.h"
/*****************************************************************************
 *  #defines and typedefs
 *****************************************************************************/
typedef enum
{	
	CQ_ERROR_OK,
	CQ_ERROR_EMPTY,
	CQ_ERROR_FULL,
}EnumCQErrors;

typedef struct
{
	uint16_t InIndex;
	uint16_t OutIndex;
	bool IsEmpty;
	bool IsFull;
	uint16_t Size;
	uint16_t Capacity; //Capacity is Size-1
	void* PtrData; 	
	uint16_t DT;
}CircularQueue;

/*****************************************************************************
 *  global functions
 *****************************************************************************/
void          CQ_Init(CircularQueue* cq, void* PtrArray, uint16_t ArrayElemCount, uint16_t data_type_size);
EnumCQErrors  CQ_Enqueue(CircularQueue* cq, void*  datum);
void*         CQ_Dequeue(CircularQueue* cq, EnumCQErrors *errorcode);
uint16_t      CQ_AvailableData(CircularQueue* cq);
uint16_t      CQ_AvailableEmptySpace(CircularQueue* cq);

#endif
