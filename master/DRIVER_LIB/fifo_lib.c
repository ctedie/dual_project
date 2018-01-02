/**********************************************************************
 * COPYRIGHT (C) Alstom 2017 - AC4 project
 * All rights reserved.
 **********************************************************************/

/******************** FILE HEADER *************************************/
/** \addtogroup FIFO
 *  \{ */

/**
 **********************************************************************
 * \file fifo.c
 * \brief  FIFO implementation
 *
 **********************************************************************/

/** \} */

/**********************************************************************
 * INCLUDES
 **********************************************************************/
#include <stddef.h>
#include "fifo.h"
#include "it_utils.h"

/** \addtogroup SCI
 *  \{ */

/**********************************************************************
 * MACROS DEFINITION
 **********************************************************************/


/**********************************************************************
 *  TYPE DEFINITION
 **********************************************************************/



/**********************************************************************
 *  LOCAL VARIABLES OF MODULE
 **********************************************************************/


/**********************************************************************
 *  LOCAL FUNCTIONS
 **********************************************************************/

static inline void appendElem(struct FIFO_t *pFifo, struct FIFO_Elem_t *pElem)
{
    if (pFifo->pHead == NULL)
    {
        // FIFO is empty
        pFifo->pHead = pElem;
    }
    else
    {
        // FIFO not empty
        pFifo->pTail->pNext = pElem;
    }
    // Elem is insert at the tail of the FIFO
    pElem->pNext = NULL;

    // Update tail
    pFifo->pTail = pElem;
}

/**********************************************************************
 *  API FUNCTIONS
 **********************************************************************/

/**
 **********************************************************************
 * \brief  Initialize a FIFO structure
 *
 * \param [in] pFifo   pointer on the FIFO structure
 *
 * \note This function is not thread/IT safe
 **********************************************************************/
void FIFO_Init(struct FIFO_t *pFifo)
{
    if (pFifo != NULL)
    {
        pFifo->pHead = NULL;
        pFifo->pTail = NULL;
    }
}

/**
 **********************************************************************
 * \brief  Initialize a FIFO structure and fill it with the elements of the array
 *         in parameter
 *
 * \param [in] pFifo    pointer on the FIFO structure
 * \param [in] array    pointer on the array (view as a buffer of int8u)
 * \param [in] elemSize size of an element of this array
 * \param [in] arrayDim number of elements of this array
 *
 * \note The array shall be an array of structures which first elem shall be a FIFO_Elem_t
 *
 * \note This function is not thread/IT safe
 **********************************************************************/
void FIFO_InitFromArray(struct FIFO_t *pFifo, void *array, uint32_t elemSize, uint32_t arrayDim)
{
    uint32_t i;

    if (pFifo == NULL || array == NULL)
    {
        return;
    }

    // initialize the FIFO
    FIFO_Init(pFifo);

    // fill it with the elements of the array
    for (i = 0u; i < arrayDim; ++i)
    {
        appendElem(pFifo, array);

        // next elem in the array
        array = ((uint8_t*)array) + elemSize;
    }
}

/**
 **********************************************************************
 * \brief  Insert an element at the tail of the FIFO (the function is thread/IT safe)
 *
 * \param [in] pFifo    pointer on the FIFO structure
 * \param [in] pElem    pointer on the element to insert in the FIFO
 *
 * \note The function return if:
 *       - The fifo passed in parameter is not valid (NULL pointer or invalid structure)
 *       - The fifo elem passed in parameter is not valid (NULL pointer or invalid structure)
 **********************************************************************/
void FIFO_Insert(struct FIFO_t *pFifo, struct FIFO_Elem_t *pElem)
{
    ItCpuCtx_t itCtx;

    if (pFifo == NULL || pElem == NULL)
    {
        return;
    }

    // Entry in a critical section
    itCtx = IT_SaveDisable();

    appendElem(pFifo, pElem);

    // Exit the critical section
    IT_Restore(itCtx);
}

/**
 **********************************************************************
 * \brief  Retrieve and remove an element at the head of the FIFO (the function is thread/IT safe)
 *
 * \param [in] pFifo    pointer on the FIFO structure
 *
 * \note The function raises an error if:
 *       - The fifo passed in parameter is not valid (NULL pointer or invalid structure)
 *
 * \return pointer on the element removed from the head of the FIFO
 *         (NULL if the FIFO was empty)
 **********************************************************************/
struct FIFO_Elem_t *FIFO_Get(struct FIFO_t *pFifo)
{
    struct FIFO_Elem_t *pRetElem = NULL;
    ItCpuCtx_t itCtx;

    if (pFifo == NULL)
    {
        return NULL;
    }

    // Entry in a critical section
    itCtx = IT_SaveDisable();

    if (pFifo->pHead != NULL)
    {
        // FIFO not empty
        // the element at the head of the FIFO is returned
        pRetElem = pFifo->pHead;
        // new head of the FIFO is the next element
        pFifo->pHead = pRetElem->pNext;
        if (pFifo->pHead == NULL)
        {
            // the FIFO is now empty
            // update tail
            pFifo->pTail = NULL;
        }
        // indicate that the elem is no more in a FIFO (for the check in the insert)
        pRetElem->pNext = NULL;
    }
    else
    {
        // FIFO is empty
        // a NULL pointer is returned
    }

    // Exit the critical section
    IT_Restore(itCtx);

    return pRetElem;
}

/**
 **********************************************************************
 * \brief  Append a Fifo pFifo to a destination Fifo pFifo_dest
 *
 * \param [in] pFifo_dest    pointer on the destination FIFO structure
 * \param [in] pFifo    pointer on the FIFO structure
 *
 * \note  pFifo should not be used again after
 **********************************************************************/
void FIFO_AppendFifo(struct FIFO_t *pFifoDest, struct FIFO_t *pFifo)
{
    ItCpuCtx_t itCtx;

    if (pFifo == NULL)
    {
        return;
    }

    // Entry in a critical section
    itCtx = IT_SaveDisable();

    if (pFifoDest->pHead == NULL)
    {
        // FIFO is empty
        *pFifoDest = *pFifo;
    }
    else
    {
        // FIFO not empty
        pFifoDest->pTail->pNext = pFifo->pHead;

        // Update tail
        pFifoDest->pTail = pFifo->pTail;
    }

    // Exit the critical section
    IT_Restore(itCtx);
}

/** \} */
/* EOF */
