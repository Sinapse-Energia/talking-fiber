/*
 * M95bis.c
 *
 *  Created on: Jun 22, 2017
 *      Author: External
 */
#ifdef FAMILY_F0
#include "stm32f0xx_hal.h"
#endif

#include "string.h"
#include "M95lite.h"
#include "Definitions.h"


//extern uint16_t LOG_ACTIVATED;
//extern uint8_t LOG_GPRS;


//extern uint8_t nTimesMaximumFail_GPRS;
//extern uint8_t retriesGPRS;
//extern uint8_t APN[SIZE_APN];


int FaseConn = 0;


uint8_t receiveString2(
		UART_HandleTypeDef *phuart,
		uint8_t *timeoutGPRS,
		unsigned char *messageOrigin,
		unsigned char *messageSubst,
		uint32_t timeout,
		uint8_t retries,
		uint8_t clearingBuffer,
		IRQn_Type IRQn,
		uint8_t *receivedBuffer,
		uint16_t sizeMAXReceivedBuffer,
		uint8_t *dataByteBufferIRQ,
		uint16_t *numberBytesReceived)
{

	uint16_t i = 0;
	uint16_t j = 0;
	uint16_t initialCounter=0;


	while ((i < retries)) {


	   if (clearingBuffer == 1) cleanningReceptionBuffer(IRQn, receivedBuffer,sizeMAXReceivedBuffer, numberBytesReceived);
	    HAL_UART_Receive_IT(phuart, dataByteBufferIRQ, 1); // Enabling IRQ
		HAL_UART_Transmit(phuart, messageOrigin, strlen((char *) messageOrigin), timeout);

		initialCounter = *numberBytesReceived;

//		if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
		*timeoutGPRS = 0;

		HAL_Delay(750);


		// convert to tics of 50 mseg
		timeout = (timeout / 20) ;
		while ((initialCounter + (strlen((char *) messageSubst))	> *numberBytesReceived) && (--timeout > 0)) {
			HAL_Delay(50);
		}

		/**
		if ((LOG_ACTIVATED==1)&(LOG_GPRS==1)) {
			HAL_UART_Transmit(phuartLOG, (uint8_t*) "->", 2, 100);
			HAL_UART_Transmit(phuartLOG, messageOrigin, lengthMessageOrigin, 100);
			HAL_UART_Transmit(phuartLOG, (uint8_t*) "\r", 1, 100);
			HAL_UART_Transmit(phuartLOG, (uint8_t*) "<-", 2, 100);
			HAL_UART_Transmit(phuartLOG, receivedBuffer,*numberBytesReceived, 100);
			HAL_UART_Transmit(phuartLOG, (uint8_t*) "\r", 1, 100);
		}
		**/


		j = 0;  // para offsetar
		while (j + strlen((char *) messageSubst) <= *numberBytesReceived) {
			if (strstr((char *) receivedBuffer + j, (char *) messageSubst)){
				return 1;
			}
			else {
				j = j +  strlen((char *) receivedBuffer)+1;
			}
		}
		i = i + 1;

	}

	return 0;

}

#if 0

struct	st_atcmdprops {
	char	*command;
	char	*reply;
	int		timeout;
	int		nretries;
};



