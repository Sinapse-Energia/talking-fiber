/*
 * parametrizacion.cpp
 *
 *  Created on: 9 ago. 2017
 *      Author: juanra
 */

#include	"context.h"
#include	"Frames.h"
#include	"Definitions.h"
#include	"NBinterface.h"
#include	"crc16.h"
#include	"wrsouth.h" 
#include	"utils.h"
#include	"Shared.h"
#include    "MQTTAPI.H"

#ifdef USE_SD_CARD
#include    "fatfs.h"
#endif

#include	"stm32f2xx_hal.h"
#include	"gpio.h"

extern int hmqtt;

#define TYPE_STR    0
#define TYPE_INT    1
#define TYPE_DEC    2

typedef struct DevParamTypeStruct
{
    const char* name;
    const char* defval;
    uint8_t type;

} DevParam_T;

static const DevParam_T devParams[] = {
        { "ANAME", "www.ab.kyivstar.net",               TYPE_STR },
        { "AUSER", "",                                  TYPE_STR },
        { "APSWD", "",                                  TYPE_STR },
        { "MURI",  "soporte-tecnico.bitnamiapp.com",    TYPE_STR },
        { "MUSER", "",                                  TYPE_STR },
        { "MPSWD", "",                                  TYPE_STR },
        { "BMURI", "soporte-tecnico.bitnamiapp.com",    TYPE_STR },
        { "BMUSR", "",                                  TYPE_STR },
        { "BMPSW", "",                                  TYPE_STR },
        { "MTPRT", "TESTING_TF",                        TYPE_STR },
        { "DODPT", "TF/MEASUREMENTS",                   TYPE_STR },
        { "DOPPT", "TF/PERIODIC",                       TYPE_STR },
        { "DALPT", "TF/ALERTS",                         TYPE_STR },
        { "ID",    "999998",                            TYPE_STR },
        { "DGRID", "FFFFFF",                            TYPE_STR },
        { "GPSSSH", "21",                               TYPE_INT },
        { "GPSSSM", "0",                                TYPE_INT },
        { "GPSSRH", "6",                                TYPE_INT },
        { "GPSSRM", "0",                                TYPE_INT },
        { "GPSLAT", "51",                               TYPE_STR },
        { "GPSLON", "0",                                TYPE_STR },
        { "MPORT", "1883",                              TYPE_INT },
        { "BMPRT", "1883",                              TYPE_INT },
        { "UTCOF", "0",                                 TYPE_INT },
        { "BTLSER",   "",                               TYPE_STR },
        { "BTLPRT",   "",                               TYPE_INT },
        { "BTLPRO",   "",                               TYPE_STR },
        { "BTLPAT",   "",                               TYPE_STR },
        { "BTLNAM",   "",                               TYPE_STR },
        { "BTLUSR",   "",                               TYPE_STR },
        { "BTLPAS",   "",                               TYPE_STR },
        { "BTLCNT",   "",                               TYPE_INT },
        { "BTLAUS",   "",                               TYPE_STR },
        { "BTLAPS",   "",                               TYPE_STR },
        { "BTLVER",   "",                               TYPE_STR },
        { "BTLRES",   "0",                              TYPE_INT },

		{ "TFVOL",    "-1",                             TYPE_STR },
		{ "TFVTS",    "0",                              TYPE_INT },
		{ "TFPER",    DEFAULT_PERIOD,                   TYPE_INT },
		{ "TFALM",    DEFAULT_ALERT_M,                  TYPE_INT },
		{ "TFATH",    DEFAULT_ALERT_TH,                 TYPE_INT },

		{ "LISEN",    "1",                              TYPE_INT },
		{ "TLSTH",    "0",                              TYPE_INT },
		{ "TLSTM",    "0",                              TYPE_INT },
		{ "TLDUR",    "60",                             TYPE_INT },
		{ "TLRTN",    "10",                             TYPE_INT },
		{ "TLRES",    "0",                              TYPE_INT },

		{ "TSPER",    "5",                              TYPE_INT },
		{ "TSLDR",    "1",                              TYPE_INT },
		{ "TSRTN",    "10",                             TYPE_INT },
		{ "TSRES",    "0",                              TYPE_INT },

		{ "TAPER",    "10",                             TYPE_INT },
		{ "TATHL",    "500",                            TYPE_INT },
		{ "TATHH",    "1000",                           TYPE_INT },
		{ "TARES",    "0",                              TYPE_INT },
};

