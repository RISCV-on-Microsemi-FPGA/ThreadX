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
#include "tx_timer.h"

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_thread_sleep                                    PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function handles application thread sleep requests.  If the    */ 
/*    sleep request was called from a non-thread, an error is returned.   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    timer_ticks                           Number of timer ticks to sleep*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Return completion status      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _tx_thread_system_suspend         Actual thread suspension          */ 
/*    _tx_thread_system_ni_suspend      Non-interruptable suspend thread  */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-12-2005     William E. Lamie         Initial Version 5.0           */ 
/*  04-02-2007     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.1    */ 
/*  12-12-2008     William E. Lamie         Modified comment(s), added    */ 
/*                                            filter option to trace      */ 
/*                                            insert, added optional      */ 
/*                                            logic for non-interruptable */ 
/*                                            operation, and added macro  */ 
/*                                            to get current thread,      */ 
/*                                            resulting in version 5.2    */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), removed  */ 
/*                                            compound conditionals, added*/ 
/*                                            explicit value checking,    */ 
/*                                            changed logic to use a macro*/ 
/*                                            to get the system state,    */ 
/*                                            merged event logging        */ 
/*                                            support, and added logic    */ 
/*                                            to explicitly check for     */ 
/*                                            valid pointer, resulting    */ 
/*                                            in version 5.4              */ 
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
UINT  _tx_thread_sleep(ULONG timer_ticks)
{

TX_INTERRUPT_SAVE_AREA

UINT            status;
TX_THREAD       *thread_ptr;


    /* Lockout interrupts while the thread is being resumed.  */
    TX_DISABLE

    /* Pickup thread pointer.  */
    TX_THREAD_GET_CURRENT(thread_ptr)

    /* Determine if this is a legal request.  */

    /* Is there a current thread?  */
    if (thread_ptr == TX_NULL)
    {

        /* Restore interrupts.  */
        TX_RESTORE
        
        /* Illegal caller of this service.  */
        status =  TX_CALLER_ERROR;
    }
    
    /* Is the caller an ISR or Initialization?  */
    else if (TX_THREAD_GET_SYSTEM_STATE() != ((ULONG) 0))
    {

        /* Restore interrupts.  */
        TX_RESTORE
        
        /* Illegal caller of this service.  */
        status =  TX_CALLER_ERROR;
    }

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Is the caller the system timer thread?  */
    else if (thread_ptr == &_tx_timer_thread)
    {

        /* Restore interrupts.  */
        TX_RESTORE
        
        /* Illegal caller of this service.  */
        status =  TX_CALLER_ERROR;
    }
#endif

    /* Determine if the requested number of ticks is zero.  */
    else if (timer_ticks == ((ULONG) 0))
    { 

        /* Restore interrupts.  */
        TX_RESTORE
      
        /* Just return with a successful status.  */
        status =  TX_SUCCESS;
    }
    else
    {

        /* If trace is enabled, insert this event into the trace buffer.  */
        TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_SLEEP, TX_ULONG_TO_POINTER_CONVERT(timer_ticks), thread_ptr -> tx_thread_state, TX_POINTER_TO_ULONG_CONVERT(&status), 0, TX_TRACE_THREAD_EVENTS)

        /* Log this kernel call.  */
        TX_EL_THREAD_SLEEP_INSERT

        /* Suspend the current thread.  */

        /* Set the state to suspended.  */
        thread_ptr -> tx_thread_state =    TX_SLEEP;

#ifdef TX_NOT_INTERRUPTABLE

        /* Call actual non-interruptable thread suspension routine.  */
        _tx_thread_system_ni_suspend(thread_ptr, timer_ticks);

        /* Restore interrupts.  */
        TX_RESTORE
#else

        /* Set the suspending flag. */
        thread_ptr -> tx_thread_suspending =  TX_TRUE;

        /* Initialize the status to successful.  */
        thread_ptr -> tx_thread_suspend_status =  TX_SUCCESS;

        /* Setup the timeout period.  */
        thread_ptr -> tx_thread_timer.tx_timer_internal_remaining_ticks =  timer_ticks;

        /* Temporarily disable preemption.  */
        _tx_thread_preempt_disable++;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Call actual thread suspension routine.  */
        _tx_thread_system_suspend(thread_ptr);
#endif

        /* Return status to the caller.  */
        status =  thread_ptr -> tx_thread_suspend_status;
    }
    
    /* Return completion status.  */
    return(status);
}

