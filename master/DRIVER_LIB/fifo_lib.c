////////////////////////////////////////////////////////////////////////////////
/// \addtogroup FIFO
/// @{
///
/// \par Justification of design choices
///     (If required)
///     - Any important choice shall be justify. For instance:
///         - Reason why using or not a mutex? Which variables are protected.
///         - Use of specific driver. 
///         - Use of specific algorithm.
///         - ...
///
/// \file 
/// \brief Implementation file of module FIFO
///
///
////////////////////////////////////////////////////////////////////////////////

/// \cond IGNORE_FOLLOWING
// For the management of the declaration AND definition of exported variables
#define FIFO_M
/// \endcond

////////////////////////////////////////////////////////////////////////////////
//                                                                      Includes
////////////////////////////////////////////////////////////////////////////////
// First standard includes (compilation tool / Adetel Group standard / Librairies)

// Then project includes
#include "fifo_lib.h"

////////////////////////////////////////////////////////////////////////////////
//                                                      Internal Defines & Types
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//                                                            Internal Variables
////////////////////////////////////////////////////////////////////////////////
// Note : static is MANDATORY


////////////////////////////////////////////////////////////////////////////////
//                                                            Internal Functions
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//                                                            Exported Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// \par Implementation details
///  Details of implementation
///  Specific par can be added for certified project (Coverage, Pseudo-code,...)
////////////////////////////////////////////////////////////////////////////////
void FIFO_Init(struct FIFO_t *pFifo)
{
    CHECK_ASSERTION(NULL != pFifo, return);

    OS_CRITSECT_CREATE(pFifo->hCritSect);
    pFifo->pHead = NULL;
    pFifo->pTail = NULL;
    pFifo->pSelf = pFifo;
}

////////////////////////////////////////////////////////////////////////////////
/// \par Implementation details
/// Remind: the array passed un parameter shall be an array of structures and the first member
///         of theses structures shall be a FIFO_Elem_t
///         the array is passed as an array of uint8_t to view it as a buffer, elemSize and arrayDim
///         parameters allow to calculate the address of each element of the array and to iterate over
///         the array
///  Details of implementation
///  Specific par can be added for certified project (Coverage, Pseudo-code,...)
////////////////////////////////////////////////////////////////////////////////
void FIFO_InitFromArray(struct FIFO_t *pFifo, uint8_t array[], uint32_t elemSize, uint32_t arrayDim)
{
    uint32_t i;
    uint8_t *pArrayElem;
    struct FIFO_Elem_t *pElem;

    CHECK_ASSERTION(NULL != pFifo, return);

    // initialize the FIFO
    FIFO_Init(pFifo);

    // fill it with the elements of the array
    pArrayElem = &array[0];
    for (i = 0LU ; i < arrayDim ; ++i)
    {
        // VIOLATION REPORTED: MISRA-C:2012 R.11.1,R.11.2,R.11.3,R.11.5,R.11.7: Casting operation to a pointer
        //                     MISRA-C:2012 R.11.1,R.11.2,R.11.3,R.11.7: Casting operation on a pointer
        //                     MISRA-C:2012 R.11.2,R.11.3: Cast to an unrelated type
        // JUSTIFICATION: Violation of MISRA-C:2012 R.11.3 is acceptable as the address of each elem in the
        //                array is calculated with the elemSize passed in parameter
        //LDRA_INSPECTED 94 S
        //LDRA_INSPECTED 95 S
        //LDRA_INSPECTED 554 S
        pElem = (struct FIFO_Elem_t *)pArrayElem;
        // indicate that the elem is no more in a FIFO (for the check in the insert)
        pElem->pNext = NULL;
        FIFO_Insert(pFifo, pElem);
        // next elem in the array
        pArrayElem += elemSize;
    }
}

////////////////////////////////////////////////////////////////////////////////
/// \par Implementation details
///  Details of implementation
///  Specific par can be added for certified project (Coverage, Pseudo-code,...)
////////////////////////////////////////////////////////////////////////////////
void FIFO_Insert(struct FIFO_t *pFifo, struct FIFO_Elem_t *pElem)
{
    DECLARE_CRITSECT_LOCVAR;

    // Entry in a critical section
    OS_CRITSECT_ENTER(pFifo->hCritSect);

    CHECK_ASSERTION(NULL != pFifo, return);
    CHECK_ASSERTION(pFifo == pFifo->pSelf, return);
    // check that the element is not in a FIFO
    CHECK_ASSERTION(NULL == pElem->pNext, return);

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

    // Exit the critical section
    OS_CRITSECT_LEAVE(pFifo->hCritSect);
}

////////////////////////////////////////////////////////////////////////////////
/// \par Implementation details
///  Details of implementation
///  Specific par can be added for certified project (Coverage, Pseudo-code,...)
////////////////////////////////////////////////////////////////////////////////
struct FIFO_Elem_t *FIFO_Get(struct FIFO_t *pFifo)
{
    struct FIFO_Elem_t *pRetElem;
    DECLARE_CRITSECT_LOCVAR;

    // Entry in a critical section
    OS_CRITSECT_ENTER(pFifo->hCritSect);

    CHECK_ASSERTION(NULL != pFifo, return NULL);
    CHECK_ASSERTION(pFifo == pFifo->pSelf, return NULL);

    if (pFifo->pHead == NULL)
    {
        // FIFO is empty
        // a NULL pointer is returned
        pRetElem = NULL;
    }
    else
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

    // Exit the critical section
    OS_CRITSECT_LEAVE(pFifo->hCritSect);

    // VIOLATION REPORTED: MISRA-C:2012 R.18.6: Pointer assignment to wider scope
    // JUSTIFICATION: No violation of MISRA-C:2012 R.18.6: pRetElem do not point on an object with automatic storage
    //LDRA_INSPECTED 71 S
    return pRetElem;
}

////////////////////////////////////////////////////////////////////////////////
/// \par Implementation details
///  Details of implementation
///  Specific par can be added for certified project (Coverage, Pseudo-code,...)
////////////////////////////////////////////////////////////////////////////////
bool FIFO_IsEmpty(const struct FIFO_t *pFifo)
{
    CHECK_ASSERTION(NULL != pFifo, return false);
    CHECK_ASSERTION(pFifo == pFifo->pSelf, return false);

    return NULL == pFifo->pHead ? true : false;
}
////////////////////////////////////////////////////////////////////////////////
/// \par Implementation details
///  Details of implementation
///  Specific par can be added for certified project (Coverage, Pseudo-code,...)
////////////////////////////////////////////////////////////////////////////////
struct FIFO_Elem_t *FIFO_ReadHead(const struct FIFO_t *pFifo)
{
    CHECK_ASSERTION(NULL != pFifo, return NULL);
    CHECK_ASSERTION(pFifo == pFifo->pSelf, return NULL);

    return pFifo->pHead;
}
////////////////////////////////////////////////////////////////////////////////
/// \par Implementation details
///  Details of implementation
///  Specific par can be added for certified project (Coverage, Pseudo-code,...)
////////////////////////////////////////////////////////////////////////////////
struct FIFO_Elem_t *FIFO_ReadNext(const struct FIFO_Elem_t *pElem)
{
    CHECK_ASSERTION(NULL != pElem, return NULL);

    return pElem->pNext;
}

///
/// @}
///
