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
/**   Event Flags                                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_thread.h"
#include "tx_event_flags.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_event_flags_cleanup                             PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes event flags timeout and thread terminate    */ 
/*    actions that require the event flags data structures to be cleaned  */ 
/*    up.                                                                 */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    thread_ptr                        Pointer to suspended thread's     */ 
/*                                        control block                   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _tx_thread_system_resume          Resume thread service             */ 
/*    _tx_thread_system_ni_resume       Non-interruptable resume thread   */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _tx_thread_timeout                Thread timeout processing         */ 
/*    _tx_thread_terminate              Thread terminate processing       */ 
/*    _tx_thread_wait_abort             Thread wait abort processing      */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-12-2005     William E. Lamie         Initial Version 5.0           */ 
/*  04-02-2007     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.1    */ 
/*  12-12-2008     William E. Lamie         Modified comment(s), added    */ 
/*                                            logic to handle wait aborts */ 
/*                                            on event flag suspension    */ 
/*                                            from ISRs, added logic to   */ 
/*                                            keep suspension list in     */ 
/*                                            order, added optional       */ 
/*                                            logic for non-interruptable */ 
/*                                            operation, and made several */ 
/*                                            optimizations, resulting    */ 
/*                                            in version 5.2              */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), removed  */ 
/*                                            compound conditionals, added*/ 
/*                                            explicit check for suspend  */ 
/*                                            count, changed some counting*/ 
/*                                            variables to type UINT, and */ 
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
VOID  _tx_event_flags_cleanup(TX_THREAD  *thread_ptr)
{

#ifndef TX_NOT_INTERRUPTABLE
TX_INTERRUPT_SAVE_AREA
#endif

TX_EVENT_FLAGS_GROUP        *group_ptr;    
UINT                        suspended_count;
TX_THREAD                   *suspension_head;
TX_THREAD                   *next_thread;
TX_THREAD                   *previous_thread;


#ifndef TX_NOT_INTERRUPTABLE

    /* Disable interrupts to remove the suspended thread from the event flags group.  */
    TX_DISABLE
#endif
    
    /* Setup pointer to event flags control block.  */
    group_ptr =  (TX_EVENT_FLAGS_GROUP *) thread_ptr -> tx_thread_suspend_control_block;

    /* Check for NULL group pointer.  */
    if (group_ptr != TX_NULL)
    {
    
#ifndef TX_NOT_INTERRUPTABLE

        /* Determine if the cleanup is still required.  */
        if (thread_ptr -> tx_thread_suspend_cleanup != TX_NULL)
        {
    
            /* Is the group pointer ID valid?  */
            if (group_ptr -> tx_event_flags_group_id == TX_EVENT_FLAGS_ID)
            {
#endif

                /* Determine if there are any thread suspensions.  */
                if (group_ptr -> tx_event_flags_group_suspended_count != TX_NO_SUSPENSIONS)
                {

                    /* Yes, we still have thread suspension!  */

                    /* Clear the suspension cleanup flag.  */
                    thread_ptr -> tx_thread_suspend_cleanup =  TX_NULL;

                    /* Pickup the suspended count.  */
                    suspended_count =  group_ptr -> tx_event_flags_group_suspended_count;

                    /* Pickup the suspension head.  */
                    suspension_head =  group_ptr -> tx_event_flags_group_suspension_list;

                    /* Determine if the cleanup is being done while a set operation was interrupted.  If the 
                       suspended count is non-zero and the suspension head is NULL, the list is being processed
                       and cannot be touched from here. The suspension list removal will instead take place 
                       inside the event flag set code.  */
                    if (suspension_head != TX_NULL)
                    {

                        /* Remove the suspended thread from the list.  */

                        /* Decrement the local suspension count.  */
                        suspended_count--;
              
                        /* Store the updated suspended count.  */
                        group_ptr -> tx_event_flags_group_suspended_count =  suspended_count;

                        /* See if this is the only suspended thread on the list.  */
                        if (suspended_count == TX_NO_SUSPENSIONS)
                        {

                            /* Yes, the only suspended thread.  */
    
                            /* Update the head pointer.  */
                            group_ptr -> tx_event_flags_group_suspension_list =  TX_NULL;
                        }
                        else
                        {

                            /* At least one more thread is on the same suspension list.  */

                            /* Update the links of the adjacent threads.  */
                            next_thread =                                  thread_ptr -> tx_thread_suspended_next;
                            previous_thread =                              thread_ptr -> tx_thread_suspended_previous;
                            next_thread -> tx_thread_suspended_previous =  previous_thread;
                            previous_thread -> tx_thread_suspended_next =  next_thread;
                
                            /* Determine if we need to update the head pointer.  */
                            if (suspension_head == thread_ptr)
                            {
                
                                /* Update the list head pointer.  */
                                group_ptr -> tx_event_flags_group_suspension_list =  next_thread;
                            }
                        } 
                    }
 
                    /* Now we need to determine if this cleanup is from a terminate, timeout,
                       or from a wait abort.  */
                    if (thread_ptr -> tx_thread_state == TX_EVENT_FLAG)
                    {

                        /* Timeout condition and the thread still suspended on the event flags group.  
                           Setup return error status and resume the thread.  */

#ifdef TX_EVENT_FLAGS_ENABLE_PERFORMANCE_INFO

                        /* Increment the total timeouts counter.  */
                        _tx_event_flags_performance_timeout_count++;

                        /* Increment the number of timeouts on this event flags group.  */
                        group_ptr -> tx_event_flags_group____performance_timeout_count++;
#endif

                        /* Setup return status.  */
                        thread_ptr -> tx_thread_suspend_status =  TX_NO_EVENTS;

#ifdef TX_NOT_INTERRUPTABLE

                        /* Resume the thread!  */
                        _tx_thread_system_ni_resume(thread_ptr);
#else

                       /* Temporarily disable preemption.  */
                        _tx_thread_preempt_disable++;

                        /* Restore interrupts.  */
                        TX_RESTORE

                        /* Resume the thread!  Check for preemption even though we are executing 
                           from the system timer thread right now which normally executes at the 
                           highest priority.  */
                        _tx_thread_system_resume(thread_ptr);

                        /* Disable interrupts.  */
                        TX_DISABLE
#endif
                    }   
                }
#ifndef TX_NOT_INTERRUPTABLE
            }
        }
#endif
    }

#ifndef TX_NOT_INTERRUPTABLE

    /* Restore interrupts.  */
    TX_RESTORE
#endif
}