uint8_t  M95_C2(
		uint8_t WDT_ENABLED, IWDG_HandleTypeDef *hiwdg,
		UART_HandleTypeDef *phuart,
		UART_HandleTypeDef *phuartLOG,
		uint8_t *timeoutGPRS,
		uint32_t timeout,
		uint8_t *rebootSystem,
		uint8_t existDNS,
		uint8_t *idSIM,

		uint8_t setTransparentConnection,

		IRQn_Type IRQn,
		uint8_t *receivedBuffer,
		uint16_t sizeMAXReceivedBuffer,
		uint8_t *dataByteBufferIRQ,
		uint16_t *numberBytesReceived


		) {
	uint8_t valid = 0;
	uint8_t counter = 0;
	char messageTX[100];
	uint8_t lengthMessageTX = 0;
	char *dnscmd;
	int	i;
	int j;


#if 0
	dnscmd = existDNS?"AT+QIDNSIP=1\r":"AT+QIDNSIP=0\r";
	struct st_atcmdprops	atCommands1[] = {
			"ATE0\r", 								"\r\nOK\r\n", timeout, retriesGPRS,
			"AT+QIFGCNT=0\r",						"\r\nOK\r\n", timeout, retriesGPRS,
			messageTX,								"\r\nOK\r\n", timeout, retriesGPRS,
			"ATE0\r",								"\r\nOK\r\n", timeout, retriesGPRS,
			existDNS?
			 "AT+QIDNSIP=1\r":"AT+QIDNSIP=0\r", 	"\r\nOK\r\n", timeout, retriesGPRS,
			"AT+QIMUX=0\r", 						"\r\nOK\r\n", timeout, retriesGPRS,
			setTransparentConnection?
			"AT+QIMODE=1\r":"AT+QIMODE=0\r", 		"\r\nOK\r\n", timeout, retriesGPRS,
			"AT+QITCFG=3,2,512,1\r",				"\r\nOK\r\n", timeout, retriesGPRS,
			"AT+QCCID\r", 							"\r\nOK\r\n", timeout, retriesGPRS
	};


	struct st_atcmdprops	atCommands2[] = {
			"AT+CREG?\r", 							"0,5", 		  timeout, retriesGPRS,
			"AT+CGREG?\r", 							"0,5", 		  timeout, retriesGPRS,


	};

	strcpy(messageTX, "AT+QICSGP=1,");

	strcat(messageTX, (char *) APN);
	lengthMessageTX = strlen( messageTX);


	for (i = 0; i < sizeof(atCommands1)/sizeof(atCommands1[0]); i++){
		if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);

		valid = receiveString2(phuart,
				timeoutGPRS,
				(unsigned char *) atCommands1[i].command, (unsigned char *) atCommands1[i].reply, atCommands1[i].timeout,atCommands1[i].nretries,1,
				IRQn,
				receivedBuffer,
				sizeMAXReceivedBuffer,
				dataByteBufferIRQ,
				numberBytesReceived);

		if (valid == 0) {

			for (counter = 0; counter < 4; counter++) {
				if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
				HAL_Delay(10000);
				if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);
			}

			*rebootSystem=1;
			return M95_GPRS_FAIL;
		}
		FaseConn++;
	}

	for (j = 0; j < 20; j++) idSIM[j] = receivedBuffer[j]; //
	idSIM[j] = '\0'; //ending character

	FaseConn++;

	for (i = 0; i < sizeof(atCommands2)/sizeof(atCommands2[0]); i++){
		if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);

		valid = receiveString2(phuart,
				timeoutGPRS,
				(unsigned char *)atCommands2[i].command, (unsigned char *) atCommands2[i].reply, atCommands2[i].timeout,atCommands2[i].nretries,1,
				IRQn,
				receivedBuffer,
				sizeMAXReceivedBuffer,
				dataByteBufferIRQ,
				numberBytesReceived);

		if (valid == 0) {

			for (counter = 0; counter < 4; counter++) {
				if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
				HAL_Delay(10000);
				if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);
			}

			*rebootSystem=1;
			return M95_GPRS_FAIL;
		}
		FaseConn++;
	}