#define DEVICE_PARAMS_CNT (sizeof(devParams)/sizeof(devParams[0]))

#define DEV_PARAMS_BUF_SIZE (5*1024ul)

static char devParamsBuf[DEV_PARAMS_BUF_SIZE];

char* UnpackDevVariable(const char* name, char* buf, size_t len);

#ifdef USE_SD_CARD
static void SDCardErrorHandler()
{
	// Blink for 30 sec, than reboot

	for (uint32_t i = 0; i < 3000; i += 250)
	{
	    RGB_Color_Blink(RGB_COLOR_RED);
		HAL_Delay(250);
	}

	tprintf(hmqtt, (char*)"SDCardError: Fatal error");

	NVIC_SystemReset();
}

static void SDCardStart()
{
    /*##-1- Link the micro SD disk I/O driver ################################*/
	/* If this fails - it is fatal error */
	if (FATFS_LinkDriver(&SD_Driver, SDPath) != 0)
		SDCardErrorHandler();

	/*##-2- Register the file system object to the FatFs module ##############*/
	/* If this fails - it is fatal error */
	FRESULT res = f_mount(&SDFatFS, SDPath, 1);
	if(res != FR_OK)
		SDCardErrorHandler();
}

static void SDCardRestart()
{
    tprintf(hmqtt, (char*)"Restarting SD driver");

	/*##-2- Unmount the file system object ###################################*/
	/* If this fails - it is fatal error */
	FRESULT res = f_mount(NULL, SDPath, 1);
	if(res != FR_OK)
		SDCardErrorHandler();

	/*##-1- Unlink the micro SD disk I/O driver ##############################*/
	/* If this fails - it is fatal error */
	if (FATFS_UnLinkDriver(SDPath) != 0)
		SDCardErrorHandler();

	/* Start the SD Driver */
	SDCardStart();
}

/**
 * @param dname data file name without extension, 27 characters max
 * @param data bytes array to write
 * @param sz size of array to write
 * @return true if success
 */
static bool SDCardWriteDataFile(const char* dname, const void* data, size_t sz)
{
	FRESULT res;
	FIL file;
	UINT byteswrite;
	UINT writesize = (UINT)sz;
	char name[2][32];
	size_t i;
	bool done;
	uint32_t retries;

	sprintf(name[0], "%s.SHD", dname);
	sprintf(name[1], "%s.TXT", dname);

	// Write shadow file and main file
	for (i = 0; i < 2; i++)
	{
		done = false;
		retries = 0;
		while (!done)
		{
			res = f_open(&file, name[i], FA_CREATE_ALWAYS | FA_WRITE);
			if(res == FR_OK)
			{
				byteswrite = 0;
				res = f_write(&file, data, writesize, &byteswrite);
				f_close(&file);
				if (res != FR_OK || byteswrite != writesize)
				    tprintf(hmqtt,
				            (char*)"SDCardWrite: %s write error %i, size %i",
				            name[i], (int)res, (int)byteswrite);
				else
					done = true;
			}
			else
			    tprintf(hmqtt, (char*)"SDCardWrite: %s create error %i",
			            name[i], (int)res);

			if (!done)
			{
				if (++retries > SD_RETRIES)
				{
					SDCardErrorHandler();
					return false;
				}
				else
					SDCardRestart();
			}
		}
	}

	return true;
}

/**
 * @param dname data file name without extension, 27 characters max
 * @param data bytes array to store read data
 * @param sz size of array
 * @return read bytes count or 0 if error
 */
