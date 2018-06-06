/**************************************************************************/ 
/*                                                                        */ 
/*            Copyright (c) 1996-2014 by Express Logic Inc.               */ 
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
/**   Mutex                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"
#include "tx_thread.h"
#include "tx_mutex.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_mutex_put                                       PORTABLE C      */ 
/*                                                           5.7          */  
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function puts back an instance of the specified mutex.         */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mutex_ptr                         Pointer to mutex control block    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    TX_SUCCESS                        Success completion status         */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _tx_thread_system_preempt_check   Check for preemption              */ 
/*    _tx_thread_system_resume          Resume thread service             */ 
/*    _tx_thread_system_ni_resume       Non-interruptable resume thread   */ 
/*    _tx_mutex_priority_change         Restore previous thread priority  */ 
/*    _tx_mutex_prioritize              Prioritize the mutex suspension   */ 
/*    _tx_mutex_thread_release          Release all thread's mutexes      */ 
/*    _tx_mutex_delete                  Release ownership upon mutex      */ 
/*                                        deletion                        */ 
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
/*                                            macro to get current thread,*/ 
/*                                            optimized the normal        */ 
/*                                            no priority-inheritance     */ 
/*                                            path, added filter option   */ 
/*                                            to trace insert, added      */ 
/*                                            optional logic for          */ 
/*                                            non-interruptable operation,*/ 
/*                                            and made several other      */ 
/*                                            optimizations, resulting    */ 
/*                                            in version 5.2              */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), removed  */ 
/*                                            compound conditionals, added*/ 
/*                                            explicit value checking,    */ 
/*                                            added logic to explicitly   */ 
/*                                            check for valid pointer,    */ 
/*                                            changed priority-inheritance*/ 
/*                                            logic to distinguish user   */ 
/*                                            priority changes from mutex */ 
/*                                            priority-inheritance,       */ 
/*                                            changed some counting       */ 
/*                                            variables to type UINT,     */ 
/*                                            merged event logging        */ 
/*                                            support, and added return   */ 
/*                                            value check for calls to    */ 
/*                                            prioritize, resulting in    */ 
/*                                            version 5.4                 */ 
/*  07-15-2011     William E. Lamie         Modified comment(s), removed  */ 
/*                                            saving original threshold,  */ 
/*                                            removed unnecessary         */ 
/*                                            parameter to mutex priority */ 
/*                                            change, and added logic to  */ 
/*                                            check for necessary priority*/ 
/*                                            changes, resulting in       */ 
/*                                            version 5.5                 */ 
/*  11-01-2012     William E. Lamie         Modified comment(s), added    */ 
/*                                            logic to allow mutex put to */ 
/*                                            be called from another      */ 
/*                                            thread for mutex cleanup,   */ 
/*                                            and adjusted priority       */ 
/*                                            restoration search to start */ 
/*                                            at user priority,           */ 
/*                                            resulting in version 5.6    */ 
/*  05-01-2015     William E. Lamie         Modified comment(s), modified */ 
/*                                            code for MISRA compliance,  */ 
/*                                            modified logic to remove    */ 
/*                                            non-priority inheritance    */ 
/*                                            mutex from owned mutex      */ 
/*                                            list, and added logic to    */ 
/*                                            handle releasing a mutex    */ 
/*                                            during initialization,      */ 
/*                                            resulting version 5.7       */ 
/*                                                                        */ 
/**************************************************************************/ 
UINT  _tx_mutex_put(TX_MUTEX *mutex_ptr)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD       *thread_ptr;            
TX_THREAD       *old_owner;             
UINT            old_priority;            
UINT            status;
TX_MUTEX        *next_mutex;            
TX_MUTEX        *previous_mutex;
UINT            owned_count;
UINT            suspended_count;
TX_THREAD       *current_thread;
TX_THREAD       *next_thread;
TX_THREAD       *previous_thread;
TX_THREAD       *suspended_thread;


    /* Setup status to indicate the processing is not complete.  */
    status =  TX_NOT_DONE;

    /* Disable interrupts to put an instance back to the mutex.  */
    TX_DISABLE

#ifdef TX_MUTEX_ENABLE_PERFORMANCE_INFO

    /* Increment the total mutex put counter.  */
    _tx_mutex_performance_put_count++;

    /* Increment the number of attempts to put this mutex.  */
    mutex_ptr -> tx_mutex_performance_put_count++;
