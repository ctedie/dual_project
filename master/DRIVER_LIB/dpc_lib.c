////////////////////////////////////////////////////////////////////////////////
/// \addtogroup DPC
/// @{
///
/// \par Justification of design choices
///     (If required)
///     - Global variables are protected from concurrent access by critical
///       section
///
/// \file 
/// \brief Implementation file of module DPC
///
///
////////////////////////////////////////////////////////////////////////////////

/// \cond IGNORE_FOLLOWING
// For the management of the declaration AND definition of exported variables
#define DPC_M
/// \endcond

////////////////////////////////////////////////////////////////////////////////
//                                                                      Includes
////////////////////////////////////////////////////////////////////////////////

// Then project includes
#include "dpc_lib.h"

////////////////////////////////////////////////////////////////////////////////
//                                                      Internal Defines & Types
////////////////////////////////////////////////////////////////////////////////

/// descriptor of a DPC
struct DpcElem_t {
    DPC_Fct_t    pDpc;           ///< DPC to call
    void        *pParam;         ///< parameter to pass to the DPC
};


////////////////////////////////////////////////////////////////////////////////
//                                                            Internal Variables
////////////////////////////////////////////////////////////////////////////////
// Note : static is MANDATORY

/// Fifo of DPCs.
/// This array is used as a FIFO with a write index and a read index, see below
static struct DpcElem_t m_arrDpcElem[NB_MAX_DPC];

/// write index in the fifo
static uint32_t m_idxPush;

/// read index in the fifo
static uint32_t m_idxPop;

/// counter of DPCs in the fifo
static uint32_t m_cntDpcBusy;

#if (DPC_WITH_HIGH_PRIO == TRUE)
/// Fifo of high priority DPCs.
/// This array is used as a FIFO with a write index and a read index, see below
static struct DpcElem_t m_arrDpcElemHighPrio[NB_MAX_DPC_HIGH_PRIO];

/// write index in the high priority fifo
static uint32_t m_idxPushHighPrio;

/// read index in the high priority fifo
static uint32_t m_idxPopHighPrio;

/// counter of DPCs in the high priority fifo
static uint32_t m_cntDpcBusyHighPrio;
#endif

/// Critical section to manage concurrent access to global variables
DECLARE_CRITSECT(static, m_hCritSect)

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
void DPC_Init(void)
{
    m_idxPush    = 0LU;
    m_idxPop     = 0LU;
    m_cntDpcBusy = 0LU;
#if (DPC_WITH_HIGH_PRIO == TRUE)
    m_idxPushHighPrio    = 0LU;
    m_idxPopHighPrio     = 0LU;
    m_cntDpcBusyHighPrio = 0LU;
#endif
    // creation of critical section
    OS_CRITSECT_CREATE(m_hCritSect);
}

///////////////////////////////////////////////////////////////////////////////
/// \par Implementation details
///  Details of implementation
///  Specific par can be added for certified project (Coverage, Pseudo-code,...)
////////////////////////////////////////////////////////////////////////////////
bool DPC_Push(DPC_Fct_t pDpc, void *pParam)
{
    DECLARE_CRITSECT_LOCVAR;

    if (m_cntDpcBusy >= ARRAY_DIM(m_arrDpcElem))
    {
        // FIFO full
        return false;
    }
    // FIFO not full => push elem
    // Enter critical section to protect access to m_cntDpcBusy and m_idxPush
    OS_CRITSECT_ENTER(m_hCritSect);
    m_arrDpcElem[m_idxPush].pDpc   = pDpc;
    m_arrDpcElem[m_idxPush].pParam = pParam;
    ++m_cntDpcBusy;
    if (m_idxPush == (ARRAY_DIM(m_arrDpcElem)-1LU))
    {
        m_idxPush = 0LU;
    }
    else
    {
        ++m_idxPush;
    }
    // Exit critical section
    OS_CRITSECT_LEAVE(m_hCritSect);
    return true;
}
#if (DPC_WITH_HIGH_PRIO == TRUE)
///////////////////////////////////////////////////////////////////////////////
/// \par Implementation details
///  Details of implementation
///  Specific par can be added for certified project (Coverage, Pseudo-code,...)
////////////////////////////////////////////////////////////////////////////////
bool DPC_PushHighPrio(DPC_Fct_t pDpc, void *pParam)
{
    DECLARE_CRITSECT_LOCVAR;

    if (m_cntDpcBusyHighPrio >= ARRAY_DIM(m_arrDpcElemHighPrio))
    {
        // FIFO full
        return false;
    }
    // FIFO not full => push elem
    // Enter critical section to protect access to m_cntDpcBusyHighPrio and m_idxPushHighPrio
    OS_CRITSECT_ENTER(m_hCritSect);
    m_arrDpcElemHighPrio[m_idxPushHighPrio].pDpc   = pDpc;
    m_arrDpcElemHighPrio[m_idxPushHighPrio].pParam = pParam;
    ++m_cntDpcBusyHighPrio;
    if (m_idxPushHighPrio == (ARRAY_DIM(m_arrDpcElemHighPrio)-1LU))
    {
        m_idxPushHighPrio = 0LU;
    }
    else
    {
        ++m_idxPushHighPrio;
    }
    // Exit critical section
    OS_CRITSECT_LEAVE(m_hCritSect);
    return true;
}
#endif
///////////////////////////////////////////////////////////////////////////////
/// \par Implementation details
///  Details of implementation
///  Specific par can be added for certified project (Coverage, Pseudo-code,...)
////////////////////////////////////////////////////////////////////////////////
void DPC_Pop(void)
{    
    DECLARE_CRITSECT_LOCVAR;
    
#if (DPC_WITH_HIGH_PRIO == TRUE)
    // check high priority FIFO
    if (m_cntDpcBusyHighPrio != 0LU)
    {
        // pop DPC
        m_arrDpcElemHighPrio[m_idxPopHighPrio].pDpc(m_arrDpcElemHighPrio[m_idxPopHighPrio].pParam);
        if (m_idxPopHighPrio == (ARRAY_DIM(m_arrDpcElemHighPrio)-1LU))
        {
            m_idxPopHighPrio = 0LU;
        }
        else
        {
            ++m_idxPopHighPrio;
        }
        // Enter critical section to protect access to m_cntDpcBusyHighPrio
        OS_CRITSECT_ENTER(m_hCritSect);
        --m_cntDpcBusyHighPrio;
        // Exit critical section
        OS_CRITSECT_LEAVE(m_hCritSect);
    }
    // else because only one DPC is managed during a call to DPC_Pop
    else
#endif
    if (m_cntDpcBusy != 0LU)
    {
        // pop DPC
        m_arrDpcElem[m_idxPop].pDpc(m_arrDpcElem[m_idxPop].pParam);
        if (m_idxPop == (ARRAY_DIM(m_arrDpcElem)-1LU))
        {
            m_idxPop = 0LU;
        }
        else
        {
            ++m_idxPop;
        }
        // Enter critical section to protect access to m_cntDpcBusyHighPrio
        OS_CRITSECT_ENTER(m_hCritSect);
        --m_cntDpcBusy;
        // Exit critical section
        OS_CRITSECT_LEAVE(m_hCritSect);
    }
    else
    {
        // Nothing to do
    }
}

///
/// @}
///
