/////////////////////////////////////////////////////////////////////////////////
/// \addtogroup flash_fs_lib
/// \{
/// \author Dev
///
///	\brief 
///
///
/// \file
/// \brief Fichier source du module flash_fs_lib
///
///
///
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////
#include <ti/mw/fatfs/diskio.h>
#include <ti/mw/fatfs/ff.h>
#include <ti/mw/fatfs/integer.h>


#include <stdint.h>
#include "flash_lib.h"
#include "flash_fs_lib.h"
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
void InitFlashFS(void)
{

}


/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
DSTATUS Flash_diskInit(BYTE drive)
{
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
DSTATUS Flash_diskStatus(BYTE drive)
{
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
DRESULT Flash_diskRead(BYTE drive, BYTE *buf, DWORD sector, UINT num)
{
	UINT secSize;

	secSize = FlashMemGetSectorSize();

	FlashMemRead((uint32_t)(secSize*sector), (uint8_t*)buf, (secSize*num));

	return RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
DRESULT Flash_diskwrite(BYTE drive, const BYTE *buf, DWORD sector, UINT num)
{
	UINT i;
	UINT secSize;

	secSize = FlashMemGetSectorSize();

	FlashMemSectorErase(sector*secSize);

	for (i = 0; i < num; ++i)
	{
		FlashMemWrite((uint32_t)(sector*secSize), (uint8_t*)buf, (num*secSize));
	}

	return RES_OK;
}

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
DRESULT Flash_diskIoctl(BYTE drive, BYTE cmd, void * buf)
{
	uint32_t* val;
	uint32_t tmp;
	val = buf;

	switch(cmd)
	{
		case CTRL_SYNC:
			break;
		case GET_SECTOR_COUNT:
			*val = FlashMemGetSectorCount();
			break;
		case GET_SECTOR_SIZE:
			*val = FlashMemGetSectorSize();
			break;
		case GET_BLOCK_SIZE: //In Unit of sector
			tmp = FlashMemGetSectorSize();
			*val = (FlashMemGetBlockSize() / tmp);
			break;
		case CTRL_TRIM:
			break;
		default:
			break;
	}

	return RES_OK;

}



///
/// \}
///
