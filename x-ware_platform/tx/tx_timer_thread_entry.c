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
/**   Timer                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_timer.h"
#include "tx_thread.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_timer_thread_entry                              PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function manages thread and application timer expirations.     */ 
/*    Actually, from this thread's point of view, there is no difference. */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    timer_thread_input                Used just for verification        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    Timer Expiration Function                                           */ 
/*    _tx_thread_system_suspend         Thread suspension                 */ 
/*    _tx_thread_system_ni_suspend      Non-interruptable suspend thread  */ 
/*    _tx_timer_system_activate         Timer reactivate processing       */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    ThreadX Scheduler                                                   */ 
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
/*                                            and made several            */ 
/*                                            optimizations, resulting    */ 
/*                                            in version 5.2              */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), added    */ 
/*                                            logic to explicitly check   */ 
/*                                            for valid pointer, added    */ 
/*                                            explicit check for expired  */ 
/*                                            flag, moved while-forever   */ 
/*                                            to top of the loop, and     */ 
/*                                            added safety critical       */ 
/*                                            exception if control        */ 
/*                                            returns after the thread's  */ 
/*                                            while-forever processing    */ 
/*                                            loop, resulting             */ 
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
VOID  _tx_timer_thread_entry(ULONG timer_thread_input)
{
#ifndef TX_TIMER_PROCESS_IN_ISR

TX_INTERRUPT_SAVE_AREA

TX_TIMER_INTERNAL           *expired_timers;            
TX_TIMER_INTERNAL           *reactivate_timer;          
TX_TIMER_INTERNAL           *next_timer;
TX_TIMER_INTERNAL           *previous_timer;
TX_TIMER_INTERNAL           *current_timer;              
VOID                        (*timeout_function)(ULONG id); 
ULONG                       timeout_param =  ((ULONG) 0); 
TX_THREAD                   *thread_ptr;
#ifdef TX_REACTIVATE_INLINE
TX_TIMER_INTERNAL           **timer_list;               /* Timer list pointer           */
UINT                        expiration_time;            /* Value used for pointer offset*/
ULONG                       delta;
#endif
#ifdef TX_TIMER_ENABLE_PERFORMANCE_INFO
TX_TIMER                    *timer_ptr;
#endif


    /* Make sure the timer input is correct.  This also gets rid of the
       silly compiler warnings.  */
    if (timer_thread_input == TX_TIMER_ID)
    {
    
        /* Yes, valid thread entry, proceed...  */

        /* Set the reactivate_timer to NULL.  */
        reactivate_timer =  TX_NULL;

        /* Now go into an infinite loop to process timer expirations.  */
        while (TX_LOOP_FOREVER)
        {

            /* First, move the current list pointer and clear the timer 
               expired value.  This allows the interrupt handling portion
               to continue looking for timer expirations.  */
            TX_DISABLE

            /* Save the current timer expiration list pointer.  */
            expired_timers =  *_tx_timer_current_ptr;

            /* Modify the head pointer in the first timer in the list, if there
               is one!  */
            if (expired_timers != TX_NULL)
            {
        
                expired_timers -> tx_timer_internal_list_head =  &expired_timers;
            }

            /* Set the current list pointer to NULL.  */
            *_tx_timer_current_ptr =  TX_NULL;

            /* Move the current pointer up one timer entry wrap if we get to 
               the end of the list.  */
            _tx_timer_current_ptr =  TX_TIMER_POINTER_ADD(_tx_timer_current_ptr, 1);
            if (_tx_timer_current_ptr == _tx_timer_list_end)
            {
        
                _tx_timer_current_ptr =  _tx_timer_list_start;
            }

            /* Clear the expired flag.  */
            _tx_timer_expired =  TX_FALSE;

            /* Restore interrupts temporarily.  */
            TX_RESTORE

            /* Disable interrupts again.  */
            TX_DISABLE

            /* Next, process the expiration of the associated timers at this
               time slot.  */
            while (expired_timers != TX_NULL)
            {

                /* Something is on the list.  Remove it and process the expiration.  */
                current_timer =  expired_timers;

                /* Pickup the next timer.  */
                next_timer =  expired_timers -> tx_timer_internal_active_next;
            
                /* Determine if this is the only timer.  */
                if (current_timer == next_timer)
                {

                    /* Yes, this is the only timer in the list.  */

                    /* Set the head pointer to NULL.  */
                    expired_timers =  TX_NULL;
                }
                else
                {

                    /* No, not the only expired timer.  */
            
                    /* Remove this timer from the expired list.  */
                    previous_timer =                                   current_timer -> tx_timer_internal_active_previous;
                    next_timer -> tx_timer_internal_active_previous =  previous_timer;
                    previous_timer -> tx_timer_internal_active_next =  next_timer;

                    /* Modify the next timer's list head to point at the current list head.  */
                    next_timer -> tx_timer_internal_list_head =  &expired_timers;

                    /* Set the list head pointer.  */
                    expired_timers =  next_timer;
                }

                /* In any case, the timer is now off of the expired list.  */

                /* Determine if the timer has expired or if it is just a really 
                   big timer that needs to be placed in the list again.  */
                if (current_timer -> tx_timer_internal_remaining_ticks > TX_TIMER_ENTRIES)
                {

                    /* Timer is bigger than the timer entries and must be
                       rescheduled.  */

#ifdef TX_TIMER_ENABLE_PERFORMANCE_INFO

                    /* Increment the total expiration adjustments counter.  */
                    _tx_timer_performance__expiration_adjust_count++;

                    /* Determine if this is an application timer.  */
                    if (current_timer -> tx_timer_internal_timeout_function != &_tx_thread_timeout)
                    {
            
                        /* Derive the application timer pointer.  */
                    
                        /* Pickup the application timer pointer.  */
                        TX_USER_TIMER_POINTER_GET(current_timer, timer_ptr)

                        /* Increment the number of expiration adjustments on this timer.  */
                        if (timer_ptr -> tx_timer_id == TX_TIMER_ID)
                        {
                    
                            timer_ptr -> tx_timer_performance__expiration_adjust_count++;
                        }
                    }
#endif

                    /* Decrement the remaining ticks of the timer.  */
                    current_timer -> tx_timer_internal_remaining_ticks =  
                            current_timer -> tx_timer_internal_remaining_ticks - TX_TIMER_ENTRIES;
                
                    /* Set the timeout function to NULL in order to bypass the
                       expiration.  */
                    timeout_function =  TX_NULL;

                    /* Make the timer appear that it is still active while interrupts
                       are enabled.  This will permit proper processing of a timer
                       deactivate from an ISR.  */
                    current_timer -> tx_timer_internal_list_head =    &reactivate_timer;
                    current_timer -> tx_timer_internal_active_next =  current_timer;
                }
                else
                {

                    /* Timer did expire.  */

#ifdef TX_TIMER_ENABLE_PERFORMANCE_INFO

                    /* Increment the total expirations counter.  */
                    _tx_timer_performance_expiration_count++;

                    /* Determine if this is an application timer.  */
                    if (current_timer -> tx_timer_internal_timeout_function != &_tx_thread_timeout)
                    {
            
                        /* Derive the application timer pointer.  */
                    
                        /* Pickup the application timer pointer.  */
                        TX_USER_TIMER_POINTER_GET(current_timer, timer_ptr)

                        /* Increment the number of expirations on this timer.  */
                        if (timer_ptr -> tx_timer_id == TX_TIMER_ID)
                        {
                    
                            timer_ptr -> tx_timer_performance_expiration_count++;
                        }
                    }
#endif
                  
                    /* Copy the calling function and ID into local variables before interrupts 
                       are re-enabled.  */
                    timeout_function =  current_timer -> tx_timer_internal_timeout_function;
                    timeout_param =     current_timer -> tx_timer_internal_timeout_param;

                    /* Copy the reinitialize ticks into the remaining ticks.  */
                    current_timer -> tx_timer_internal_remaining_ticks =  current_timer -> tx_timer_internal_re_initialize_ticks;

                    /* Determine if the timer should be reactivated.  */
                    if (current_timer -> tx_timer_internal_remaining_ticks != ((ULONG) 0))
                    {

                        /* Make the timer appear that it is still active while processing
                           the expiration routine and with interrupts enabled.  This will 
                           permit proper processing of a timer deactivate from both the
                           expiration routine and an ISR.  */
                        current_timer -> tx_timer_internal_list_head =    &reactivate_timer;
                        current_timer -> tx_timer_internal_active_next =  current_timer;
                    }
                    else
                    {

                        /* Set the list pointer of this timer to NULL.  This is used to indicate
                           the timer is no longer active.  */
                        current_timer -> tx_timer_internal_list_head =  TX_NULL;
                    }
                }

                /* Restore interrupts for timer expiration call.  */
                TX_RESTORE

                /* Call the timer-expiration function, if non-NULL.  */
                if (timeout_function != TX_NULL)
                {
            
                    (timeout_function) (timeout_param);
                }

                /* Lockout interrupts again.  */
                TX_DISABLE

                /* Determine if the timer needs to be reactivated.  */
                if (current_timer -> tx_timer_internal_list_head == &reactivate_timer)
                {

                    /* Reactivate the timer.  */

#ifdef TX_TIMER_ENABLE_PERFORMANCE_INFO

                    /* Determine if this timer expired.  */
                    if (timeout_function != TX_NULL)
                    {

                        /* Increment the total reactivations counter.  */
                        _tx_timer_performance_reactivate_count++;

                        /* Determine if this is an application timer.  */
                        if (current_timer -> tx_timer_internal_timeout_function != &_tx_thread_timeout)
                        {
            
                            /* Derive the application timer pointer.  */
                    
                            /* Pickup the application timer pointer.  */
                            TX_USER_TIMER_POINTER_GET(current_timer, timer_ptr)

                            /* Increment the number of expirations on this timer.  */
                            if (timer_ptr -> tx_timer_id == TX_TIMER_ID)
                            {
                              
                                timer_ptr -> tx_timer_performance_reactivate_count++;
                            }
                        }
                    }
#endif

#ifdef TX_REACTIVATE_INLINE

                    /* Calculate the amount of time remaining for the timer.  */
                    if (current_timer -> tx_timer_internal_remaining_ticks > TX_TIMER_ENTRIES)
                    {

                        /* Set expiration time to the maximum number of entries.  */
                        expiration_time =  TX_TIMER_ENTRIES - ((UINT) 1);
                    }
                    else
                    {

                        /* Timer value fits in the timer entries.  */

                        /* Set the expiration time.  */
                        expiration_time =  ((UINT) current_timer -> tx_timer_internal_remaining_ticks) - ((UINT) 1);
                    }

                    /* At this point, we are ready to put the timer back on one of
                       the timer lists.  */
    
                    /* Calculate the proper place for the timer.  */
                    timer_list =  TX_TIMER_POINTER_ADD(_tx_timer_current_ptr, expiration_time);
                    if (timer_list >= _tx_timer_list_end)
                    {

                        /* Wrap from the beginning of the list.  */
                        delta =  TX_TIMER_POINTER_DIF(timer_list, _tx_timer_list_end);
                        timer_list =  TX_TIMER_POINTER_ADD(_tx_timer_list_start, delta);
                    }

                    /* Now put the timer on this list.  */
                    if ((*timer_list) == TX_NULL)
                    {
                
                        /* This list is NULL, just put the new timer on it.  */

                        /* Setup the links in this timer.  */
                        current_timer -> tx_timer_internal_active_next =      current_timer;
                        current_timer -> tx_timer_internal_active_previous =  current_timer;

                        /* Setup the list head pointer.  */
                        *timer_list =  current_timer;
                    }
                    else
                    {

                        /* This list is not NULL, add current timer to the end. */
                        next_timer =                                          *timer_list;
                        previous_timer =                                      next_timer -> tx_timer_internal_active_previous;
                        previous_timer -> tx_timer_internal_active_next =     current_timer;
                        next_timer -> tx_timer_internal_active_previous =     current_timer;
                        current_timer -> tx_timer_internal_active_next =      next_timer;
                        current_timer -> tx_timer_internal_active_previous =  previous_timer;
                    }

                    /* Setup list head pointer.  */
                    current_timer -> tx_timer_internal_list_head =  timer_list;
#else

                    /* Reactivate through the timer activate function.  */

                    /* Clear the list head for the timer activate call.  */
                    current_timer -> tx_timer_internal_list_head = TX_NULL;

                    /* Restore interrupts.  */
                    TX_RESTORE

                    /* Activate the current timer.  */
                    _tx_timer_system_activate(current_timer);

                    /* Disable interrupts.  */
                    TX_DISABLE
#endif
                }

                /* Restore interrupts.  */
                TX_RESTORE

                /* Lockout interrupts again.  */
                TX_DISABLE
            }

            /* Finally, suspend this thread and wait for the next expiration.  */

            /* Determine if another expiration took place while we were in this
               thread.  If so, process another expiration.  */
            if (_tx_timer_expired == TX_FALSE)
            {

                /* Otherwise, no timer expiration, so suspend the thread.  */

                /* Build pointer to the timer thread.  */
                thread_ptr =  &_tx_timer_thread;

                /* Set the status to suspending, in order to indicate the 
                   suspension is in progress.  */
                thread_ptr -> tx_thread_state =  TX_SUSPENDED;

#ifdef TX_NOT_INTERRUPTABLE

                /* Call actual non-interruptable thread suspension routine.  */
                _tx_thread_system_ni_suspend(thread_ptr, ((ULONG) 0));

                /* Restore interrupts.  */
                TX_RESTORE
#else

                /* Set the suspending flag. */
                thread_ptr -> tx_thread_suspending =  TX_TRUE;

                /* Increment the preempt disable count prior to suspending.  */
                _tx_thread_preempt_disable++;

                /* Restore interrupts.  */
                TX_RESTORE

                /* Call actual thread suspension routine.  */
                _tx_thread_system_suspend(thread_ptr);
#endif
            }
            else
            {

                /* Restore interrupts.  */
                TX_RESTORE
            }
        } 
    }

#ifdef TX_SAFETY_CRITICAL

    /* If we ever get here, raise safety critical exception.  */
    TX_SAFETY_CRITICAL_EXCEPTION(__FILE__, __LINE__, 0);
#endif

#endif
}

