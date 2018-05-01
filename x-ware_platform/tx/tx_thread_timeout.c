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
/*    _tx_thread_timeout                                  PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function handles thread timeout processing.  Timeouts occur in */ 
/*    two flavors, namely the thread sleep timeout and all other service  */ 
/*    call timeouts.  Thread sleep timeouts are processed locally, while  */ 
/*    the others are processed by the appropriate suspension clean-up     */ 
/*    service.                                                            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    timeout_input                         Contains the thread pointer   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    Suspension Cleanup Functions                                        */ 
/*    _tx_thread_system_resume          Resume thread                     */ 
/*    _tx_thread_system_ni_resume       Non-interruptable resume thread   */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _tx_timer_expiration_process          Timer expiration function     */ 
/*    _tx_timer_thread_entry                Timer thread function         */ 
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
/*                                            resulting in version 5.2    */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), and      */ 
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
VOID  _tx_thread_timeout(ULONG timeout_input)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD       *thread_ptr;                
VOID            (*suspend_cleanup)(struct TX_THREAD_STRUCT *suspend_thread_ptr);


    /* Pickup the thread pointer.  */
    thread_ptr =  (TX_THREAD *) TX_ULONG_TO_POINTER_CONVERT(timeout_input);

    /* Disable interrupts.  */
    TX_DISABLE

    /* Determine how the thread is currently suspended.  */
    if (thread_ptr -> tx_thread_state == TX_SLEEP)
    {

#ifdef TX_NOT_INTERRUPTABLE

        /* Resume the thread!  */
        _tx_thread_system_ni_resume(thread_ptr);

        /* Restore interrupts.  */
        TX_RESTORE
#else

        /* Increment the disable preemption flag.  */
        _tx_thread_preempt_disable++;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Lift the suspension on the sleeping thread.  */
        _tx_thread_system_resume(thread_ptr);
#endif
    }
    else
    {

        /* Process all other suspension timeouts.  */

#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

        /* Increment the total number of thread timeouts.  */
        _tx_thread_performance_timeout_count++;

        /* Increment the number of timeouts for this thread.  */
        thread_ptr -> tx_thread_performance_timeout_count++;
#endif
    
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

#ifdef TX_NOT_INTERRUPTABLE

        /* Restore interrupts.  */
        TX_RESTORE
#endif
    }
}

