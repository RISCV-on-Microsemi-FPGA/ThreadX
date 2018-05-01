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
/**   Semaphore                                                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"
#include "tx_thread.h"
#include "tx_semaphore.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_semaphore_get                                   PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function gets an instance from the specified counting          */ 
/*    semaphore.                                                          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    semaphore_ptr                     Pointer to semaphore control block*/ 
/*    wait_option                       Suspension option                 */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Completion status                 */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _tx_thread_system_suspend         Suspend thread service            */ 
/*    _tx_thread_system_ni_suspend      Non-interruptable suspend thread  */ 
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
/*                                            added filter option to trace*/ 
/*                                            insert, added optional      */ 
/*                                            logic for non-interruptable */ 
/*                                            operation, and made several */ 
/*                                            optimizations, resulting    */ 
/*                                            in version 5.2              */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), changed  */ 
/*                                            some counting variables to  */ 
/*                                            type UINT, merged event     */ 
/*                                            logging support, and added  */ 
/*                                            explicit value checking,    */ 
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
UINT  _tx_semaphore_get(TX_SEMAPHORE *semaphore_ptr, ULONG wait_option)
{

TX_INTERRUPT_SAVE_AREA
            
TX_THREAD       *thread_ptr;            
TX_THREAD       *next_thread;
TX_THREAD       *previous_thread;
UINT            status;


    /* Default the status to TX_SUCCESS.  */
    status =  TX_SUCCESS;

    /* Disable interrupts to get an instance from the semaphore.  */
    TX_DISABLE

#ifdef TX_SEMAPHORE_ENABLE_PERFORMANCE_INFO

    /* Increment the total semaphore get counter.  */
    _tx_semaphore_performance_get_count++;

    /* Increment the number of attempts to get this semaphore.  */
    semaphore_ptr -> tx_semaphore_performance_get_count++;
#endif

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_SEMAPHORE_GET, semaphore_ptr, wait_option, semaphore_ptr -> tx_semaphore_count, TX_POINTER_TO_ULONG_CONVERT(&thread_ptr), TX_TRACE_SEMAPHORE_EVENTS)

    /* Log this kernel call.  */
    TX_EL_SEMAPHORE_GET_INSERT

    /* Determine if there is an instance of the semaphore.  */
    if (semaphore_ptr -> tx_semaphore_count != ((ULONG) 0))
    {

        /* Decrement the semaphore count.  */
        semaphore_ptr -> tx_semaphore_count--;

        /* Restore interrupts.  */
        TX_RESTORE
    }

    /* Determine if the request specifies suspension.  */
    else if (wait_option != TX_NO_WAIT)
    {

        /* Prepare for suspension of this thread.  */

#ifdef TX_SEMAPHORE_ENABLE_PERFORMANCE_INFO

        /* Increment the total semaphore suspensions counter.  */
        _tx_semaphore_performance_suspension_count++;

        /* Increment the number of suspensions on this semaphore.  */
        semaphore_ptr -> tx_semaphore_performance_suspension_count++;
#endif
            
        /* Pickup thread pointer.  */
        TX_THREAD_GET_CURRENT(thread_ptr)

        /* Setup cleanup routine pointer.  */
        thread_ptr -> tx_thread_suspend_cleanup =  &(_tx_semaphore_cleanup);

        /* Setup cleanup information, i.e. this semaphore control
           block.  */
        thread_ptr -> tx_thread_suspend_control_block =  (VOID *) semaphore_ptr;

        /* Setup suspension list.  */
        if (semaphore_ptr -> tx_semaphore_suspended_count == TX_NO_SUSPENSIONS)
        {

            /* No other threads are suspended.  Setup the head pointer and
               just setup this threads pointers to itself.  */
            semaphore_ptr -> tx_semaphore_suspension_list =         thread_ptr;
            thread_ptr -> tx_thread_suspended_next =                thread_ptr;
            thread_ptr -> tx_thread_suspended_previous =            thread_ptr;
        }
        else
        {

            /* This list is not NULL, add current thread to the end. */
            next_thread =                                   semaphore_ptr -> tx_semaphore_suspension_list;
            thread_ptr -> tx_thread_suspended_next =        next_thread;
            previous_thread =                               next_thread -> tx_thread_suspended_previous;
            thread_ptr -> tx_thread_suspended_previous =    previous_thread;
            previous_thread -> tx_thread_suspended_next =   thread_ptr;
            next_thread -> tx_thread_suspended_previous =   thread_ptr;
        }

        /* Increment the number of suspensions.  */
        semaphore_ptr -> tx_semaphore_suspended_count++;

        /* Set the state to suspended.  */
        thread_ptr -> tx_thread_state =    TX_SEMAPHORE_SUSP;

#ifdef TX_NOT_INTERRUPTABLE

        /* Call actual non-interruptable thread suspension routine.  */
        _tx_thread_system_ni_suspend(thread_ptr, wait_option);

        /* Restore interrupts.  */
        TX_RESTORE
#else

        /* Set the suspending flag.  */
        thread_ptr -> tx_thread_suspending =  TX_TRUE;

        /* Setup the timeout period.  */
        thread_ptr -> tx_thread_timer.tx_timer_internal_remaining_ticks =  wait_option;

        /* Temporarily disable preemption.  */
        _tx_thread_preempt_disable++;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Call actual thread suspension routine.  */
        _tx_thread_system_suspend(thread_ptr);
#endif

        /* Return the completion status.  */
        status =  thread_ptr -> tx_thread_suspend_status;
    }
    else
    {

        /* Restore interrupts.  */
        TX_RESTORE

        /* Immediate return, return error completion.  */
        status =  TX_NO_INSTANCE;
    }

    /* Return completion status.  */
    return(status);
}

