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
/*    _tx_thread_terminate                                PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function handles application thread terminate requests.  Once  */ 
/*    a thread is terminated, it cannot be executed again unless it is    */ 
/*    deleted and recreated.                                              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    thread_ptr                            Pointer to thread to suspend  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Return completion status      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _tx_timer_system_deactivate           Timer deactivate function     */ 
/*    _tx_thread_system_suspend             Actual thread suspension      */ 
/*    _tx_thread_system_ni_suspend          Non-interruptable suspend     */ 
/*                                            thread                      */ 
/*    _tx_thread_system_preempt_check       Check for preemption          */ 
/*    Suspend Cleanup Routine               Suspension cleanup function   */ 
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
/*                                            state change macro, added   */ 
/*                                            filter option to trace      */ 
/*                                            insert, moved extension     */ 
/*                                            processing to interrupt     */ 
/*                                            enabled area, added optional*/ 
/*                                            logic for non-interruptable */ 
/*                                            operation, and made several */ 
/*                                            optimizations, resulting    */ 
/*                                            in version 5.2              */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), removed  */ 
/*                                            compound conditionals,      */ 
/*                                            merged event logging        */ 
/*                                            support, and added logic to */ 
/*                                            explicitly check for valid  */ 
/*                                            pointer, resulting in       */ 
/*                                            version 5.4                 */ 
/*  07-15-2011     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.5    */ 
/*  11-01-2012     William E. Lamie         Modified comment(s), and      */ 
/*                                            added logic to remove all   */ 
/*                                            mutexes owned by thread     */ 
/*                                            when it is terminated,      */ 
/*                                            resulting in version 5.6    */ 
/*  05-01-2015     William E. Lamie         Modified comment(s), and      */ 
/*                                            modified code for MISRA     */ 
/*                                            compliance, resulting in    */ 
/*                                            version 5.7                 */ 
/*                                                                        */ 
/**************************************************************************/ 
UINT  _tx_thread_terminate(TX_THREAD *thread_ptr)
{

TX_INTERRUPT_SAVE_AREA

VOID        (*suspend_cleanup)(struct TX_THREAD_STRUCT *suspend_thread_ptr);
#ifndef TX_DISABLE_NOTIFY_CALLBACKS
VOID        (*entry_exit_notify)(TX_THREAD *notify_thread_ptr, UINT id);
#endif
UINT        status;


    /* Default to successful completion.  */
    status =  TX_SUCCESS;

    /* Determine if the application is using mutexes.  */
    if (_tx_thread_mutex_release != TX_NULL)
    {

        /* Yes, call the mutex release function via a function pointer that 
           is setup during initialization.  */
        (_tx_thread_mutex_release)(thread_ptr);
    }

#ifdef TX_NOT_INTERRUPTABLE

    /* Lockout interrupts while the thread is being terminated.  */
    TX_DISABLE

    /* Deactivate thread timer, if active.  */
    _tx_timer_system_deactivate(&thread_ptr -> tx_thread_timer);

#else

    /* Deactivate thread timer, if active.  */
    _tx_timer_system_deactivate(&thread_ptr -> tx_thread_timer);

    /* Lockout interrupts while the thread is being terminated.  */
    TX_DISABLE
#endif

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_TERMINATE, thread_ptr, thread_ptr -> tx_thread_state, TX_POINTER_TO_ULONG_CONVERT(&suspend_cleanup), 0, TX_TRACE_THREAD_EVENTS)

    /* Log this kernel call.  */
    TX_EL_THREAD_TERMINATE_INSERT

    /* Is the thread already terminated?  */
    if (thread_ptr -> tx_thread_state == TX_TERMINATED)
    {
    
        /* Restore interrupts.  */
        TX_RESTORE

        /* Return success since thread is already terminated.  */
        status =  TX_SUCCESS; 
    }

    /* Check the specified thread's current status.  */
    else if (thread_ptr -> tx_thread_state != TX_COMPLETED)
    {


#ifndef TX_DISABLE_NOTIFY_CALLBACKS

        /* Pickup the entry/exit application callback routine.  */
        entry_exit_notify =  thread_ptr -> tx_thread_entry_exit_notify;
#endif

        /* Check to see if the thread is currently ready.  */
        if (thread_ptr -> tx_thread_state == TX_READY)
        {

            /* Set the state to terminated.  */
            thread_ptr -> tx_thread_state =  TX_TERMINATED;

            /* Thread state change.  */
            TX_THREAD_STATE_CHANGE(thread_ptr, TX_TERMINATED)

#ifdef TX_NOT_INTERRUPTABLE

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

            /* Determine if an application callback routine is specified.  */
            if (entry_exit_notify != TX_NULL)
            {

                /* Yes, notify application that this thread has exited!  */
                (entry_exit_notify)(thread_ptr, TX_THREAD_EXIT);
            }
#endif

            /* Call actual non-interruptable thread suspension routine.  */
            _tx_thread_system_ni_suspend(thread_ptr, ((ULONG) 0));

            /* Restore interrupts.  */
            TX_RESTORE
#else

            /* Set the suspending flag.  */
            thread_ptr -> tx_thread_suspending =  TX_TRUE;

            /* Setup for no timeout period.  */
            thread_ptr -> tx_thread_timer.tx_timer_internal_remaining_ticks =  ((ULONG) 0);

            /* Disable internal preemption.  */
            _tx_thread_preempt_disable++;

            /* Since the thread is currently ready, we don't need to
               worry about calling the suspend cleanup routine!  */

            /* Restore interrupts.  */
            TX_RESTORE

            /* Perform any additional activities for tool or user purpose.  */
            TX_THREAD_TERMINATED_EXTENSION(thread_ptr)

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

            /* Determine if an application callback routine is specified.  */
            if (entry_exit_notify != TX_NULL)
            {

                /* Yes, notify application that this thread has exited!  */
                (entry_exit_notify)(thread_ptr, TX_THREAD_EXIT);
            }
#endif

            /* Call actual thread suspension routine.  */
            _tx_thread_system_suspend(thread_ptr);
#endif
        }
        else
        {

            /* Change the state to terminated.  */
            thread_ptr -> tx_thread_state =    TX_TERMINATED;

            /* Thread state change.  */
            TX_THREAD_STATE_CHANGE(thread_ptr, TX_TERMINATED)

            /* Set the suspending flag.  This prevents the thread from being 
               resumed before the cleanup routine is executed.  */
            thread_ptr -> tx_thread_suspending =  TX_TRUE;

            /* Pickup the cleanup routine address.  */
            suspend_cleanup =  thread_ptr -> tx_thread_suspend_cleanup;

#ifndef TX_NOT_INTERRUPTABLE

            /* Restore interrupts.  */
            TX_RESTORE
#endif

            /* Call any cleanup routines.  */
            if (suspend_cleanup != TX_NULL)
            {

                /* Yes, there is a function to call.  */
                (suspend_cleanup)(thread_ptr);
            }

#ifndef TX_NOT_INTERRUPTABLE

            /* Disable interrupts.  */
            TX_DISABLE
#endif

            /* Clear the suspending flag.  */
            thread_ptr -> tx_thread_suspending =  TX_FALSE;

            /* Restore interrupts.  */
            TX_RESTORE

            /* Perform any additional activities for tool or user purpose.  */
            TX_THREAD_TERMINATED_EXTENSION(thread_ptr)

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

            /* Determine if an application callback routine is specified.  */
            if (entry_exit_notify != TX_NULL)
            {

                /* Yes, notify application that this thread has exited!  */
                (entry_exit_notify)(thread_ptr, TX_THREAD_EXIT);
            }
#endif
        }
    }
    else
    {

        /* Restore interrupts.  */
        TX_RESTORE
    }

    /* Check for preemption.  */
    _tx_thread_system_preempt_check();

    /* Return completion status.  */
    return(status);
}

