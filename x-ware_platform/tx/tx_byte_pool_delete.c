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
/**   Byte Pool                                                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"
#include "tx_thread.h"
#include "tx_byte_pool.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_byte_pool_delete                                PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deletes the specified byte pool.  All threads         */ 
/*    suspended on the byte pool are resumed with the TX_DELETED status   */ 
/*    code.                                                               */ 
/*                                                                        */ 
/*    It is important to note that the byte pool being deleted, or the    */ 
/*    memory associated with it should not be in use when this function   */ 
/*    is called.                                                          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pool_ptr                          Pointer to pool control block     */ 
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
UINT  _tx_byte_pool_delete(TX_BYTE_POOL *pool_ptr)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD       *thread_ptr;               
TX_THREAD       *next_thread; 
UINT            suspended_count;
TX_BYTE_POOL    *next_pool;
TX_BYTE_POOL    *previous_pool;


    /* Disable interrupts to remove the byte pool from the created list.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_BYTE_POOL_DELETE, pool_ptr, TX_POINTER_TO_ULONG_CONVERT(&thread_ptr), 0, 0, TX_TRACE_BYTE_POOL_EVENTS)

    /* Optional byte pool delete extended processing.  */
    TX_BYTE_POOL_DELETE_EXTENSION(pool_ptr)

    /* If trace is enabled, unregister this object.  */
    TX_TRACE_OBJECT_UNREGISTER(pool_ptr)

    /* Log this kernel call.  */
    TX_EL_BYTE_POOL_DELETE_INSERT

    /* Clear the byte pool ID to make it invalid.  */
    pool_ptr -> tx_byte_pool_id =  TX_CLEAR_ID;

    /* Decrement the number of byte pools created.  */
    _tx_byte_pool_created_count--;
    
    /* See if the byte pool is the only one on the list.  */
    if (_tx_byte_pool_created_count == TX_EMPTY)
    {

        /* Only created byte pool, just set the created list to NULL.  */
        _tx_byte_pool_created_ptr =  TX_NULL;
    }
    else
    {

        /* Link-up the neighbors.  */
        next_pool =                                   pool_ptr -> tx_byte_pool_created_next;
        previous_pool =                               pool_ptr -> tx_byte_pool_created_previous;
        next_pool -> tx_byte_pool_created_previous =  previous_pool;
        previous_pool -> tx_byte_pool_created_next =  next_pool;

        /* See if we have to update the created list head pointer.  */
        if (_tx_byte_pool_created_ptr == pool_ptr)
        {
                    
            /* Yes, move the head pointer to the next link. */
            _tx_byte_pool_created_ptr =  next_pool; 
        }
    }

    /* Temporarily disable preemption.  */
    _tx_thread_preempt_disable++;

    /* Pickup the suspension information.  */
    thread_ptr =                                pool_ptr -> tx_byte_pool_suspension_list;    
    pool_ptr -> tx_byte_pool_suspension_list =  TX_NULL;
    suspended_count =                           pool_ptr -> tx_byte_pool_suspended_count;
    pool_ptr -> tx_byte_pool_suspended_count =  TX_NO_SUSPENSIONS;
    
    /* Restore interrupts.  */
    TX_RESTORE

    /* Walk through the byte pool list to resume any and all threads suspended
       on this byte pool.  */
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

