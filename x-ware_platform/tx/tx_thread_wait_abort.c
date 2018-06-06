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
/*    _tx_thread_wait_abort                               PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function aborts the wait condition that the specified thread   */ 
/*    is in - regardless of what object the thread is waiting on - and    */ 
/*    returns a TX_WAIT_ABORTED status to the specified thread.           */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    thread_ptr                            Thread to abort the wait on   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Return completion status      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    Suspension Cleanup Functions                                        */ 
/*    _tx_thread_system_resume                                            */ 
/*    _tx_thread_system_ni_resume       Non-interruptable resume thread   */ 
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
/*                                            optional logic for          */ 
/*                                            non-interruptable operation,*/ 
/*                                            and added filter option to  */ 
/*                                            trace insert, resulting     */ 
/*                                            in version 5.2              */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), merged   */ 
/*                                            event logging support, and  */ 
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
UINT  _tx_thread_wait_abort(TX_THREAD  *thread_ptr)
{

TX_INTERRUPT_SAVE_AREA

VOID            (*suspend_cleanup)(struct TX_THREAD_STRUCT *suspend_thread_ptr);
UINT            status;


    /* Disable interrupts.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_WAIT_ABORT, thread_ptr, thread_ptr -> tx_thread_state, 0, 0, TX_TRACE_THREAD_EVENTS)

    /* Log this kernel call.  */
    TX_EL_THREAD_WAIT_ABORT_INSERT

    /* Determine if the thread is currently suspended.  */
    if (thread_ptr -> tx_thread_state < TX_SLEEP)
    {
    
        /* Thread is either ready, completed, terminated, or in a pure 
           suspension condition.  */

        /* Restore interrupts.  */
        TX_RESTORE

        /* Just return with an error message to indicate that 
           nothing was done.  */
        status =  TX_WAIT_ABORT_ERROR;
    }    
    else
    {

        /* Check for a sleep condition.  */
        if (thread_ptr -> tx_thread_state == TX_SLEEP)
        {

            /* Set the state to terminated.  */
            thread_ptr -> tx_thread_state =    TX_SUSPENDED;

            /* Set the TX_WAIT_ABORTED status in the thread that is
               sleeping.  */
            thread_ptr -> tx_thread_suspend_status =  TX_WAIT_ABORTED;

            /* Make sure there isn't a suspend cleanup routine.  */
            thread_ptr -> tx_thread_suspend_cleanup =  TX_NULL;

#ifndef TX_NOT_INTERRUPTABLE

            /* Increment the disable preemption flag.  */
            _tx_thread_preempt_disable++;

            /* Restore interrupts.  */
            TX_RESTORE
#endif
        }
        else
        {

            /* Process all other suspension timeouts.  */
    
            /* Set the state to suspended.  */
            thread_ptr -> tx_thread_state =    TX_SUSPENDED;

            /* Pickup the cleanup routine address.  */
            suspend_cleanup =  thread_ptr -> tx_thread_suspend_cleanup;

            /* Set the TX_WAIT_ABORTED status in the thread that was
               suspended.  */
            thread_ptr -> tx_thread_suspend_status =  TX_WAIT_ABORTED;

#ifndef TX_NOT_INTERRUPTABLE

            /* Increment the disable preemption flag.  */
            _tx_thread_preempt_disable++;

            /* Restore interrupts.  */
            TX_RESTORE
#endif

            /* Call any cleanup routines.  */
            if (suspend_cleanup != TX_NULL)
            {

                /* Yes, there is a function to call.  */
                (suspend_cleanup)(thread_ptr);
            }
        }

        /* If the abort of the thread wait was successful, if so resume the thread.  */
        if (thread_ptr -> tx_thread_suspend_status == TX_WAIT_ABORTED)
        {

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

            /* Increment the total number of thread wait aborts.  */
            _tx_thread_performance_wait_abort_count++;

            /* Increment this thread's wait abort count.  */
            thread_ptr -> tx_thread_performance_wait_abort_count++;
#endif

#ifdef TX_NOT_INTERRUPTABLE

            /* Resume the thread!  */
            _tx_thread_system_ni_resume(thread_ptr);

            /* Restore interrupts.  */
            TX_RESTORE
#else

            /* Lift the suspension on the previously waiting thread.  */
            _tx_thread_system_resume(thread_ptr);
#endif

            /* Return a successful status.  */
            status =  TX_SUCCESS;
        }
        else
        {

#ifdef TX_NOT_INTERRUPTABLE

            /* Restore interrupts.  */
            TX_RESTORE

#else 

            /* Disable interrupts.  */
            TX_DISABLE
        
            /* Decrement the disable preemption flag.  */
            _tx_thread_preempt_disable--;

            /* Restore interrupts.  */
            TX_RESTORE
#endif

            /* Return with an error message to indicate that 
               nothing was done.  */
            status =  TX_WAIT_ABORT_ERROR;
        }
    }

    /* Return completion status.  */
    return(status);
}

