/////////////////////////////////////////////////////////////////////////////////
/// \addtogroup clock_lib
/// \{
///
///
/// \file
/// \brief Fichier en-tete du module clock_lib
///
///
///
/////////////////////////////////////////////////////////////////////////////////

// Define to prevent recursive inclusion
#ifndef __CLOCK_LIB_H_
/// \cond IGNORE_FOLLOWING
#define __CLOCK_LIB_H_
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

/// Availiable clock for UC
typedef enum
{
	SYS_CLOCK_10MHz,//!< SYS_CLOCK_10MHz
	SYS_CLOCK_20MHz,//!< SYS_CLOCK_20MHz
	SYS_CLOCK_40MHz,//!< SYS_CLOCK_40MHz
	SYS_CLOCK_80MHz,//!< SYS_CLOCK_80MHz
	// Nb of availiable clock values
	NB_SYS_CLOCK    //!< NB_SYS_CLOCK
}T_SYS_CLOCK_FREQ;

typedef void (*T_SYSTEM_INTERRUPT_CALLBACK)(void *arg);


typedef struct
{
	T_SYSTEM_INTERRUPT_CALLBACK pfnIntHandler;
	void *pArg;
}T_CLOCK_INT_OBJECT;
/////////////////////////////////////////////////////////////////////////////////
// Exported constants
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Exported macro
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Exported functions
/////////////////////////////////////////////////////////////////////////////////
int SysClockInit(T_SYS_CLOCK_FREQ freq);
uint64_t SystemGetClockValue(void);
uint64_t GetTickCount(void);
uint16_t SystemTickInit(uint64_t u64tickPeriod, T_CLOCK_INT_OBJECT *clock);

#ifdef __cplusplus
}
#endif

#endif //__CLOCK_LIB_H_

///
/// \}
///
