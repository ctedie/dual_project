/////////////////////////////////////////////////////////////////////////////////
/// \addtogroup mnt_commands
/// \{
/// \author Dev
///
///	\brief 
///
///
/// \file
/// \brief Fichier source du module mnt_commands
///
///
///
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////
#include "system.h"
#include <stdlib.h>
//#include <utils/cmdline.h>

#include "mnt_commands.h"

/////////////////////////////////////////////////////////////////////////////////
// Private typedef
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Private define
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Private macro
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Private variables
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Exported variables
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Private function prototypes
/////////////////////////////////////////////////////////////////////////////////
int cmd_help(int argc, char *argv[]);
int cmd_ls(int argc, char *argv[]);
int cmd_infos(int argc, char *argv[]);
int cmd_SetLed(int argc, char *argv[]);
int cmd_reboot(int argc, char *argv[]);
int cmd_cat(int argc, char *argv[]);
int cmd_time(int argc, char *argv[]);
int cmd_fcreate(int argc, char *argv[]);

struct tTimeArg
{
	char* arg;
	unsigned char index;
};

struct tTimeArg TimeArgList[] =
{ 	{"-Y", 0},
	{"-M", 1},
	{"-D", 2},
	{"-h", 3},
	{"-m", 4},
	{"-s", 5},
	{NULL}
};

tCmdLineEntry g_psCmdTable[] =
{
		{ "help"	,cmd_help		, "Give some help"},
		{ "?"		,cmd_help		, "help aliasis"},
		{ "ls"		,cmd_ls 		, "File list"},
		{ "infos"	,cmd_infos 		, "Board informations"},
		{ "set"		,cmd_SetLed 	, "Set led luminosity from 0 to 19"},
		{ "reset"	,cmd_reboot 	, "Reset the board"},
		{ "cat"		,cmd_cat 		, "Print the file content"},
		{ "time"	,cmd_time 		, "Set or get the current clock"},
		{ "fc"		,cmd_fcreate	, "Create new file"},
		{NULL, NULL, NULL}
};


