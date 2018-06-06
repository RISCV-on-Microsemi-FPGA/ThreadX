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
/**   Queue                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"
#include "tx_thread.h"
#include "tx_queue.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_queue_flush                                     PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function resets the specified queue, if there are any messages */ 
/*    in it.  Messages waiting to be placed on the queue are also thrown  */ 
/*    out.                                                                */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    queue_ptr                         Pointer to queue control block    */ 
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
/*                                            optional logic for          */ 
/*                                            non-interruptable operation,*/ 
/*                                            and added filter option to  */ 
/*                                            trace insert, resulting     */ 
/*                                            in version 5.2              */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), removed  */ 
/*                                            compound conditionals, added*/ 
/*                                            explicit value checking,    */ 
/*                                            changed some counting       */ 
/*                                            variables to type UINT,     */ 
/*                                            merged event logging        */ 
/*                                            support, and added logic    */ 
/*                                            to check for null thread    */ 
/*                                            pointer, resulting in       */ 
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
UINT  _tx_queue_flush(TX_QUEUE *queue_ptr)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD       *suspension_list;           
UINT            suspended_count;            
TX_THREAD       *thread_ptr;                


    /* Initialize the suspended count and list.  */
    suspended_count =  TX_NO_SUSPENSIONS;
    suspension_list =  TX_NULL;

    /* Disable interrupts to reset various queue parameters.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_QUEUE_FLUSH, queue_ptr, TX_POINTER_TO_ULONG_CONVERT(&thread_ptr), 0, 0, TX_TRACE_QUEUE_EVENTS)

    /* Log this kernel call.  */
    TX_EL_QUEUE_FLUSH_INSERT

    /* Determine if there is something on the queue.  */
    if (queue_ptr -> tx_queue_enqueued != TX_NO_MESSAGES)
    {

        /* Yes, there is something in the queue.  */

        /* Reset the queue parameters to erase all of the queued messages.  */
        queue_ptr -> tx_queue_enqueued =           TX_NO_MESSAGES;
        queue_ptr -> tx_queue_available_storage =  queue_ptr -> tx_queue_capacity;
        queue_ptr -> tx_queue_read =               queue_ptr -> tx_queue_start;
        queue_ptr -> tx_queue_write =              queue_ptr -> tx_queue_start;

        /* Now determine if there are any threads suspended on a full queue.  */
        if (queue_ptr -> tx_queue_suspended_count != TX_NO_SUSPENSIONS)
        {

            /* Yes, there are threads suspended on this queue, they must be 
               resumed!  */

            /* Copy the information into temporary variables.  */
            suspension_list =  queue_ptr -> tx_queue_suspension_list;
            suspended_count =  queue_ptr -> tx_queue_suspended_count;

            /* Clear the queue variables.  */
            queue_ptr -> tx_queue_suspension_list =  TX_NULL;
            queue_ptr -> tx_queue_suspended_count =  TX_NO_SUSPENSIONS;

            /* Temporarily disable preemption.  */
            _tx_thread_preempt_disable++;
        }
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Walk through the queue list to resume any and all threads suspended
       on this queue.  */
    if (suspended_count != TX_NO_SUSPENSIONS)
    {

        /* Pickup the thread to resume.  */
        thread_ptr =  suspension_list;
        while (suspended_count != ((ULONG) 0))
        {
        
            /* Decrement the suspension count.  */
            suspended_count--;

            /* Check for a NULL thread pointer.  */
            if (thread_ptr == TX_NULL)
            {
            
                /* Get out of the loop.  */
                break;
            }

            /* Resume the next suspended thread.  */
            
            /* Lockout interrupts.  */
            TX_DISABLE

            /* Clear the cleanup pointer, this prevents the timeout from doing 
               anything.  */
            thread_ptr -> tx_thread_suspend_cleanup =  TX_NULL;

            /* Set the return status in the thread to TX_SUCCESS.  */
            thread_ptr -> tx_thread_suspend_status =  TX_SUCCESS;

            /* Move the thread pointer ahead.  */
            thread_ptr =  thread_ptr -> tx_thread_suspended_next;

#ifdef TX_NOT_INTERRUPTABLE

            /* Resume the thread!  */
            _tx_thread_system_ni_resume(thread_ptr -> tx_thread_suspended_previous);

            /* Restore interrupts.  */
            TX_RESTORE
#else

            /* Temporarily disable preemption again.  */
            _tx_thread_preempt_disable++;

            /* Restore interrupts.  */
            TX_RESTORE
    
            /* Resume the thread.  */
            _tx_thread_system_resume(thread_ptr -> tx_thread_suspended_previous);
#endif
        } 

        /* Disable interrupts.  */
        TX_DISABLE

        /* Restore previous preempt posture.  */
        _tx_thread_preempt_disable--;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Check for preemption.  */
        _tx_thread_system_preempt_check();
    }

    /* Return TX_SUCCESS.  */
    return(TX_SUCCESS);
}