static size_t SDCardReadDataFile(const char* dname, void* data, size_t sz)
{
	FRESULT res;
	FIL file;
	UINT bytesread = 0;
	UINT readsize = (UINT)sz;
	char name[2][16];
	size_t i;

	sprintf(name[0], "%s.TXT", dname);
	sprintf(name[1], "%s.SHD", dname);

	// Try to read main file, if fails - try shadow file
	for (i = 0; i < 2; i++)
	{
		res = f_open(&file, name[i], FA_OPEN_EXISTING | FA_READ);
		if(res == FR_OK)
		{
			bytesread = 0;
			res = f_read(&file, data, readsize, &bytesread);
			f_close(&file);
		}

		if (res == FR_OK && bytesread > 0) break;
	}

	return (size_t)bytesread;
}
#endif
//		This is the PROVISIONAL way of populating the Context :
//		Declare an static array of pointers to CVariable objects
//			and then, assign it to the static member
int	 CreateContext()
{
#ifdef USE_SD_CARD
	/* Start the SD Driver */
	SDCardStart();

	/*##-3- Create a FAT file system (format) on the logical drive ###########*/
    /* WARNING: Formatting the uSD card will delete all content on the device */
//	static uint8_t buffer[_MAX_SS]; /* a work buffer for the f_mkfs() */
//	res = f_mkfs((TCHAR const*)SDPath, FM_ANY, 0, buffer, sizeof(buffer));
//	if(res != FR_OK) ContextErrorHandler();
#endif

    static CVariable *ALLVARS[DEVICE_PARAMS_CNT+1];
    ALLVARS[DEVICE_PARAMS_CNT] = NULL;

#ifdef USE_SD_CARD
    // Read device parameters from NVM
    size_t bytesread = SDCardReadDataFile("DEV", devParamsBuf, DEV_PARAMS_BUF_SIZE);
#else
    size_t bytesread = 0;
#endif

	for (size_t i = 0; i < DEVICE_PARAMS_CNT; i++)
	{
	    const char* value = UnpackDevVariable(devParams[i].name, devParamsBuf, bytesread);
	    if (value == NULL) value = devParams[i].defval;

	    switch (devParams[i].type)
	    {
	    case TYPE_INT:
	        ALLVARS[i] = new CVariable (devParams[i].name, devParams[i].name,
	                new CVarint(atoi(value)));
	        break;
	    case TYPE_DEC:
	        ALLVARS[i] = new CVariable (devParams[i].name, devParams[i].name,
	                new CVardec(atof(value)));
	        break;
	    default:
	        ALLVARS[i] = new CVariable (devParams[i].name, devParams[i].name,
	                new CVarstring(value));
            break;
	    }
	}

	CVariable::Context = ALLVARS;
	CSubspace::RestoreAll();
	return 1;
}

int RestoreSharedValues(void)
{
    SharedMemoryData sharedData;
    ReadSharedMemory(&sharedData);
    char buf[8];

    SetVariable((char*)"BTLSER", sharedData.variables.FW_SERVER_URI);
    sprintf(buf, "%i", (int)(sharedData.variables.PORT));
    SetVariable((char*)"BTLPRT", buf);
    SetVariable((char*)"BTLPRO", sharedData.variables.PROTOCOL);
    SetVariable((char*)"BTLPAT", sharedData.variables.PATH);
    SetVariable((char*)"BTLNAM", sharedData.variables.FW_NAME);
    SetVariable((char*)"BTLUSR", sharedData.variables.USER);
    SetVariable((char*)"BTLPAS", sharedData.variables.PASSWORD);
    sprintf(buf, "%i", (int)(sharedData.variables.UPDFW_COUNT));
    SetVariable((char*)"BTLCNT", buf);
    SetVariable((char*)"BTLAUS", (char*)"-1");
    SetVariable((char*)"BTLAPS", (char*)"-1");
    SetVariable((char*)"BTLVER", sharedData.variables.FW_VERSION);

    return 1;
}