#endif

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_MUTEX_PUT, mutex_ptr, TX_POINTER_TO_ULONG_CONVERT(mutex_ptr -> tx_mutex_owner), mutex_ptr -> tx_mutex_ownership_count, TX_POINTER_TO_ULONG_CONVERT(&old_priority), TX_TRACE_MUTEX_EVENTS)

    /* Log this kernel call.  */
    TX_EL_MUTEX_PUT_INSERT

    /* Determine if this mutex is owned.  */
    if (mutex_ptr -> tx_mutex_ownership_count != ((UINT) 0))
    {

        /* Pickup the owning thread pointer.  */
        thread_ptr =  mutex_ptr -> tx_mutex_owner;

        /* Pickup thread pointer.  */
        TX_THREAD_GET_CURRENT(current_thread)

        /* Check to see if the mutex is owned by the calling thread.  */
        if (mutex_ptr -> tx_mutex_owner != current_thread)
        {
        
            /* Determine if the preempt disable flag is set, indicating that 
               the caller is not the application but from ThreadX. In such
               cases, the thread mutex owner does not need to match.  */
            if (_tx_thread_preempt_disable == ((UINT) 0))
            {

                /* Invalid mutex release.  */

                /* Restore interrupts.  */
                TX_RESTORE

                /* Caller does not own the mutex.  */
                status =  TX_NOT_OWNED;
            }
        }
        
        /* Determine if we should continue.  */
        if (status == TX_NOT_DONE)
        {
    
            /* Decrement the mutex ownership count.  */
            mutex_ptr -> tx_mutex_ownership_count--;

            /* Determine if the mutex is still owned by the current thread.  */
            if (mutex_ptr -> tx_mutex_ownership_count != ((UINT) 0))
            {

                /* Restore interrupts.  */
                TX_RESTORE

                /* Mutex is still owned, just return successful status.  */
                status =  TX_SUCCESS;
            }
            else
            {        

                /* Check for a NULL thread pointer, which can only happen during initialization.   */
                if (thread_ptr == TX_NULL)
                {

                    /* Restore interrupts.  */
                    TX_RESTORE

                    /* Mutex is now available, return successful status.  */
                    status =  TX_SUCCESS;
                }
                else
                {

                    /* The mutex is now available.   */
            
                    /* Remove this mutex from the owned mutex list.  */
                    
                    /* Decrement the ownership count.  */
                    thread_ptr -> tx_thread_owned_mutex_count--;

                    /* Determine if this mutex was the only one on the list.  */
                    if (thread_ptr -> tx_thread_owned_mutex_count == ((UINT) 0))
                    {

                        /* Yes, the list is empty.  Simply set the head pointer to NULL.  */
                        thread_ptr -> tx_thread_owned_mutex_list =  TX_NULL;
                    }
                    else
                    {

                        /* No, there are more mutexes on the list.  */

                        /* Link-up the neighbors.  */
                        next_mutex =                             mutex_ptr -> tx_mutex_owned_next;
                        previous_mutex =                         mutex_ptr -> tx_mutex_owned_previous;
                        next_mutex -> tx_mutex_owned_previous =  previous_mutex;
                        previous_mutex -> tx_mutex_owned_next =  next_mutex;

                        /* See if we have to update the created list head pointer.  */
                        if (thread_ptr -> tx_thread_owned_mutex_list == mutex_ptr)
                        {

                            /* Yes, move the head pointer to the next link. */
                            thread_ptr -> tx_thread_owned_mutex_list =  next_mutex; 
                        }
                    }

                    /* Determine if the simple, non-suspension, non-priority inheritance case is present.  */ 
                    if (mutex_ptr -> tx_mutex_suspension_list == TX_NULL)
                    {
                    
                        /* Is this a priority inheritance mutex?  */
                        if (mutex_ptr -> tx_mutex_inherit == TX_FALSE)
                        {

                            /* Yes, we are done - set the mutex owner to NULL.   */
                            mutex_ptr -> tx_mutex_owner =  TX_NULL;
                            
                            /* Restore interrupts.  */
                            TX_RESTORE

                            /* Mutex is now available, return successful status.  */
                            status =  TX_SUCCESS;
                        }
                    }
                     
                    /* Determine if the processing is complete.  */
                    if (status == TX_NOT_DONE)
                    {
   
                        /* Initialize original owner and thread priority.  */
                        old_owner =      TX_NULL;
                        old_priority =   thread_ptr -> tx_thread_user_priority;

                        /* Does this mutex support priority inheritance?  */
                        if (mutex_ptr -> tx_mutex_inherit == TX_TRUE)
                        {

#ifndef TX_NOT_INTERRUPTABLE

                            /* Temporarily disable preemption.  */
                            _tx_thread_preempt_disable++;

                            /* Restore interrupts.  */
                            TX_RESTORE
#endif

                            /* Search the owned mutexes for this thread to determine the highest priority for this 
                               former mutex owner to return to.  */
                            next_mutex =  thread_ptr -> tx_thread_owned_mutex_list;
                            while (next_mutex != TX_NULL)
                            {

                                /* Does this mutex support priority inheritance?  */
                                if (next_mutex -> tx_mutex_inherit == TX_TRUE)
                                {
                            
                                    /* Determine if highest priority field of the mutex is higher than the priority to 
                                       restore.  */
                                    if (next_mutex -> tx_mutex_highest_priority_waiting < old_priority)
                                    {

                                        /* Use this priority to return releasing thread to.  */
                                        old_priority =   next_mutex -> tx_mutex_highest_priority_waiting;
                                    }
                                }

                                /* Move mutex pointer to the next mutex in the list.  */
                                next_mutex =  next_mutex -> tx_mutex_owned_next;

                                /* Are we at the end of the list?  */
                                if (next_mutex == thread_ptr -> tx_thread_owned_mutex_list)
                                {
                            
                                    /* Yes, set the next mutex to NULL.  */
                                    next_mutex =  TX_NULL;
                                }
                            }

#ifndef TX_NOT_INTERRUPTABLE

                            /* Disable interrupts.  */
                            TX_DISABLE

                            /* Undo the temporarily preemption disable.  */
                            _tx_thread_preempt_disable--;
#endif
                        }

                        /* Determine if priority inheritance is in effect and there are one or more
                           threads suspended on the mutex.  */
                        if (mutex_ptr -> tx_mutex_suspended_count > ((UINT) 1))
                        {

                            /* Is priority inheritance in effect?  */
                            if (mutex_ptr -> tx_mutex_inherit == TX_TRUE)
                            {

                                /* Yes, this code is simply to ensure the highest priority thread is positioned
                                   at the front of the suspension list.  */

#ifndef TX_NOT_INTERRUPTABLE

                                /* Temporarily disable preemption.  */
                                _tx_thread_preempt_disable++;

                                /* Restore interrupts.  */
                                TX_RESTORE
#endif

                                /* Call the mutex prioritize processing to ensure the 
                                   highest priority thread is resumed.  */
#ifdef TX_MISRA_ENABLE
                                do
                                {
                                    status =  _tx_mutex_prioritize(mutex_ptr);
                                } while (status != TX_SUCCESS);
#else
                                _tx_mutex_prioritize(mutex_ptr);
#endif

                                /* At this point, the highest priority thread is at the
                                   front of the suspension list.  */

#ifndef TX_NOT_INTERRUPTABLE

                                /* Disable interrupts.  */
                                TX_DISABLE

                                /* Back off the preemption disable.  */
                                _tx_thread_preempt_disable--;
#endif
                            }
                        }

                        /* Now determine if there are any threads still waiting on the mutex.  */
                        if (mutex_ptr -> tx_mutex_suspension_list == TX_NULL)
                        {           

                            /* No, there are no longer any threads waiting on the mutex.  */

#ifndef TX_NOT_INTERRUPTABLE

                            /* Temporarily disable preemption.  */
                            _tx_thread_preempt_disable++;
 
                            /* Restore interrupts.  */
                            TX_RESTORE
#endif

                            /* Mutex is not owned, but it is possible that a thread that 
                               caused a priority inheritance to occur is no longer waiting
                               on the mutex.  */
                            if (mutex_ptr -> tx_mutex_inherit == TX_TRUE) 
                            {

                                /* Setup the highest priority waiting thread.  */
                                mutex_ptr -> tx_mutex_highest_priority_waiting =  (UINT) TX_MAX_PRIORITIES;
  
                                /* Determine if we need to restore priority.  */
                                if ((mutex_ptr -> tx_mutex_owner) -> tx_thread_priority != old_priority)
                                {
                      
                                    /* Yes, restore the priority of thread.  */
                                    _tx_mutex_priority_change(mutex_ptr -> tx_mutex_owner, old_priority);
                                }
                            }

#ifndef TX_NOT_INTERRUPTABLE

                            /* Disable interrupts again.  */
                            TX_DISABLE

                            /* Back off the preemption disable.  */
                            _tx_thread_preempt_disable--;
#endif

                            /* Clear the owner flag.  */
                            if (mutex_ptr -> tx_mutex_ownership_count == ((UINT) 0))
                            {

                                /* Set the mutex owner to NULL.  */
                                mutex_ptr -> tx_mutex_owner =  TX_NULL;
                            }

                            /* Restore interrupts.  */
                            TX_RESTORE

                            /* Check for preemption.  */
                            _tx_thread_system_preempt_check();

                            /* Set status to success.  */
                            status =  TX_SUCCESS;
                        }
                        else
                        {

                            /* Pickup the thread at the front of the suspension list.  */
                            thread_ptr =  mutex_ptr -> tx_mutex_suspension_list;

                            /* Save the previous ownership information, if inheritance is
                               in effect.  */
                            if (mutex_ptr -> tx_mutex_inherit == TX_TRUE)
                            {

                                /* Remember the old mutex owner.  */
                                old_owner =  mutex_ptr -> tx_mutex_owner;
        
                                /* Setup owner thread priority information.  */
                                mutex_ptr -> tx_mutex_original_priority =   thread_ptr -> tx_thread_priority;

                                /* Setup the highest priority waiting thread.  */
                                mutex_ptr -> tx_mutex_highest_priority_waiting =  (UINT) TX_MAX_PRIORITIES;
                            }
 
                            /* Determine how many mutexes are owned by this thread.  */
                            owned_count =  thread_ptr -> tx_thread_owned_mutex_count;

                            /* Determine if this thread owns any other mutexes that have priority inheritance.  */
                            if (owned_count == ((UINT) 0))
                            {

                                /* The owned mutex list is empty.  Add mutex to empty list.  */
                                thread_ptr -> tx_thread_owned_mutex_list =     mutex_ptr;
                                mutex_ptr -> tx_mutex_owned_next =             mutex_ptr;
                                mutex_ptr -> tx_mutex_owned_previous =         mutex_ptr;
                            }
                            else
                            {

                                /* Non-empty list. Link up the mutex.  */

                                /* Pickup tail pointer.  */
                                next_mutex =                            thread_ptr -> tx_thread_owned_mutex_list;
                                previous_mutex =                        next_mutex -> tx_mutex_owned_previous;

                                /* Place the owned mutex in the list.  */
                                next_mutex -> tx_mutex_owned_previous =  mutex_ptr;
                                previous_mutex -> tx_mutex_owned_next =  mutex_ptr;

                                /* Setup this mutex's next and previous created links.  */
                                mutex_ptr -> tx_mutex_owned_previous =   previous_mutex;
                                mutex_ptr -> tx_mutex_owned_next =       next_mutex;
                            }

                            /* Increment the number of mutexes owned counter.  */
                            thread_ptr -> tx_thread_owned_mutex_count =  owned_count + ((UINT) 1);

                            /* Mark the Mutex as owned and fill in the corresponding information.  */
                            mutex_ptr -> tx_mutex_ownership_count =  (UINT) 1;
                            mutex_ptr -> tx_mutex_owner =            thread_ptr;

                            /* Remove the suspended thread from the list.  */

                            /* Decrement the suspension count.  */
                            mutex_ptr -> tx_mutex_suspended_count--;
                
                            /* Pickup the suspended count.  */
                            suspended_count =  mutex_ptr -> tx_mutex_suspended_count;

                            /* See if this is the only suspended thread on the list.  */
                            if (suspended_count == TX_NO_SUSPENSIONS)
                            {

                                /* Yes, the only suspended thread.  */
    
                                /* Update the head pointer.  */
                                mutex_ptr -> tx_mutex_suspension_list =  TX_NULL;
                            }
                            else
                            {

                                /* At least one more thread is on the same expiration list.  */

                                /* Update the list head pointer.  */
                                next_thread =                                  thread_ptr -> tx_thread_suspended_next;
                                mutex_ptr -> tx_mutex_suspension_list =        next_thread;

                                /* Update the links of the adjacent threads.  */
                                previous_thread =                              thread_ptr -> tx_thread_suspended_previous;
                                next_thread -> tx_thread_suspended_previous =  previous_thread;
                                previous_thread -> tx_thread_suspended_next =  next_thread;
                            } 
 
                            /* Prepare for resumption of the first thread.  */

                            /* Clear cleanup routine to avoid timeout.  */
                            thread_ptr -> tx_thread_suspend_cleanup =  TX_NULL;

                            /* Put return status into the thread control block.  */
                            thread_ptr -> tx_thread_suspend_status =  TX_SUCCESS;        

#ifdef TX_NOT_INTERRUPTABLE

                            /* Determine if priority inheritance is enabled for this mutex.  */
                            if (mutex_ptr -> tx_mutex_inherit == TX_TRUE)
                            {

                                /* Yes, priority inheritance is requested.  */

                                /* Determine if there are any more threads still suspended on the mutex.  */
                                if (mutex_ptr -> tx_mutex_suspended_count != ((ULONG) 0))
                                {

                                    /* Determine if there are more than one thread suspended on the mutex.  */
                                    if (mutex_ptr -> tx_mutex_suspended_count > ((ULONG) 1))
                                    {

                                        /* If so, prioritize the list so the highest priority thread is placed at the
                                           front of the suspension list.  */
#ifdef TX_MISRA_ENABLE
                                        do
                                        {
                                            status =  _tx_mutex_prioritize(mutex_ptr);
                                        } while (status != TX_SUCCESS);
#else
                                        _tx_mutex_prioritize(mutex_ptr);
#endif
                                    }
    
                                    /* Now, pickup the list head and set the priority.  */

                                    /* Determine if there still are threads suspended for this mutex.  */
                                    suspended_thread =  mutex_ptr -> tx_mutex_suspension_list;
                                    if (suspended_thread != TX_NULL)
                                    {

                                        /* Setup the highest priority thread waiting on this mutex.  */
                                        mutex_ptr -> tx_mutex_highest_priority_waiting =  suspended_thread -> tx_thread_priority;
                                    }
                                }

                                /* Restore previous priority needs to be restored after priority
                                   inheritance.  */
                                if (old_owner != TX_NULL)
                                {
                    
                                    /* Determine if we need to restore priority.  */
                                    if (old_owner -> tx_thread_priority != old_priority)
                                    {
    
                                        /* Restore priority of thread.  */
                                        _tx_mutex_priority_change(old_owner, old_priority);
                                    }
                                }
                            }

                            /* Resume the thread!  */
                            _tx_thread_system_ni_resume(thread_ptr);

                            /* Restore interrupts.  */
                            TX_RESTORE
#else

                            /* Temporarily disable preemption.  */
                            _tx_thread_preempt_disable++;

                            /* Restore interrupts.  */
                            TX_RESTORE

                            /* Determine if priority inheritance is enabled for this mutex.  */
                            if (mutex_ptr -> tx_mutex_inherit == TX_TRUE)
                            {

                                /* Yes, priority inheritance is requested.  */
        
                                /* Determine if there are any more threads still suspended on the mutex.  */
                                if (mutex_ptr -> tx_mutex_suspended_count != TX_NO_SUSPENSIONS)
                                {

                                    /* Prioritize the list so the highest priority thread is placed at the
                                       front of the suspension list.  */
#ifdef TX_MISRA_ENABLE
                                    do
                                    {
                                        status =  _tx_mutex_prioritize(mutex_ptr);
                                    } while (status != TX_SUCCESS);
#else
                                    _tx_mutex_prioritize(mutex_ptr);
#endif
                            
                                    /* Now, pickup the list head and set the priority.  */

                                    /* Disable interrupts.  */
                                    TX_DISABLE

                                    /* Determine if there still are threads suspended for this mutex.  */
                                    suspended_thread =  mutex_ptr -> tx_mutex_suspension_list;
                                    if (suspended_thread != TX_NULL)
                                    {

                                        /* Setup the highest priority thread waiting on this mutex.  */
                                        mutex_ptr -> tx_mutex_highest_priority_waiting =  suspended_thread -> tx_thread_priority;
                                    }

                                    /* Restore interrupts.  */
                                    TX_RESTORE
                                }

                                /* Restore previous priority needs to be restored after priority
                                   inheritance.  */
                                if (old_owner != TX_NULL)
                                {
                    
                                    /* Is the priority different?  */
                                    if (old_owner -> tx_thread_priority != old_priority)
                                    {
        
                                        /* Restore the priority of thread.  */
                                        _tx_mutex_priority_change(old_owner, old_priority);
                                    }
                                }
                            }

                            /* Resume thread.  */
                            _tx_thread_system_resume(thread_ptr);
#endif
                     
                            /* Return a successful status.  */
                            status =  TX_SUCCESS;
                        }
                    }
                }
            }
        }
    }
    else
    {

        /* Restore interrupts.  */
        TX_RESTORE
    
        /* Caller does not own the mutex.  */
        status =  TX_NOT_OWNED;  
    }

    /* Return the completion status.  */
    return(status);
}

