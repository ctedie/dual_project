////////////////////////////////////////////////////////////////////////////////
/// \addtogroup FIFO
/// @{
///
/// \brief Management of FIFO
///
/// \par Limitation
///     (None if no limitation)
///     - Any limitation of the public services of module.
///
/// \par Safety Aspects
///     (If required)
///     - The error handling strategy, 
///         - Raise a fault (use of raise_error). Shall be the strategy used in most cases.
///         - Propagate the error outside of the module. You shall tell why use this strategy.
///
/// \file 
/// \brief Header file of module FIFO
///
///
////////////////////////////////////////////////////////////////////////////////

#ifndef FIFO_H
/// \cond IGNORE_FOLLOWING
//  safe include guard : FIFO_H shall be unique within the project
#define FIFO_H
/// \endcond
#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////
//                                                                      Includes
////////////////////////////////////////////////////////////////////////////////
// Required include files for correct compilation of this header
#include <os_wrapper.h>

// Then project includes

/// \cond IGNORE_FOLLOWING
// For the management of the declaration AND definition of exported variables
#undef PUBLIC
#ifdef FIFO_M
#define PUBLIC
#else
#define PUBLIC extern
#endif
/// \endcond
// MANDATORY : no #include directive after this previous #ifdef FIFO_M directive

// Note : PREMOD is the module prefix (it shall be short and in Upper-case)

////////////////////////////////////////////////////////////////////////////////
//                                                      Exported Defines & Types
////////////////////////////////////////////////////////////////////////////////

/// structure used for the chaining in FIFO
/// one member of this type shall be in front of each elem which shall be
/// inserted in a FIFO
struct FIFO_Elem_t {
    struct FIFO_Elem_t *pNext;  ///< pointer on next elem in FIFO
};

///< FIFO structure
struct FIFO_t {
    struct FIFO_Elem_t *pHead;  ///< pointer on the head elem in the FIFO
    struct FIFO_Elem_t *pTail;  ///< pointer on the tail elem in the FIFO
    DECLARE_CRITSECT(/*no qualifier */, hCritSect)
    struct FIFO_t *pSelf;       ///< self pointer for internal check
};

////////////////////////////////////////////////////////////////////////////////
//                                                            Exported Variables
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//                                                            Exported Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// \brief  Initialize a FIFO structure
///
/// \param [in] pFifo   pointer on the FIFO structure
///
/// \note This function is not thread/IT safe
/// \note The function raises an error (and do not return) in the following cases:
///       - The fifo passed in parameter is not valid (NULL pointer)
////////////////////////////////////////////////////////////////////////////////
void FIFO_Init(struct FIFO_t *pFifo);

////////////////////////////////////////////////////////////////////////////////
/// \brief  Initialize a FIFO structure and fill it with the elements of the array
///         in parameter
///
/// \param [in] pFifo    pointer on the FIFO structure
/// \param [in] array    pointer on the array (view as a buffer of int8u)
/// \param [in] elemSize size of an element of this array
/// \param [in] arrayDim number of elements of this array
///
/// \note The array shall be an array of structures which first elem shall be a FIFO_Elem_t
///       Ex: struct My_Struct_t {
///               struct FIFO_Elem_t      fifoElem;   ///< shall be the first member of the structure as this structure
///                                                   ///< may be put in a FIFO
///               ... other members ...
///           };
///           struct My_Struct_t m_arrOfStructs[10];  /// array of structures
///           struct FIFO_t m_myFifo;
///           ...
///           // Initialize the FIFO and fill it with the elements of m_arrOfStructs
///           FIFO_InitFromArray(&_myFifo, (uint8_t *)&m_arrOfStructs[0], sizeof(m_arrOfStructs[0]), ARRAY_DIM(m_arrOfStructs));
///       
/// \note This function is not thread/IT safe
/// \note The function raises an error (and do not return) in the following cases:
///       - The fifo passed in parameter is not valid (NULL pointer or invalid structure)
////////////////////////////////////////////////////////////////////////////////
void FIFO_InitFromArray(struct FIFO_t *pFifo, uint8_t array[], uint32_t elemSize, uint32_t arrayDim);

////////////////////////////////////////////////////////////////////////////////
/// \brief  Insert an element at the tail of the FIFO (the function is thread/IT safe)
///
/// \param [in] pFifo    pointer on the FIFO structure
/// \param [in] pElem    pointer on the element to insert in the FIFO
///
/// \note The function raises an error (and do not return) in the following cases:
///       - The fifo passed in parameter is not valid (NULL pointer or invalid structure)
///       - The fifo elem passed in parameter is not valid (NULL pointer or invalid structure)
////////////////////////////////////////////////////////////////////////////////
void FIFO_Insert(struct FIFO_t *pFifo, struct FIFO_Elem_t *pElem);

////////////////////////////////////////////////////////////////////////////////
/// \brief  Retrieve and remove an element at the head of the FIFO (the function is thread/IT safe)
///
/// \param [in] pFifo    pointer on the FIFO structure
///
/// \note The function raises an error (and do not return) in the following cases:
///       - The fifo passed in parameter is not valid (NULL pointer or invalid structure)
///
/// \return pointer on the element removed from the head of the FIFO
///         (NULL if the FIFO was empty)
////////////////////////////////////////////////////////////////////////////////
struct FIFO_Elem_t *FIFO_Get(struct FIFO_t *pFifo);

////////////////////////////////////////////////////////////////////////////////
/// \brief  return the state of the FIFO
///
/// \param [in] pFifo    pointer on the FIFO structure
///
/// \note The function raises an error (and do not return) in the following cases:
///       - The fifo passed in parameter is not valid (NULL pointer or invalid structure)
///
/// \retval true  FIFO is empty
/// \retval false FIFO is not empty
////////////////////////////////////////////////////////////////////////////////
bool FIFO_IsEmpty(const struct FIFO_t *pFifo);

////////////////////////////////////////////////////////////////////////////////
/// \brief  Get a pointer on the head element of the FIFO without removing it
///
/// \param [in] pFifo    pointer on the FIFO structure
///
/// \note The function raises an error (and do not return) in the following cases:
///       - The fifo passed in parameter is not valid (NULL pointer or invalid structure)
///
/// \return pointer on the head element of the FIFO
///         (NULL if the FIFO was empty)
////////////////////////////////////////////////////////////////////////////////
struct FIFO_Elem_t *FIFO_ReadHead(const struct FIFO_t *pFifo);

////////////////////////////////////////////////////////////////////////////////
/// \brief  Get pointer on next element
///
/// \param [in] pElem    pointer on a FIFO element
///
/// \note The function raises an error (and do not return) in the following cases:
///       - The pointer passed in parameter is NULL
///
/// \return pointer on next element
///         (NULL if the element was at the tail of a FIFO)
////////////////////////////////////////////////////////////////////////////////
struct FIFO_Elem_t *FIFO_ReadNext(const struct FIFO_Elem_t *pElem);

#ifdef __cplusplus
}
#endif
#endif // FIFO_H

///
/// @}
///
