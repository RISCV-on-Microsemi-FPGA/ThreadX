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
/*    _tx_semaphore_put                                   PORTABLE C      */ 
/*                                                           5.7          */  
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function puts an instance into the specified counting          */ 
/*    semaphore.                                                          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    semaphore_ptr                     Pointer to semaphore control block*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    TX_SUCCESS                        Success completion status         */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
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
/*  12-12-2009     William E. Lamie         Modified comment(s), changed  */ 
/*                                            some counting variables to  */ 
/*                                            type UINT, merged event     */ 
/*                                            logging support, and added  */ 
/*                                            logic to explicitly check   */ 
/*                                            for valid pointer, resulting*/ 
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
UINT  _tx_semaphore_put(TX_SEMAPHORE *semaphore_ptr)
{

TX_INTERRUPT_SAVE_AREA

#ifndef TX_DISABLE_NOTIFY_CALLBACKS
VOID            (*semaphore_put_notify)(struct TX_SEMAPHORE_STRUCT *notify_semaphore_ptr);
#endif

TX_THREAD       *thread_ptr;            
UINT            suspended_count;
TX_THREAD       *next_thread;
TX_THREAD       *previous_thread;


    /* Disable interrupts to put an instance back to the semaphore.  */
    TX_DISABLE

#ifdef TX_SEMAPHORE_ENABLE_PERFORMANCE_INFO

    /* Increment the total semaphore put counter.  */
    _tx_semaphore_performance_put_count++;

    /* Increment the number of puts on this semaphore.  */
    semaphore_ptr -> tx_semaphore_performance_put_count++;
#endif

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_SEMAPHORE_PUT, semaphore_ptr, semaphore_ptr -> tx_semaphore_count, semaphore_ptr -> tx_semaphore_suspended_count, TX_POINTER_TO_ULONG_CONVERT(&thread_ptr), TX_TRACE_SEMAPHORE_EVENTS)

    /* Log this kernel call.  */
    TX_EL_SEMAPHORE_PUT_INSERT

    /* Pickup the number of suspended threads.  */
    suspended_count =  semaphore_ptr -> tx_semaphore_suspended_count;

    /* Determine if there are any threads suspended on the semaphore.  */
    if (suspended_count == TX_NO_SUSPENSIONS)
    {

        /* Increment the semaphore count.  */
        semaphore_ptr -> tx_semaphore_count++;

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

        /* Pickup the application notify function.  */
        semaphore_put_notify =  semaphore_ptr -> tx_semaphore_put_notify;
#endif

        /* Restore interrupts.  */
        TX_RESTORE

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

        /* Determine if notification is required.  */
        if (semaphore_put_notify != TX_NULL)
        {

            /* Yes, call the appropriate notify callback function.  */
            (semaphore_put_notify)(semaphore_ptr);
        }
#endif
    }
    else
    {

        /* A thread is suspended on this semaphore.  */
        
        /* Pickup the pointer to the first suspended thread.  */
        thread_ptr =  semaphore_ptr -> tx_semaphore_suspension_list;

        /* Remove the suspended thread from the list.  */

        /* See if this is the only suspended thread on the list.  */
        suspended_count--;
        if (suspended_count == TX_NO_SUSPENSIONS)
        {

            /* Yes, the only suspended thread.  */

            /* Update the head pointer.  */
            semaphore_ptr -> tx_semaphore_suspension_list =  TX_NULL;
        }
        else
        {

            /* At least one more thread is on the same expiration list.  */

            /* Update the list head pointer.  */
            next_thread =                                     thread_ptr -> tx_thread_suspended_next;
            semaphore_ptr -> tx_semaphore_suspension_list =   next_thread;

            /* Update the links of the adjacent threads.  */
            previous_thread =                               thread_ptr -> tx_thread_suspended_previous;
            next_thread -> tx_thread_suspended_previous =   previous_thread;
            previous_thread -> tx_thread_suspended_next =   next_thread;
        } 
 
        /* Decrement the suspension count.  */
        semaphore_ptr -> tx_semaphore_suspended_count =  suspended_count;

        /* Prepare for resumption of the first thread.  */

        /* Clear cleanup routine to avoid timeout.  */
        thread_ptr -> tx_thread_suspend_cleanup =  TX_NULL;

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

        /* Pickup the application notify function.  */
        semaphore_put_notify =  semaphore_ptr -> tx_semaphore_put_notify;
#endif

        /* Put return status into the thread control block.  */
        thread_ptr -> tx_thread_suspend_status =  TX_SUCCESS;        

#ifdef TX_NOT_INTERRUPTABLE

        /* Resume the thread!  */
        _tx_thread_system_ni_resume(thread_ptr);

        /* Restore interrupts.  */
        TX_RESTORE
#else

        /* Temporarily disable preemption.  */
        _tx_thread_preempt_disable++;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Resume thread.  */
        _tx_thread_system_resume(thread_ptr);
#endif

#ifndef TX_DISABLE_NOTIFY_CALLBACKS

        /* Determine if notification is required.  */
        if (semaphore_put_notify != TX_NULL)
        {

            /* Yes, call the appropriate notify callback function.  */
            (semaphore_put_notify)(semaphore_ptr);
        }
#endif
    }

    /* Return successful completion.  */
    return(TX_SUCCESS);
}

