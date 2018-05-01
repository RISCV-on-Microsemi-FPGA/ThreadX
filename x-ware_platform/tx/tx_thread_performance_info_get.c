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
#include "tx_thread.h"
#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO
#include "tx_trace.h"
#endif


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_thread_performance_info_get                     PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves performance information from the specified  */ 
/*    thread.                                                             */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    thread_ptr                        Pointer to thread control block   */ 
/*    resumptions                       Destination for number of times   */ 
/*                                        thread was resumed              */ 
/*    suspensions                       Destination for number of times   */ 
/*                                        thread was suspended            */ 
/*    solicited_preemptions             Destination for number of times   */ 
/*                                        thread called another service   */ 
/*                                        that resulted in preemption     */ 
/*    interrupt_preemptions             Destination for number of times   */ 
/*                                        thread was preempted by another */ 
/*                                        thread made ready in Interrupt  */ 
/*                                        Service Routine (ISR)           */ 
/*    priority_inversions               Destination for number of times   */ 
/*                                        a priority inversion was        */ 
/*                                        detected for this thread        */ 
/*    time_slices                       Destination for number of times   */ 
/*                                        thread was time-sliced          */ 
/*    relinquishes                      Destination for number of thread  */ 
/*                                        relinquishes                    */ 
/*    timeouts                          Destination for number of timeouts*/ 
/*                                        for thread                      */ 
/*    wait_aborts                       Destination for number of wait    */ 
/*                                        aborts for thread               */ 
/*    last_preempted_by                 Destination for pointer of the    */ 
/*                                        thread that last preempted this */ 
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
/*  12-12-2009     William E. Lamie         Modified comment(s), removed  */ 
/*                                            compound conditionals, added*/ 
/*                                            logic to explicitly check   */ 
/*                                            for valid pointer, merged   */ 
/*                                            event logging support, and  */ 
/*                                            added code to ensure that   */ 
/*                                            input parameters are        */ 
/*                                            accessed in non-enabled     */ 
/*                                            case (default),             */ 
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
UINT  _tx_thread_performance_info_get(TX_THREAD *thread_ptr, ULONG *resumptions, ULONG *suspensions, 
                ULONG *solicited_preemptions, ULONG *interrupt_preemptions, ULONG *priority_inversions,
                ULONG *time_slices, ULONG *relinquishes, ULONG *timeouts, ULONG *wait_aborts, TX_THREAD **last_preempted_by)
{

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

TX_INTERRUPT_SAVE_AREA
UINT                    status;


    /* Determine if this is a legal request.  */
    if (thread_ptr == TX_NULL)
    {
        
        /* Thread pointer is illegal, return error.  */
        status =  TX_PTR_ERROR;
    }
    
    /* Determine if the thread ID is invalid.  */
    else if (thread_ptr -> tx_thread_id != TX_THREAD_ID)
    {
        
        /* Thread pointer is illegal, return error.  */
        status =  TX_PTR_ERROR;
    }
    else
    {

        /* Disable interrupts.  */
        TX_DISABLE

        /* If trace is enabled, insert this event into the trace buffer.  */
        TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_PERFORMANCE_INFO_GET, thread_ptr, thread_ptr -> tx_thread_state, 0, 0, TX_TRACE_THREAD_EVENTS)

        /* Log this kernel call.  */
        TX_EL_THREAD_PERFORMANCE_INFO_GET_INSERT

        /* Retrieve all the pertinent information and return it in the supplied
           destinations.  */

        /* Retrieve number of resumptions for this thread.  */
        if (resumptions != TX_NULL)
        {
    
            *resumptions =  thread_ptr -> tx_thread_performance_resume_count;
        }
    
        /* Retrieve number of suspensions for this thread.  */
        if (suspensions != TX_NULL)
        {
    
            *suspensions =  thread_ptr -> tx_thread_performance_suspend_count;
        }
    
        /* Retrieve number of solicited preemptions for this thread.  */
        if (solicited_preemptions != TX_NULL)
        {
    
            *solicited_preemptions =  thread_ptr -> tx_thread_performance_solicited_preemption_count;
        }
    
        /* Retrieve number of interrupt preemptions for this thread.  */
        if (interrupt_preemptions != TX_NULL)
        {
    
            *interrupt_preemptions =  thread_ptr -> tx_thread_performance_interrupt_preemption_count;
        }
    
        /* Retrieve number of priority inversions for this thread.  */
        if (priority_inversions != TX_NULL)
        {
    
            *priority_inversions =  thread_ptr -> tx_thread_performance_priority_inversion_count;
        }
    
        /* Retrieve number of time-slices for this thread.  */
        if (time_slices != TX_NULL)
        {
    
            *time_slices =  thread_ptr -> tx_thread_performance_time_slice_count;
        }
    
        /* Retrieve number of relinquishes for this thread.  */
        if (relinquishes != TX_NULL)
        {
    
            *relinquishes =  thread_ptr -> tx_thread_performance_relinquish_count;
        }
    
        /* Retrieve number of timeouts for this thread.  */
        if (timeouts != TX_NULL)
        {
    
            *timeouts =  thread_ptr -> tx_thread_performance_timeout_count;
        }
    
        /* Retrieve number of wait aborts for this thread.  */
        if (wait_aborts != TX_NULL)
        {
    
            *wait_aborts =  thread_ptr -> tx_thread_performance_wait_abort_count;
        }
    
        /* Retrieve the pointer of the last thread that preempted this thread.  */
        if (last_preempted_by != TX_NULL)
        {
    
            *last_preempted_by =  thread_ptr -> tx_thread_performance_last_preempting_thread;
        }
    
        /* Restore interrupts.  */
        TX_RESTORE

        /* Return completion status.  */
        status =  TX_SUCCESS;
    }
#else
UINT                    status;


    /* Access input arguments just for the sake of lint, MISRA, etc.  */
    if (thread_ptr != TX_NULL) 
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (resumptions != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (suspensions != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (solicited_preemptions != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (interrupt_preemptions != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (priority_inversions != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (time_slices != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (relinquishes != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (timeouts != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (wait_aborts != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (last_preempted_by != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
#endif

    /* Return completion status.  */
    return(status);
}

