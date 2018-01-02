/////////////////////////////////////////////////////////////////////////////////
/// \addtogroup fs_module
/// \{
/// \author Dev
///
///	\brief 
///
///
/// \file
/// \brief Fichier source du module fs_module
///
///
///
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include <stdbool.h>

#include "system.h"
#include "fs_module.h"

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

#include "utils/spi_flash.h"

/////////////////////////////////////////////////////////////////////////////////
// Private typedef
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Private define
/////////////////////////////////////////////////////////////////////////////////
#define FSSTACKSIZE 4096
/////////////////////////////////////////////////////////////////////////////////
// Private macro
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Private variables
/////////////////////////////////////////////////////////////////////////////////
//Task_Struct FSTaskStruct;
//Char FSTaskStack[FSSTACKSIZE];

FATFS fs;
static FIL filePointer;

/////////////////////////////////////////////////////////////////////////////////
// Exported variables
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Private function prototypes
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Functions
/////////////////////////////////////////////////////////////////////////////////
static void FileSystem_TaskBody(UArg arg0, UArg arg1);
static void PrepareFlashMemory(bool needErase);

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
void FileSystem_Init(void)
{

	//TODO Init GPIO and SPI for external flash

	SPIFlashInit(SSI0_BASE, CPU_CLOCK, 10000000);

	disk_register(
	    0,
		Flash_diskInit,
	    Flash_diskStatus,
	    Flash_diskRead,
	    Flash_diskwrite,
	    Flash_diskIoctl
	);

    PrepareFlashMemory(false);

//	Task_Params taskParams;
//
//    Task_Params_init(&taskParams);
//
//    taskParams.stackSize = FSSTACKSIZE;
//    taskParams.stack = &FSTaskStack;
//    Task_construct(&FSTaskStruct, (Task_FuncPtr)FileSystem_TaskBody, &taskParams, NULL);

}

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
FRESULT FileSystem_readFile(char* fileName, T_PRINT_CALLBACK cbPrint)
{

	FRESULT res = FR_OK;
	char path[64];
	uint8_t i;
	TCHAR buff[64];
	UINT bytesRead = 0;
	UINT bytesToRead = 0;
	uint8_t fnameSize;

	fnameSize = strlen(fileName);
	if(fnameSize > 64)
	{
		return FR_INVALID_NAME;
	}

	memset(path, 0, 64);
	path[0]='/';
	for (i = 0; (i < fnameSize)&&(fileName[i]!=0); ++i)
	{
		path[i+1] = fileName[i];
	}

	res = f_open(&filePointer, path, FA_READ);
	if(res!=FR_OK)
	{
		return res;
	}


	while(bytesToRead < filePointer.fsize)
	{
		res = f_read(&filePointer, buff, sizeof(buff), &bytesRead);
		if(res != FR_OK)
		{
			f_close(&filePointer);
			return res;
		}
		cbPrint("%s",buff);
		bytesToRead += bytesRead;
	}

	f_close(&filePointer);
	cbPrint("\n\neverything is ok\nThe file is : %s\n", fileName);

	return res;
}


/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
static void FileSystem_TaskBody(UArg arg0, UArg arg1)
{

	while(1)
	{
		TaskSleep(1000);
	}
}

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
static void PrepareFlashMemory(bool needErase)
{
	uint8_t init = 1;

	Bool filecreate = 0;

	FRESULT res;

	char writeBuf[]={"Vive la "COMMAND FOREGROUND_BLACK COMMAND BACKGROUND_BLUE"FR"COMMAND BACKGROUND_WHITE"AN"COMMAND BACKGROUND_RED"CE""!\0"};
	UINT writtenchar;
	char readBuf[sizeof(writeBuf)];

	int i;


	if(init == 1)
	{

		res = f_mount(&fs, "0:", 1);
		if(res != FR_OK)
		{
			if(res == FR_NO_FILESYSTEM)
			{
				FlashMemEraseChip();
				filecreate = 1;

				res = f_mkfs("0:", 0, 1); //Old FatFs
				if(res != FR_OK)
				{
					while(1)
					{

					}
				}

				res = f_mount(&fs, "0:", 1);
				if(res != FR_OK)
				{
					while(1)
					{

					}
				}


			}
			else
			{
				while(1)
				{

				}
			}
		}

		init = 1;
		if(filecreate == 1)
		{
			res = f_open(&filePointer, "/newfile", FA_WRITE | FA_CREATE_ALWAYS);
			if(res != FR_OK)
			{
				while(1)
				{

				}
			}

			res = f_write(&filePointer, writeBuf, sizeof(writeBuf), &writtenchar);
			if(res != FR_OK)
			{
				while(1)
				{

				}
			}
			writtenchar = 0;
			f_close(&filePointer);

			res = f_open(&filePointer, "/newfile", FA_READ);
			if(res != FR_OK)
			{
				while(1)
				{

				}
			}

			res = f_read(&filePointer, readBuf, filePointer.fsize, &writtenchar);
			if(res != FR_OK)
			{
				while(1)
				{

				}
			}

			f_close(&filePointer);
		}

	}

}

///
/// \}
///