// This is a PROVISIONAL way of populating the input and output Framesets
//	IMPORTANT:	the OutAPI array has to be populated BEFORE,
//				because input frames can eventually reference output frames as replay
//
//	The string in the lines have some PARTS, separated by % characte
//
//		FRAMEID % TEMPLATE % DESCRIPTION % TOPICID % REPLY
//			FRAMEID:	Optional.Is only necesary for Request/Replay pairs
//			TEMPLATE	The template of the message (bolerplate text and variables)
//			DESCRIPTION	Optional, as aditional information
//			TOPICID		(not used by now)
//			REPLAY		Optional. Only for INPUT messages which have to trigger a response.
int	 ReadMetadata(char *dominioIn, char *dominioOut){
	const char	*linesOut[] = {
		"ERROR%999;$MSSG;Unknown message%Error%%",
		"BTANS%607;$ID;$BTLRES;$BTLVER;$BTLSER;$BTLPRT;$BTLPRO;$BTLPAT;$BTLUSR;$BTLPAS;$BTLCNT;%Bootloader ans%%",

		"CONLA%L3_TF_CONF_LISTEN_STATE_R;$ID;$TLRES;%listen state%%",
		"CONSA%L3_TF_CONF_SEND_STATUS_STATE_R;$ID;$TSRES;%status state%%",
		"CONAA%L3_TF_CONF_SAMPLING_STATE_R;$ID;$TARES;%alert state%%",

		"L3TFR%L3_TF_STATUS_R;$ID;$TFVOL;$TFVTS;%return photodiode voltage%%",

		NULL
	};

	const char	*linesIn[] = {

		"CCOMM%5;$MURI;$MPORT;$MUSER;$MPSWD;%configure communications%%",
		"BTCFG%207;$BTLSER;$BTLPRT;$BTLPRO;$BTLPAT;$BTLNAM;$BTLUSR;$BTLPAS;$BTLCNT;%boot config%xx%BTANS%",
		"BTACT%208;$BTLAUS;$BTLAPS;%boot act%xx%BTANS%",
		// 314 is done through main function
		"CHAPN%175;$ANAME;%configure APN%%",

		"CONLS%L3_TF_CONF_LISTEN_STATE;$LISEN;$TLSTH;$TLDUR;$TLRTN;%config listen%xx%CONLA%",
		"CONSN%L3_TF_CONF_SEND_STATUS_STATE;$TSPER;$TSLDR;$TSRTN;%config status%xx%CONSA%",
		"CONAL%L3_TF_CONF_SAMPLING_STATE;$TAPER;$TATHL;$TATHH;%config alert%xx%CONAA%",

		"L3TFD%L3_TF_STATUS_OD;%request diode voltage%xx%L3TFR%",

		NULL
	};

	CFrame::OutAPI = ReadFrames2(linesOut);
	CFrame::InAPI = ReadFrames2(linesIn);

	return 1;
}

#ifdef USE_AP_CONTEXT

int GenericReadElement(unsigned int n, unsigned char *to)
{
#ifdef DISP_TIMING
	uint32_t startTick = HAL_GetTick();
#endif

	char name[16];
	sprintf(name, "%u", n);

	size_t bytesread = SDCardReadDataFile(name, to, CELLSIZE);
	if (bytesread < 1) return 0;

	// Append '\0' at the end of read data
	to[bytesread] = '\0';

#ifdef DISP_TIMING
	uint32_t endTick = HAL_GetTick();
	TraceThreadSafeFromContext(hmqtt, (char*)"ok GenericReadElement %lu msec",
			(endTick - startTick));
#endif

	return 1;
}

int GenericWriteElement(unsigned int n, unsigned char *to)
{
#ifdef DISP_TIMING
	uint32_t startTick = HAL_GetTick();
#endif

	char name[16];
	sprintf(name, "%u", n);

	if (!SDCardWriteDataFile(name, to, strlen((char*)to))) return 0;

#ifdef DISP_TIMING
	uint32_t endTick = HAL_GetTick();
	TraceThreadSafeFromContext(hmqtt, (char*)"GenericWriteElement %lu msec",
			(endTick - startTick));
#endif

	return 1;
}
#else