/////////////////////////////////////////////////////////////////////////////////
// Functions
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
int cmd_help(int argc, char *argv[])
{
	uint8_t i = 0;

	UARTprintf("\nAvailable commands\n");
	UARTprintf("-----------------------------\n");

	while(g_psCmdTable[i].pfnCmd)
	{
		UARTprintf("%s \t\t : %s\n",g_psCmdTable[i].pcCmd, g_psCmdTable[i].pcHelp );
		i++;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
int cmd_ls(int argc, char *argv[])
{
    FRESULT res;
    DIR dir;
    FATFS *pFatFs;
    uint32_t ui32TotalSize;

//    UINT i;
    static FILINFO fno;
    TCHAR path[64];


    res = f_opendir(&dir, "/");                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
//            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
//                UARTprintf("%s/%s/\n", path, fno.fname);
//            } else {                                       /* It is a file. */
//            	UARTprintf("%s/%s\n", path, fno.fname);
//            }
            //
            // Print the entry information on a single line with formatting to show
            // the attributes, date, time, size, and name.
            //
            UARTprintf("%c%c%c%c%c %u/%02u/%02u %02u:%02u %9u  %s\n",
                        (fno.fattrib & AM_DIR) ? 'D' : '-',
                        (fno.fattrib & AM_RDO) ? 'R' : '-',
                        (fno.fattrib & AM_HID) ? 'H' : '-',
                        (fno.fattrib & AM_SYS) ? 'S' : '-',
                        (fno.fattrib & AM_ARC) ? 'A' : '-',
                        (fno.fdate >> 9) + 1980,
                        (fno.fdate >> 5) & 15,
                         fno.fdate & 31,
                        (fno.ftime >> 11),
                        (fno.ftime >> 5) & 63,
                         fno.fsize,
                         fno.fname);


        }

    	UARTFlushTx(false);

        //
        // Get the free space.
        //
        res = f_getfree("/", (DWORD*)&ui32TotalSize, &pFatFs);

        //
        // Check for error and return if there is a problem.
        //
        if(res != FR_OK)
        {
            return(res);
        }

        //
        // Display the amount of free space that was calculated.
        //
        UARTprintf("%u bytes free\n", ui32TotalSize  * pFatFs->csize *4096);
//        UARTprintf(", %10uK bytes free\n", ui32TotalSize * pFatFs->csize / 2);

        f_closedir(&dir);
    }

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
int cmd_infos(int argc, char *argv[])
{


    UARTprintf("Here will be board informations available soon.\n");

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
int cmd_SetLed(int argc, char *argv[])
{
	unsigned long val;

	if(argc != 2)
	{
		return CMDLINE_TOO_MANY_ARGS;
	}
	val = strtoul(argv[1],0, 10);

//	FIXME Set_Ton((unsigned int)val);


	return 0;
}

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
int cmd_reboot(int argc, char *argv[])
{

	UARTprintf("\nBoard reset...\n\n\n");
	UARTFlushTx(false);
	SysCtlReset();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
int cmd_cat(int argc, char *argv[])
{
	FRESULT ret = FR_OK;
	if(argc != 2)
	{
		return CMDLINE_INVALID_ARG;
	}

    ret = FileSystem_readFile(argv[1], UARTprintf);
    if(ret > 0)
    {
    	UARTprintf("ERROR : %d\n", ret);
    }
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
int cmd_time(int argc, char *argv[])
{
	int ret;
	uint8_t i;
	uint8_t j = 0;
	time_t temps = 0;

	tDateTime date =
		{	.day = 1,
			.month = 1,
			.year = 1970,
			.hour = 0,
			.minute = 0,
			.second = 0,
			.dayOfWeek = LUNDI
		};
	struct tm *pdateTime;

		temps = CLOCK_get();
//		time(&temps);
		pdateTime = localtime( &temps );
	if(argc == 1)
	{
//		CLOCK_getDateTime(&date);
//		UARTprintf("%d/%d/%d %d:%d:%d\n", date.day, date.month, date.year, date.hour, date.minute, date.second);
//
		UARTprintf("The current local time is: %s", ctime (&temps));



		return 0;
	}


	for (i = 1; i < argc; i+=2)
	{
		j = 0;

		while((TimeArgList[j].arg) != NULL)
		{
			if(strcmp(argv[i], TimeArgList[j].arg) == 0)
			{
				break;
			}
			else
			{
				j++;
			}
		}

		switch (j)
		{
			case 0:
				UARTprintf("Setting Year %s\n",argv[i+1]);
				date.year = atoi(argv[i+1]);
				pdateTime->tm_year = atoi(argv[i+1])-1900;
				break;
			case 1:
				UARTprintf("Setting Month %s\n",argv[i+1]);
				date.month = atoi(argv[i+1]);
				pdateTime->tm_mon = atoi(argv[i+1])-1;
				break;
			case 2:
				UARTprintf("Setting Day %s\n",argv[i+1]);
				date.day = atoi(argv[i+1]);
				pdateTime->tm_mday = atoi(argv[i+1]);
				break;
			case 3:
				UARTprintf("Setting Hour %s\n",argv[i+1]);
				date.hour = atoi(argv[i+1]);
				pdateTime->tm_hour = atoi(argv[i+1]);
				break;
			case 4:
				UARTprintf("Setting Minute %s\n",argv[i+1]);
				pdateTime->tm_min = atoi(argv[i+1]);
				break;
			case 5:
				UARTprintf("Setting Second %s\n",argv[i+1]);
				pdateTime->tm_sec = atoi(argv[i+1]);
				break;
			default:
				return CMDLINE_INVALID_ARG;
				break;
		}
	}
	temps = mktime(pdateTime);

	CLOCK_set(temps);

//	UARTprintf("List of args :\n");
//	for (i = 1; i < argc; ++i)
//	{
//		UARTprintf("\tArg %d : %s\n", i, argv[i]);
//	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
int cmd_fcreate(int argc, char *argv[])
{
	FIL filePointer;
	char *filename;
	char* fileContent;
	FRESULT res;
	int fileContentLength;
	UINT writtenchar;

	if(argc < 3)
	{
		return CMDLINE_TOO_FEW_ARGS;
	}

	filename = argv[1];
	fileContent = argv[2];
	fileContentLength = strlen(fileContent);


		res = f_open(&filePointer, filename, FA_WRITE | FA_CREATE_ALWAYS);
		if(res != FR_OK)
		{
			while(1)
			{

			}
		}

		res = f_write(&filePointer, fileContent, fileContentLength, &writtenchar);
		if(res != FR_OK)
		{
			while(1)
			{

			}
		}

		f_close(&filePointer);

//		res = f_open(&filePointer, "/newfile", FA_READ);
//		if(res != FR_OK)
//		{
//			while(1)
//			{
//
//			}
//		}
//
//		res = f_read(&filePointer, readBuf, 13, &writtenchar);
//		if(res != FR_OK)
//		{
//			while(1)
//			{
//
//			}
//		}
//
//		f_close(&filePointer);




	return 0;
}


///
/// \}
///
