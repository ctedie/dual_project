/////////////////////////////////////////////////////////////////////////////////
/// \addtogroup system
/// \{
///
///
/// \file
/// \brief Fichier en-tete du module system
///
///
///
/////////////////////////////////////////////////////////////////////////////////

// Define to prevent recursive inclusion
#ifndef __SYSTEM_H_
/// \cond IGNORE_FOLLOWING
#define __SYSTEM_H_
/// \endcond

#ifdef __cplusplus
extern "C"
{
#endif
/////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>

#include <ti/sysbios/family/arm/m3/Hwi.h>
//#include <ti/sysbios/hal/Hwi.h>
#include <ti/sysbios/hal/Timer.h>
#include <ti/sysbios/interfaces/ITimer.h>

/* NDK BSD support */
//#include <ti/ndk/inc/usertype.h>
//#include <ti/ndk/inc/socketndk.h>
//#include <ti/ndk/inc/socket.h>

#include <sys/socket.h>


#include <inc/hw_ints.h>
#include <inc/hw_memmap.h>
#include <inc/hw_types.h>
#include <inc/hw_gpio.h>

#include <driverlib/flash.h>
#include <driverlib/gpio.h>
#include <driverlib/i2c.h>
#include <driverlib/pin_map.h>
#include <driverlib/sysctl.h>
#include <driverlib/uart.h>

#include <utils/uartstdio.h>
#include <utils/cmdline.h>


#include <ti/mw/fatfs/ff.h>
#include <ti/mw/fatfs/diskio.h>

#include "flash_lib.h"
#include "flash_fs_lib.h"
#include "txt_lib.h"

#include "fs_module.h"
#include "clock_module.h"
#include "common.h"

/////////////////////////////////////////////////////////////////////////////////
// Exported types
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Exported constants
/////////////////////////////////////////////////////////////////////////////////
#define CPU_CLOCK	120000000U

extern uint16_t ctedi_test;

/////////////////////////////////////////////////////////////////////////////////
// Exported macro
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Exported functions
/////////////////////////////////////////////////////////////////////////////////
void Set_Ton(unsigned int val);


#ifdef __cplusplus
}
#endif

#endif //__SYSTEM_H_

///
/// \}
///
