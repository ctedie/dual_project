/**********************************************************************
 * COPYRIGHT (C) Alstom 2017 - AC4 project
 * All rights reserved.
 **********************************************************************/

/* ****************** FILE HEADER *************************************/
/** \addtogroup UTILS
 *  \{
 * \addtogroup IT_UTILS
 *  \{ */

/**
 **********************************************************************
 * \file it_utils.h
 * \brief IT_UTILS interface
 *
 **********************************************************************/

/** \} */
/** \} */

#ifndef IT_UTILS_H
#define IT_UTILS_H

/**********************************************************************/
/*  INCLUDE                                                           */
/**********************************************************************/
#include <stdint.h>

/** \addtogroup IT_UTILS
 *  \{ */
/**********************************************************************/
/*  MACROS DEFINITION                                                 */
/**********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

// critical section
#ifdef __ARMCC_VERSION

typedef int ItCpuCtx_t;

static inline ItCpuCtx_t IT_SaveDisable(void)
{
    return __disable_irq();
}

static inline void IT_Restore(ItCpuCtx_t was_masked)
{
    if (!was_masked)
    {
        __enable_irq();
    }
}

#else

typedef uint32_t ItCpuCtx_t;

static inline ItCpuCtx_t IT_SaveDisable(void)
{
    uint32_t priMask;

    __asm__ volatile (             \
        "MRS   %0, PRIMASK \n\t"   \
        "CPSID I           \n\t"   \
        : "=r" (priMask) : : "memory");

    return priMask;
}

static inline void IT_Restore(ItCpuCtx_t priMask)
{
    __asm__ volatile (                       \
        "MSR   PRIMASK,%0  \n\t"             \
        : : "r" (priMask) : "memory");
}

#endif

/**********************************************************************/
/*  TYPE DEFINITION                                                   */
/**********************************************************************/


/**********************************************************************/
/*  FUNCTIONS PROTOTYPE                                               */
/**********************************************************************/


#ifdef __cplusplus
}
#endif

/** \} */
#endif /* IT_UTILS_H */
/* EOF */
