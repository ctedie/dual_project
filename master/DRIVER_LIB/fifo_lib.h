/**********************************************************************
 * COPYRIGHT (C) Alstom 2017 - AC4 project
 * All rights reserved.
 **********************************************************************/

/* ****************** FILE HEADER *************************************/
/** \addtogroup BASIC
 *  \{
 * \addtogroup FIFO
 *  \{ */

/**
 **********************************************************************
 * \file fifo.h
 * \brief FIFO interface
 *
 **********************************************************************/

/** \} */
/** \} */

#ifndef FIFO_H
#define FIFO_H

/**********************************************************************/
/*  INCLUDE                                                           */
/**********************************************************************/
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** \addtogroup FIFO
 *  \{ */
/**********************************************************************/
/*  MACROS DEFINITION                                                 */
/**********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


/**
 * structure used for the chaining in FIFO
 * one member of this type shall be in front of each elem which shall be inserted in a FIFO
 */
struct FIFO_Elem_t {
    struct FIFO_Elem_t *pNext;  /**< pointer on next elem in FIFO */
};

/** FIFO structure */
struct FIFO_t {
    struct FIFO_Elem_t *pHead;  /**< pointer on the head elem in the FIFO */
    struct FIFO_Elem_t *pTail;  /**< pointer on the tail elem in the FIFO */
};

/**********************************************************************/
/*  TYPE DEFINITION                                                   */
/**********************************************************************/

/**********************************************************************/
/*  FUNCTIONS PROTOTYPE                                               */
/**********************************************************************/

void FIFO_Init(struct FIFO_t *pFifo);
void FIFO_InitFromArray(struct FIFO_t *pFifo, void *array, uint32_t elemSize, uint32_t arrayDim);
void FIFO_Insert(struct FIFO_t *pFifo, struct FIFO_Elem_t *pElem);
void FIFO_AppendFifo(struct FIFO_t *pFifoDest, struct FIFO_t *pFifo);
struct FIFO_Elem_t *FIFO_Get(struct FIFO_t *pFifo);

/**********************************************************************/
/*  FUNCTIONS Static inline                                           */
/**********************************************************************/

/**
 **********************************************************************
 * \brief  return the state of the FIFO
 *
 * \param [in] pFifo    pointer on the FIFO structure
 *
 * \retval true  FIFO is empty
 * \retval false FIFO is not empty
 **********************************************************************/
static inline bool FIFO_IsEmpty(const struct FIFO_t *pFifo)
{
    return ((pFifo == NULL) || (pFifo->pHead == NULL));
}

#ifdef __cplusplus
}
#endif

/** \} */
#endif /* FIFO_H */
/* EOF */