#else

	valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
							timeoutGPRS,
							(uint8_t*) "ATE0\r",5,
							(uint8_t*) "\r\nOK\r\n", 6,
							timeout,
							retriesGPRS,1,
							IRQn,
							receivedBuffer,
							sizeMAXReceivedBuffer,
							dataByteBufferIRQ,
							numberBytesReceived);

	if (valid == 0) {

		for (counter = 0; counter < 4; counter++) {
			if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
			HAL_Delay(10000);
			if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);
		}

		*rebootSystem=1;
		return M95_GPRS_FAIL;
	}


	if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);

	valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
								timeoutGPRS,
								(uint8_t*) "AT+QIFGCNT=0\r",13,
								(uint8_t*) "\r\nOK\r\n", 6,
								timeout,
								retriesGPRS,1,
								IRQn,
								receivedBuffer,
								sizeMAXReceivedBuffer,
								dataByteBufferIRQ,
								numberBytesReceived);


	if (valid == 0) {

		for (counter = 0; counter < 4; counter++)
		{
			if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
			HAL_Delay(10000);
			if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);
		}

		*rebootSystem=1;
		return M95_GPRS_FAIL;
	}


	if (WDT_ENABLED == 1)	HAL_IWDG_Refresh(hiwdg);


	valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
							timeoutGPRS,
							messageTX,lengthMessageTX,
							(uint8_t*) "\r\nOK\r\n", 6,
							timeout,
							retriesGPRS,1,
							IRQn,
							receivedBuffer,
							sizeMAXReceivedBuffer,
							dataByteBufferIRQ,
							numberBytesReceived);

	if (valid == 0) {
		for (counter = 0; counter < 4; counter++)
		{
			if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
			HAL_Delay(10000);
			if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);
		}

		*rebootSystem=1;
		return M95_GPRS_FAIL;
	}


	if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);

	valid = receiveString(LOG_ACTIVATED,LOG_GPRS,WDT_ENABLED,hiwdg,phuart,phuartLOG,
							timeoutGPRS,
							(uint8_t*) "ATE0\r",5,
							(uint8_t*) "\r\nOK\r\n", 6,
							timeout,
							retriesGPRS,1,
							IRQn,
							receivedBuffer,
							sizeMAXReceivedBuffer,
							dataByteBufferIRQ,
							numberBytesReceived);

		if (valid == 0)
		{

			for (counter = 0; counter < 4; counter++) {
				if (WDT_ENABLED == 1) HAL_IWDG_Refresh(hiwdg);
				HAL_Delay(10000);
				if (WDT_ENABLED == 1)HAL_IWDG_Refresh(hiwdg);
		}

		*rebootSystem=1;
		return M95_GPRS_FAIL;
	}


	if (WDT_ENABLED == 1)	HAL_IWDG_Refresh(hiwdg);
#endif

		return 1;
}





M95Status M95_Connect1(
		uint8_t LOG_ACTIVATED,
		uint8_t LOG_GPRS,
		uint8_t WDT_ENABLED,
		IWDG_HandleTypeDef *hiwdg,
		UART_HandleTypeDef *phuart,
		UART_HandleTypeDef *phuartLOG,
		uint8_t *timeoutGPRS,
		uint32_t timeout,
		uint8_t *rebootSystem,
		GPIO_TypeDef* ctrlEmerg_PORT, uint16_t ctrlEmerg_PIN,
		GPIO_TypeDef* ctrlPwrkey_PORT, uint16_t ctrlPwrkey_PIN,
		GPIO_TypeDef* m95Status_PORT, uint16_t m95Status_PIN,
		uint8_t nTimesMaximumFail_GPRS,
		uint8_t retriesGPRS,
		uint8_t existDNS,
		uint8_t offsetLocalHour,
		uint8_t *APN,
		uint8_t *IPPORT,
		uint8_t *SERVER_NTP,
		uint8_t *calendar,
		uint8_t *idSIM,
		uint8_t *openFastConnection,
		uint8_t setTransparentConnection,
		IRQn_Type IRQn,
		uint8_t *receivedBuffer,
		uint16_t sizeMAXReceivedBuffer,
		uint8_t *dataByteBufferIRQ,
		uint16_t *numberBytesReceived  ) {

		M95_C1(WDT_ENABLED, hiwdg, phuart, phuartLOG, ctrlEmerg_PORT, ctrlEmerg_PIN, ctrlPwrkey_PORT, ctrlPwrkey_PIN, m95Status_PORT, m95Status_PIN);
		M95_C2(WDT_ENABLED, hiwdg, phuart, phuartLOG,
				timeoutGPRS, timeout, rebootSystem, existDNS, idSIM, setTransparentConnection,
				IRQn, receivedBuffer, sizeMAXReceivedBuffer, dataByteBufferIRQ, numberBytesReceived);
	return 1;
}

#endif

