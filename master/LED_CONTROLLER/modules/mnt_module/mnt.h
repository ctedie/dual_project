/////////////////////////////////////////////////////////////////////////////////
/// \addtogroup mnt
/// \{
///
///
/// \file
/// \brief Fichier en-tete du module mnt
///
///
///
/////////////////////////////////////////////////////////////////////////////////

// Define to prevent recursive inclusion
#ifndef __MNT_H_
/// \cond IGNORE_FOLLOWING
#define __MNT_H_
/// \endcond

#ifdef __cplusplus
extern "C"
{
#endif
/////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Exported types
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Exported constants
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Exported macro
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Exported functions
void MNT_init(void);
extern void UARTStdioIntHandler(void);

#ifdef __cplusplus
}
#endif

#endif //__MNT_H_

///
/// \}
///
