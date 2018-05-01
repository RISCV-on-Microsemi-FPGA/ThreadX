/**************************************************************************/ 
/*                                                                        */ 
/*            Copyright (c) 1996-2015 by Express Logic Inc.               */ 
/*                                                                        */ 
/*  This software is copyrighted by and is the sole property of Express   */ 
/*  Logic, Inc.  All rights, title, ownership, or other interests         */ 
/*  in the software remain the property of Express Logic, Inc.  This      */ 
/*  software may only be used in accordance with the corresponding        */ 
/*  license agreement.  Any unauthorized use, duplication, transmission,  */ 
/*  distribution, or disclosure of this software is expressly forbidden.  */ 
/*                                                                        */
/*  This Copyright notice may not be removed or modified without prior    */ 
/*  written consent of Express Logic, Inc.                                */ 
/*                                                                        */ 
/*  Express Logic, Inc. reserves the right to modify this software        */ 
/*  without notice.                                                       */ 
/*                                                                        */ 
/*  Express Logic, Inc.                     info@expresslogic.com         */
/*  11423 West Bernardo Court               www.expresslogic.com          */
/*  San Diego, CA  92127                                                  */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */ 
/** ThreadX Component                                                     */ 
/**                                                                       */
/**   Thread                                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"
#include "tx_thread.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_thread_info_get                                 PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves information from the specified thread.      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    thread_ptr                        Pointer to thread control block   */ 
/*    name                              Destination for the thread name   */ 
/*    state                             Destination for thread state      */ 
/*    run_count                         Destination for thread run count  */ 
/*    priority                          Destination for thread priority   */ 
/*    preemption_threshold              Destination for thread preemption-*/ 
/*                                        threshold                       */ 
/*    time_slice                        Destination for thread time-slice */ 
/*    next_thread                       Destination for next created      */ 
/*                                        thread                          */ 
/*    next_suspended_thread             Destination for next suspended    */ 
/*                                        thread                          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Completion status                 */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-12-2005     William E. Lamie         Initial Version 5.0           */ 
/*  04-02-2007     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.1    */ 
/*  12-12-2008     William E. Lamie         Modified comment(s), and      */ 
/*                                            added filter option to      */ 
/*                                            trace insert, resulting     */ 
/*                                            in version 5.2              */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), merged   */ 
/*                                            event logging support,      */ 
/*                                            added logic to return the   */ 
/*                                            user-specified priority and */ 
/*                                            preemption-threshold, and   */ 
/*                                            added logic to explicitly   */ 
/*                                            check for valid pointer,    */ 
/*                                            resulting in version 5.4    */ 
/*  07-15-2011     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.5    */ 
/*  11-01-2012     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.6    */ 
/*  05-01-2015     William E. Lamie         Modified comment(s), and      */ 
/*                                            modified code for MISRA     */ 
/*                                            compliance, resulting in    */ 
/*                                            version 5.7                 */ 
/*                                                                        */ 
/**************************************************************************/ 
UINT  _tx_thread_info_get(TX_THREAD *thread_ptr, CHAR **name, UINT *state, ULONG *run_count, 
                UINT *priority, UINT *preemption_threshold, ULONG *time_slice, 
                TX_THREAD **next_thread, TX_THREAD **next_suspended_thread)
{

TX_INTERRUPT_SAVE_AREA


    /* Disable interrupts.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_INFO_GET, thread_ptr, thread_ptr -> tx_thread_state, 0, 0, TX_TRACE_THREAD_EVENTS)

    /* Log this kernel call.  */
    TX_EL_THREAD_INFO_GET_INSERT

    /* Retrieve all the pertinent information and return it in the supplied
       destinations.  */

    /* Retrieve the name of the thread.  */
    if (name != TX_NULL)
    {
    
        *name =  thread_ptr -> tx_thread_name;
    }
    
    /* Pickup the thread's current state.  */
    if (state != TX_NULL)
    {
    
        *state =  thread_ptr -> tx_thread_state;
    }
    
    /* Pickup the number of times the thread has been scheduled.  */
    if (run_count != TX_NULL)
    {
    
        *run_count =  thread_ptr -> tx_thread_run_count;
    }
    
    /* Pickup the thread's priority.  */
    if (priority != TX_NULL)
    {
    
        *priority =  thread_ptr -> tx_thread_user_priority;
    }
    
    /* Pickup the thread's preemption-threshold.  */
    if (preemption_threshold != TX_NULL)
    {
    
        *preemption_threshold =  thread_ptr -> tx_thread_user_preempt_threshold;
    }
    
    /* Pickup the thread's current time-slice.  */
    if (time_slice != TX_NULL)
    {
    
        *time_slice =  thread_ptr -> tx_thread_time_slice;
    }
    
    /* Pickup the next created thread.  */
    if (next_thread != TX_NULL)
    {
    
        *next_thread =  thread_ptr -> tx_thread_created_next;
    }
    
    /* Pickup the next thread suspended.  */
    if (next_suspended_thread != TX_NULL)
    {
    
        *next_suspended_thread =  thread_ptr -> tx_thread_suspended_next;
    }
    
    /* Restore interrupts.  */
    TX_RESTORE

    /* Return completion status.  */
    return(TX_SUCCESS);
}