int GenericReadElement(unsigned int n, unsigned char *to)
{
	return 0;
}

int GenericWriteElement(unsigned int n, unsigned char *to)
{
	return 0;
}

#endif

int SaveDevParamsToNVM(void)
{
#ifdef DISP_TIMING
	uint32_t startTick = HAL_GetTick();
#endif

	size_t i;
	size_t bias = 0;

	// Prepare memory buffer
	for (i = 0; i < DEVICE_PARAMS_CNT; i++)
	{
		bias += snprintf(&devParamsBuf[bias], (DEV_PARAMS_BUF_SIZE - bias),
		        "%s=%s\n",
		        devParams[i].name, GetVariable((char*)devParams[i].name));
	}
	bias++; // Include trailing '\0' in buffer length

#ifdef USE_SD_CARD
	// Save device context to memory
	if (!SDCardWriteDataFile("DEV", devParamsBuf, bias)) return 0;
#endif

#ifdef DISP_TIMING
	uint32_t endTick = HAL_GetTick();
	TraceThreadSafeFromContext(hmqtt, (char*)"SaveDevParamsToNVM %lu msec",
			(endTick - startTick));
#endif

	return 1;
}

static char* FindPrevChar(char* bufPtr, char* startPtr, char charToFind)
{
    for (; startPtr >= bufPtr; startPtr--)
        if (*startPtr == charToFind)
            return startPtr;
    return NULL;
}

char* UnpackDevVariable(const char* name, char* buf, size_t len)
{
    char* ptr = buf;
    char* end = buf + len - 1;

    static char valueBuf[1024];

    while (ptr != NULL && ptr <= end)
    {
        // Now in &buf[i] name=val\n......

        // Retrieve name
        char* curNameStart = ptr;                       // Start is the start of our chunk
        if (curNameStart == NULL || *curNameStart == '\0')
            return NULL;                                // Check name start

        char* curNameAftrEnd = strstr(curNameStart, "=");// End is before the '=' sign, so search for '='
        if (curNameAftrEnd == NULL)
            return NULL;                                // If '=' is not found - error

        char* tmp = strstr(curNameStart, "\n");         // Check forlines without paramerets
        if (tmp != NULL && tmp < curNameAftrEnd)
        {
            ptr = tmp + 1;
            continue;
        }

        size_t curNameLen = curNameAftrEnd - curNameStart;
        if (curNameLen == 0)                            // Check for malformed record and skip it
        {
            ptr = strstr(curNameAftrEnd, "\n");
            if (ptr != NULL) ptr++;
            continue;
        }

        // Retrieve value
        char* curValStart = curNameAftrEnd + 1;         // Start is after '='
        if (curValStart == NULL || *curValStart == '\0')
            return NULL;                                // Check value start

        char* nextNameEnd = strstr(curValStart, "=");   // Next name end before the '=' sign, so search for '='

        char* curValAftrEnd;            // Search for value end, there should be '\n' symbol
        if (nextNameEnd == NULL)    // if no next name - start at the end of buf
            curValAftrEnd = FindPrevChar(curValStart, end, '\n');
        else                        // start search at next name end
            curValAftrEnd = FindPrevChar(curValStart, nextNameEnd, '\n');
        if (curValAftrEnd == NULL)
            return NULL;                                // Check value end
        size_t curValLen = curValAftrEnd - curValStart;

        // Place next pointer after val end
        ptr = curValAftrEnd + 1;

        // Check name if it is one we need
        if (strlen(name) == curNameLen &&
            memcmp(name, curNameStart, curNameLen) == 0)
        {
            // Copy value and return it
            if (curValLen > 0)
                memcpy(valueBuf, curValStart, curValLen);
            valueBuf[curValLen] = '\0';
            return valueBuf;
        }
    }

    return NULL;
}
