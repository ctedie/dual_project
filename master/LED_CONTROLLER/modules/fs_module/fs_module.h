/////////////////////////////////////////////////////////////////////////////////
/// \addtogroup fs_module
/// \{
///
///
/// \file
/// \brief Fichier en-tete du module fs_module
///
///
///
/////////////////////////////////////////////////////////////////////////////////

// Define to prevent recursive inclusion
#ifndef __FS_MODULE_H_
/// \cond IGNORE_FOLLOWING
#define __FS_MODULE_H_
/// \endcond

#ifdef __cplusplus
extern "C"
{
#endif
/////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////
#include <ti/mw/fatfs/ff.h>
#include <ti/mw/fatfs/diskio.h>

/////////////////////////////////////////////////////////////////////////////////
// Exported types
/////////////////////////////////////////////////////////////////////////////////
typedef void (*T_PRINT_CALLBACK)(const char* str, ...);
/////////////////////////////////////////////////////////////////////////////////
// Exported constants
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Exported macro
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Exported functions
/////////////////////////////////////////////////////////////////////////////////
void FileSystem_Init(void);
FRESULT FileSystem_readFile(char* fileName, T_PRINT_CALLBACK cbPrint);



#ifdef __cplusplus
}
#endif

#endif //__FS_MODULE_H_

///
/// \}
///
