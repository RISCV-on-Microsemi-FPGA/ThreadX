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


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_thread_shell_entry                              PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function calls the specified entry function of the thread.  It */ 
/*    also provides a place for the thread's entry function to return.    */ 
/*    If the thread returns, this function places the thread in a         */ 
/*    "COMPLETED" state.                                                  */ 
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
/*    thread_entry                      Thread's entry function           */ 
/*    _tx_thread_system_suspend         Thread suspension routine         */ 
/*    _tx_thread_system_ni_suspend      Non-interruptable suspend thread  */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Initial thread stack frame                                          */ 
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
/*                                            added state change macro,   */ 
/*                                            cleared the timeout value   */ 
/*                                            to avoid a timeout on a     */ 
/*                                            completed thread, moved     */ 
/*                                            extension processing to     */ 
/*                                            interrupt enabled area,     */ 
/*                                            added optional logic for    */ 
/*                                            non-interruptable operation,*/ 
/*                                            and made several            */ 
/*                                            optimizations, resulting    */ 
/*                                            in version 5.2              */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), added    */ 
/*                                            logic to explicitly check   */ 
/*                                            for valid pointer, and      */ 
/*                                            safety critical exception   */ 
/*                                            if control returns after    */ 
/*                                            the thread enters the       */ 
/*                                            completed state, resulting  */ 
/*                                            in version 5.4              */ 
/*  07-15-2011     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.5    */ 
/*  11-01-2012     William E. Lamie         Modified comment(s), and      */ 
/*                                            added logic to remove all   */ 
/*                                            mutexes owned by thread     */ 
/*                                            when it completes,          */ 
/*                                            resulting in version 5.6    */ 
/*  05-01-2015     William E. Lamie         Modified comment(s), added    */ 
/*                                            thread start macro for      */ 
/*                                            performing port and/or user */ 
/*                                            specified processing when   */ 
/*                                            a thread starts, and        */ 
/*                                            modified code for MISRA     */ 
/*                                            compliance, resulting in    */ 
/*                                            version 5.7                 */ 
/*                                                                        */ 
/**************************************************************************/ 
VOID  _tx_thread_shell_entry(VOID)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD       *thread_ptr;
#ifndef TX_DISABLE_NOTIFY_CALLBACKS
VOID            (*entry_exit_notify)(TX_THREAD *notify_thread_ptr, UINT type);
#endif


    /* Pickup thread pointer.  */
    TX_THREAD_GET_CURRENT(thread_ptr)

    /* Perform any additional activities for tool or user purpose.  */
    TX_THREAD_STARTED_EXTENSION(thread_ptr)

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

    /* Disable interrupts.  */
    TX_DISABLE

    /* Pickup the entry/exit application callback routine.  */
    entry_exit_notify =  thread_ptr -> tx_thread_entry_exit_notify;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Determine if an application callback routine is specified.  */
    if (entry_exit_notify != TX_NULL)
    {

        /* Yes, notify application that this thread has been entered!  */
        (entry_exit_notify)(thread_ptr, TX_THREAD_ENTRY);
    }
#endif

    /* Call current thread's entry function.  */
    (thread_ptr -> tx_thread_entry) (thread_ptr -> tx_thread_entry_parameter);

    /* Suspend thread with a "completed" state.  */

    /* Determine if the application is using mutexes.  */
    if (_tx_thread_mutex_release != TX_NULL)
    {

        /* Yes, call the mutex release function via a function pointer that 
           is setup during mutex initialization.  */
        (_tx_thread_mutex_release)(thread_ptr);
    }

    /* Lockout interrupts while the thread state is setup.  */
    TX_DISABLE

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

    /* Pickup the entry/exit application callback routine again.  */
    entry_exit_notify =  thread_ptr -> tx_thread_entry_exit_notify;
#endif

    /* Set the status to suspending, in order to indicate the suspension
       is in progress.  */
    thread_ptr -> tx_thread_state =  TX_COMPLETED;

    /* Thread state change.  */
    TX_THREAD_STATE_CHANGE(thread_ptr, TX_COMPLETED)

#ifdef TX_NOT_INTERRUPTABLE

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

    /* Determine if an application callback routine is specified.  */
    if (entry_exit_notify != TX_NULL)
    {

        /* Yes, notify application that this thread has exited!  */
        (entry_exit_notify)(thread_ptr, TX_THREAD_EXIT);
    }
#endif

    /* Perform any additional activities for tool or user purpose.  */
    TX_THREAD_COMPLETED_EXTENSION(thread_ptr)

    /* Call actual non-interruptable thread suspension routine.  */
    _tx_thread_system_ni_suspend(thread_ptr, ((ULONG) 0));

    /* Restore interrupts.  */
    TX_RESTORE
#else

    /* Set the suspending flag. */
    thread_ptr -> tx_thread_suspending =  TX_TRUE;

    /* Setup for no timeout period.  */
    thread_ptr -> tx_thread_timer.tx_timer_internal_remaining_ticks =  ((ULONG) 0);

    /* Temporarily disable preemption.  */
    _tx_thread_preempt_disable++;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Perform any additional activities for tool or user purpose.  */
    TX_THREAD_COMPLETED_EXTENSION(thread_ptr)

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


#ifdef TX_SAFETY_CRITICAL

    /* If we ever get here, raise safety critical exception.  */
    TX_SAFETY_CRITICAL_EXCEPTION(__FILE__, __LINE__, 0);
#endif
}

