////////////////////////////////////////////////////////////////////////////////
/// \addtogroup DPC
/// @{
///
/// \brief Public interface of DPC (Deferred Procedure Call)
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
/// \brief Header file of module DPC
///
///
////////////////////////////////////////////////////////////////////////////////

#ifndef DPC_H
/// \cond IGNORE_FOLLOWING
//  safe include guard : DPC_H shall be unique within the project
#define DPC_H
/// \endcond
#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////
//                                                                      Includes
////////////////////////////////////////////////////////////////////////////////
// Required include files for correct compilation of this header
// First standard includes (compilation tool / Adetel Group standard / Librairies)
// none

// Then project includes

/// \cond IGNORE_FOLLOWING
// For the management of the declaration AND definition of exported variables
#undef PUBLIC
#ifdef DPC_M
#define PUBLIC
#else
#define PUBLIC extern
#endif
/// \endcond
// MANDATORY : no #include directive after this previous #ifdef DPC_M directive

////////////////////////////////////////////////////////////////////////////////
//                                                      Exported Defines & Types
////////////////////////////////////////////////////////////////////////////////
#define NB_MAX_DPC	4

#if (DPC_WITH_HIGH_PRIO == TRUE)
#define NB_MAX_DPC_HIGH_PRIO	4
#endif


/// Type of a DPC
typedef void (*DPC_Fct_t)(void *pParam);

////////////////////////////////////////////////////////////////////////////////
//                                                            Exported Variables
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//                                                            Exported Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// \brief  Initialize the internal data of the module
///
////////////////////////////////////////////////////////////////////////////////
void DPC_Init(void);

////////////////////////////////////////////////////////////////////////////////
/// \brief  Push the DPC passed in parameter in the fifo of DPCs to be called
///
/// \param [in] pDpc   pointer on the DPC (ie the function which shall be called)
/// \param [in] pParam user argument to be passed to the DPC when it is called
///
/// \retval true  the DPC was pushed in the fifo
/// \retval false the DPC was not pushed in the fifo
////////////////////////////////////////////////////////////////////////////////
bool DPC_Push(DPC_Fct_t pDpc, void *pParam);

////////////////////////////////////////////////////////////////////////////////
/// \brief  Push the DPC passed in parameter in the high priority fifo of 
///         DPCs to be called
///
/// \param [in] pDpc   pointer on the DPC (ie the function which shall be called)
/// \param [in] pParam user argument to be passed to the DPC when it is called
///
/// \retval true  the DPC was pushed in the high priority fifo
/// \retval false the DPC was not pushed in the high priority fifo
////////////////////////////////////////////////////////////////////////////////
#if (DPC_WITH_HIGH_PRIO == TRUE)
bool DPC_PushHighPrio(DPC_Fct_t pDpc, void *pParam);
#endif

////////////////////////////////////////////////////////////////////////////////
/// \brief  Pop a DPC from the high priority fifo and execute it if  the fifo 
///         is not empty (if high priority is used)
///         Otherwise, pop a DPC from the fifo and execute it if  the fifo is not empty
///
/// \note   This function may be called from a low priority task or from
///         a background process 
////////////////////////////////////////////////////////////////////////////////
void DPC_Pop(void);

#ifdef __cplusplus
}
#endif
#endif // DPC_H

///
/// @}
///
