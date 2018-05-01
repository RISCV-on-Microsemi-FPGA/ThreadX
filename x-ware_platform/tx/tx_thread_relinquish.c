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
#ifndef TX_NO_TIMER
#include "tx_timer.h"
#endif


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_thread_relinquish                               PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function moves the currently executing thread to the end of    */ 
/*    the list of threads ready at the same priority. If no other threads */ 
/*    of the same or higher priority are ready, this function simply      */ 
/*    returns.                                                            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _tx_thread_system_return              Return to the system          */ 
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
/*  12-12-2008     William E. Lamie         Modified comment(s), added    */ 
/*                                            macro to get current thread,*/ 
/*                                            moved relinquish trace event*/ 
/*                                            added next thread parameter,*/ 
/*                                            added filter option to      */ 
/*                                            trace insert, added stack   */ 
/*                                            check macro, and removed    */ 
/*                                            unnecessary code,           */ 
/*                                            resulting in version 5.2    */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), merged   */ 
/*                                            event logging support, and  */ 
/*                                            added logic to reset the    */ 
/*                                            time-slice, resulting in    */ 
/*                                            version 5.4                 */ 
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
VOID  _tx_thread_relinquish(VOID)
{

TX_INTERRUPT_SAVE_AREA

UINT            priority;                   
TX_THREAD       *thread_ptr;                


    /* Pickup thread pointer.  */
    TX_THREAD_GET_CURRENT(thread_ptr)

#ifdef TX_ENABLE_STACK_CHECKING 

    /* Check this thread's stack.  */
    TX_THREAD_STACK_CHECK(thread_ptr)
#endif

    /* Disable interrupts.  */
    TX_DISABLE

#ifndef TX_NO_TIMER

    /* Reset time slice for current thread.  */
    _tx_timer_time_slice =  thread_ptr -> tx_thread_new_time_slice;
#endif

    /* Pickup the thread's priority.  */
    priority =  thread_ptr -> tx_thread_priority; 

    /* Determine if there is another thread at the same priority.  */
    if (thread_ptr -> tx_thread_ready_next != thread_ptr)
    {

        /* Yes, there is another thread at this priority, make it the highest at
           this priority level.  */
        _tx_thread_priority_list[priority] =  thread_ptr -> tx_thread_ready_next;
    
        /* Mark the new thread as the one to execute.  */
        _tx_thread_execute_ptr = thread_ptr -> tx_thread_ready_next;
    }

    /* Determine if there is a higher-priority thread ready.  */
    if (_tx_thread_highest_priority < priority)
    {

        /* Yes, there is a higher priority thread ready to execute.  Make
           it visible to the thread scheduler.  */
        _tx_thread_execute_ptr =  _tx_thread_priority_list[_tx_thread_highest_priority];

        /* No need to clear the preempted bit in this case, since the currently running
           thread must already have its preempted bit clear.  */
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_RELINQUISH, &thread_ptr, TX_POINTER_TO_ULONG_CONVERT(_tx_thread_execute_ptr), 0, 0, TX_TRACE_THREAD_EVENTS)

    /* Log this kernel call.  */
    TX_EL_THREAD_RELINQUISH_INSERT

    /* Restore previous interrupt posture.  */
    TX_RESTORE

    /* Determine if this thread needs to return to the system.  */
    if (_tx_thread_execute_ptr != thread_ptr)
    {

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

        /* Increment the number of thread relinquishes.  */
        thread_ptr -> tx_thread_performance_relinquish_count++;

        /* Increment the total number of thread relinquish operations.  */
        _tx_thread_performance_relinquish_count++;

        /* Increment the non-idle return count.  */
        _tx_thread_performance_non_idle_return_count++;
#endif

#ifdef TX_ENABLE_STACK_CHECKING 

        /* Pickup the next execute pointer.  */
        thread_ptr =  _tx_thread_execute_ptr;

        /* Check this thread's stack.  */
        TX_THREAD_STACK_CHECK(thread_ptr)
#endif

        /* Transfer control to the system so the scheduler can execute
           the next thread.  */
        _tx_thread_system_return();
    }
}

