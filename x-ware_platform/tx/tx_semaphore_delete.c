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
/*    _tx_semaphore_delete                                PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deletes the specified semaphore.  All threads         */ 
/*    suspended on the semaphore are resumed with the TX_DELETED status   */ 
/*    code.                                                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    semaphore_ptr                     Pointer to semaphore control block*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    TX_SUCCESS                        Successful completion status      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _tx_thread_system_preempt_check   Check for preemption              */ 
/*    _tx_thread_system_resume          Resume thread service             */ 
/*    _tx_thread_system_ni_resume       Non-interruptable resume thread   */ 
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
/*                                            filter option to trace      */ 
/*                                            insert, added optional      */ 
/*                                            logic for non-interruptable */ 
/*                                            operation, and made several */ 
/*                                            optimizations, resulting    */ 
/*                                            in version 5.2              */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), merged   */ 
/*                                            event logging support,      */ 
/*                                            eliminated created_count    */ 
/*                                            local variable, and         */ 
/*                                            changed some counting       */ 
/*                                            variables to type UINT,     */ 
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
UINT  _tx_semaphore_delete(TX_SEMAPHORE *semaphore_ptr)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD       *thread_ptr;                
TX_THREAD       *next_thread;
UINT            suspended_count;
TX_SEMAPHORE    *next_semaphore;
TX_SEMAPHORE    *previous_semaphore;


    /* Disable interrupts to remove the semaphore from the created list.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_SEMAPHORE_DELETE, semaphore_ptr, TX_POINTER_TO_ULONG_CONVERT(&thread_ptr), 0, 0, TX_TRACE_SEMAPHORE_EVENTS)

    /* Optional semaphore delete extended processing.  */
    TX_SEMAPHORE_DELETE_EXTENSION(semaphore_ptr)

    /* If trace is enabled, unregister this object.  */
    TX_TRACE_OBJECT_UNREGISTER(semaphore_ptr)

    /* Log this kernel call.  */
    TX_EL_SEMAPHORE_DELETE_INSERT

    /* Clear the semaphore ID to make it invalid.  */
    semaphore_ptr -> tx_semaphore_id =  TX_CLEAR_ID;

    /* Decrement the number of semaphores.  */
    _tx_semaphore_created_count--;

    /* See if the semaphore is the only one on the list.  */
    if (_tx_semaphore_created_count == TX_EMPTY)
    {

        /* Only created semaphore, just set the created list to NULL.  */
        _tx_semaphore_created_ptr =  TX_NULL;
    }
    else
    {

        /* Link-up the neighbors.  */
        next_semaphore =                                   semaphore_ptr -> tx_semaphore_created_next;
        previous_semaphore =                               semaphore_ptr -> tx_semaphore_created_previous;
        next_semaphore -> tx_semaphore_created_previous =  previous_semaphore;
        previous_semaphore -> tx_semaphore_created_next =  next_semaphore;

        /* See if we have to update the created list head pointer.  */
        if (_tx_semaphore_created_ptr == semaphore_ptr)
        {
                    
            /* Yes, move the head pointer to the next link. */
            _tx_semaphore_created_ptr =  next_semaphore; 
        }
    }

    /* Temporarily disable preemption.  */
    _tx_thread_preempt_disable++;

    /* Pickup the suspension information.  */
    thread_ptr =                                     semaphore_ptr -> tx_semaphore_suspension_list;
    semaphore_ptr -> tx_semaphore_suspension_list =  TX_NULL;
    suspended_count =                                semaphore_ptr -> tx_semaphore_suspended_count;
    semaphore_ptr -> tx_semaphore_suspended_count =  TX_NO_SUSPENSIONS;   

    /* Restore interrupts.  */
    TX_RESTORE

    /* Walk through the semaphore list to resume any and all threads suspended
       on this semaphore.  */
    while (suspended_count != TX_NO_SUSPENSIONS)
    {
    
        /* Decrement the suspension count.  */
        suspended_count--;

        /* Lockout interrupts.  */
        TX_DISABLE

        /* Clear the cleanup pointer, this prevents the timeout from doing 
           anything.  */
        thread_ptr -> tx_thread_suspend_cleanup =  TX_NULL;

        /* Set the return status in the thread to TX_DELETED.  */
        thread_ptr -> tx_thread_suspend_status =  TX_DELETED;

        /* Move the thread pointer ahead.  */
        next_thread =  thread_ptr -> tx_thread_suspended_next;

#ifdef TX_NOT_INTERRUPTABLE

        /* Resume the thread!  */
        _tx_thread_system_ni_resume(thread_ptr);

        /* Restore interrupts.  */
        TX_RESTORE
#else

        /* Temporarily disable preemption again.  */
        _tx_thread_preempt_disable++;

        /* Restore interrupts.  */
        TX_RESTORE
    
        /* Resume the thread.  */
        _tx_thread_system_resume(thread_ptr);
#endif

        /* Move to next thread.  */
        thread_ptr =  next_thread;
    }

    /* Disable interrupts.  */
    TX_DISABLE

    /* Release previous preempt disable.  */
    _tx_thread_preempt_disable--;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Check for preemption.  */
    _tx_thread_system_preempt_check();

    /* Return TX_SUCCESS.  */
    return(TX_SUCCESS);
}

