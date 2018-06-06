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
/**   Byte Memory                                                         */
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
/*    _tx_byte_release                                    PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function returns previously allocated memory to its            */ 
/*    associated memory byte pool.                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    memory_ptr                        Pointer to allocated memory       */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    [TX_PTR_ERROR | TX_SUCCESS]       Completion status                 */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _tx_thread_system_preempt_check   Check for preemption              */ 
/*    _tx_thread_system_resume          Resume thread service             */ 
/*    _tx_thread_system_ni_resume       Non-interruptable resume thread   */ 
/*    _tx_byte_pool_search              Search the byte pool for memory   */ 
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
/*  04-02-2007     William E. Lamie         Modified comment(s), and      */ 
/*                                            added optimization for      */ 
/*                                            memory search pointer       */ 
/*                                            update, resulting in        */ 
/*                                            version 5.1                 */ 
/*  12-12-2008     William E. Lamie         Modified comment(s), added    */ 
/*                                            macro to get current thread,*/ 
/*                                            added filter option to trace*/ 
/*                                            insert, added optional      */ 
/*                                            logic for non-interruptable */ 
/*                                            operation, and made several */ 
/*                                            optimizations, resulting    */ 
/*                                            in version 5.2              */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), added    */ 
/*                                            logic to explicitly check   */ 
/*                                            for valid pointer, added    */ 
/*                                            void pointer cast in        */ 
/*                                            pointer type conversions,   */ 
/*                                            changed some counting       */ 
/*                                            variables to type UINT,     */ 
/*                                            merged event logging        */ 
/*                                            support, and added          */ 
/*                                            parentheses in calculation  */ 
/*                                            of available bytes,         */ 
/*                                            resulting in version 5.4    */ 
/*  07-15-2011     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.5    */ 
/*  11-01-2012     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.6    */ 
/*  05-01-2015     William E. Lamie         Modified comment(s), modified */ 
/*                                            code for MISRA compliance,  */ 
/*                                            added code to assert pool   */ 
/*                                            ownership after protection  */ 
/*                                            is obtained to ensure no    */ 
/*                                            changes to the pool or the  */ 
/*                                            pool search pointer are     */ 
/*                                            made without ownership,     */ 
/*                                            resulting in version 5.7    */ 
/*                                                                        */ 
/**************************************************************************/ 
UINT  _tx_byte_release(VOID *memory_ptr)
{

TX_INTERRUPT_SAVE_AREA

UINT                status;
TX_BYTE_POOL        *pool_ptr;          
TX_THREAD           *thread_ptr;        
UCHAR               *work_ptr;           
UCHAR               *next_block_ptr;
TX_THREAD           *susp_thread_ptr;   
UINT                suspended_count;
TX_THREAD           *next_thread;
TX_THREAD           *previous_thread;
ULONG               memory_size;


    /* Default to successful status.  */
    status =  TX_SUCCESS;

    /* Lockout interrupts.  */
    TX_DISABLE

    /* Determine if the memory pointer is valid.  */
    work_ptr =  (UCHAR *) memory_ptr;
    if (work_ptr != TX_NULL)
    {
        
        /* Back off the memory pointer to pickup its header.  */
        work_ptr =  TX_UCHAR_POINTER_SUB(work_ptr, ((sizeof(UCHAR *)) + (sizeof(ULONG))));

        /* There is a pointer, pickup the pool pointer address.  */
        if (*((ULONG *) ((VOID *) (TX_UCHAR_POINTER_ADD(work_ptr, (sizeof(UCHAR *)))))) != TX_BYTE_BLOCK_FREE)
        {

            /* Pickup the pool pointer.  */
            pool_ptr =  (TX_BYTE_POOL *) ((VOID *) *((TX_BYTE_POOL **) ((VOID *) (TX_UCHAR_POINTER_ADD(work_ptr, (sizeof(UCHAR *)))))));

            /* See if we have a valid pool pointer.  */
            if (pool_ptr == TX_NULL)
            {
                
                /* Return pointer error.  */
                status =  TX_PTR_ERROR;
            }
            else 
            {

                /* See if we have a valid pool.  */
                if (pool_ptr -> tx_byte_pool_id != TX_BYTE_POOL_ID)
                {
                
                    /* Return pointer error.  */
                    status =  TX_PTR_ERROR;
                }
            }
        }
        else
        {

            /* Return pointer error.  */
            status =  TX_PTR_ERROR;
        }
    }
    else
    {

        /* Return pointer error.  */
        status =  TX_PTR_ERROR;
    }

    /* Determine if the pointer is valid.  */
    if (status == TX_PTR_ERROR)
    {
    
        /* Restore interrupts.  */
        TX_RESTORE
    }
    else
    {
    
        /* At this point, we know that the pointer is valid.  */

        /* Pickup thread pointer.  */
        TX_THREAD_GET_CURRENT(thread_ptr)

        /* Indicate that this thread is the current owner.  */
        pool_ptr -> tx_byte_pool_owner =  thread_ptr;

#ifdef TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO

        /* Increment the total release counter.  */
        _tx_byte_pool_performance_release_count++;

        /* Increment the number of releases on this pool.  */
        pool_ptr -> tx_byte_pool_performance_release_count++;
#endif

        /* If trace is enabled, insert this event into the trace buffer.  */
        TX_TRACE_IN_LINE_INSERT(TX_TRACE_BYTE_RELEASE, pool_ptr, TX_POINTER_TO_ULONG_CONVERT(memory_ptr), pool_ptr -> tx_byte_pool_suspended_count, pool_ptr -> tx_byte_pool_available, TX_TRACE_BYTE_POOL_EVENTS)

        /* Log this kernel call.  */
        TX_EL_BYTE_RELEASE_INSERT

        /* Release the memory.  */
        *((ULONG *) ((VOID *) (TX_UCHAR_POINTER_ADD(work_ptr, (sizeof(UCHAR *)))))) =  TX_BYTE_BLOCK_FREE;

        /* Update the number of available bytes in the pool.  */
        next_block_ptr =  *((UCHAR **) ((VOID *) work_ptr));
        pool_ptr -> tx_byte_pool_available =  
            pool_ptr -> tx_byte_pool_available + TX_UCHAR_POINTER_DIF(next_block_ptr, work_ptr);

        /* Determine if the free block is prior to current search pointer.  */
        if (work_ptr < (pool_ptr -> tx_byte_pool_search))
        {

            /* Yes, update the search pointer to the released block.  */
            pool_ptr -> tx_byte_pool_search =  work_ptr;
        }

        /* Determine if there are threads suspended on this byte pool.  */
        if (pool_ptr -> tx_byte_pool_suspended_count != TX_NO_SUSPENSIONS)
        {
                
            /* Now examine the suspension list to find threads waiting for 
               memory.  Maybe it is now available!  */
            while (pool_ptr -> tx_byte_pool_suspended_count != TX_NO_SUSPENSIONS)
            {

                /* Pickup the first suspended thread pointer.  */
                susp_thread_ptr =  pool_ptr -> tx_byte_pool_suspension_list;

                /* Pickup the size of the memory the thread is requesting.  */
                memory_size =  susp_thread_ptr -> tx_thread_suspend_info;

                /* Restore interrupts.  */
                TX_RESTORE

                /* See if the request can be satisfied.  */
                work_ptr =  _tx_byte_pool_search(pool_ptr, memory_size);   

                /* Disable interrupts.  */
                TX_DISABLE

                /* Indicate that this thread is the current owner.  */
                pool_ptr -> tx_byte_pool_owner =  thread_ptr;

                /* If there is not enough memory, break this loop!  */
                if (work_ptr == TX_NULL)
                {
          
                  /* Break out of the loop.  */
                    break;
                }

                /* Check to make sure the thread is still suspended.  */
                if (susp_thread_ptr ==  pool_ptr -> tx_byte_pool_suspension_list)
                {

                    /* Also, makes sure the memory size is the same.  */
                    if (susp_thread_ptr -> tx_thread_suspend_info == memory_size)
                    {
                  
                        /* Remove the suspended thread from the list.  */

                        /* Decrement the number of threads suspended.  */
                        pool_ptr -> tx_byte_pool_suspended_count--;

                        /* Pickup the suspended count.  */
                        suspended_count =  pool_ptr -> tx_byte_pool_suspended_count;

                        /* See if this is the only suspended thread on the list.  */
                        if (suspended_count == TX_NO_SUSPENSIONS)
                        {

                            /* Yes, the only suspended thread.  */

                            /* Update the head pointer.  */
                            pool_ptr -> tx_byte_pool_suspension_list =  TX_NULL;
                        }
                        else
                        {

                            /* At least one more thread is on the same expiration list.  */

                            /* Update the list head pointer.  */
                            next_thread =                                susp_thread_ptr -> tx_thread_suspended_next;
                            pool_ptr -> tx_byte_pool_suspension_list =   next_thread;

                            /* Update the links of the adjacent threads.  */
                            previous_thread =                              susp_thread_ptr -> tx_thread_suspended_previous;
                            next_thread -> tx_thread_suspended_previous =  previous_thread;
                            previous_thread -> tx_thread_suspended_next =  next_thread;
                        } 
 
                        /* Prepare for resumption of the thread.  */

                        /* Clear cleanup routine to avoid timeout.  */
                        susp_thread_ptr -> tx_thread_suspend_cleanup =  TX_NULL;

                        /* Return this block pointer to the suspended thread waiting for
                           a block.  */
                        *((UCHAR **) ((VOID *) susp_thread_ptr -> tx_thread_additional_suspend_info)) =  (UCHAR *) ((VOID *) work_ptr);

                        /* Clear the memory pointer to indicate that it was given to the suspended thread.  */
                        work_ptr =  TX_NULL;
                        
                        /* Put return status into the thread control block.  */
                        susp_thread_ptr -> tx_thread_suspend_status =  TX_SUCCESS;

#ifdef TX_NOT_INTERRUPTABLE

                        /* Resume the thread!  */
                        _tx_thread_system_ni_resume(susp_thread_ptr);

                        /* Restore interrupts.  */
                        TX_RESTORE
#else
                        /* Temporarily disable preemption.  */
                        _tx_thread_preempt_disable++;

                        /* Restore interrupts.  */
                        TX_RESTORE

                        /* Resume thread.  */
                        _tx_thread_system_resume(susp_thread_ptr);
#endif

                        /* Lockout interrupts.  */
                        TX_DISABLE
                    }
                }
                    
                /* Determine if the memory was given to the suspended thread.  */
                if (work_ptr != TX_NULL)
                {
                
                    /* No, it wasn't given to the suspended thread.  */

                    /* Put the memory back on the available list since this thread is no longer
                       suspended.  */
                    work_ptr =  TX_UCHAR_POINTER_SUB(work_ptr, (((sizeof(UCHAR *)) + (sizeof(ULONG)))));
                    *((ULONG *) ((VOID *) (TX_UCHAR_POINTER_ADD(work_ptr, (sizeof(UCHAR *)))))) =  TX_BYTE_BLOCK_FREE;

                    /* Update the number of available bytes in the pool.  */
                    next_block_ptr =  *((UCHAR **) ((VOID *) work_ptr));
                    pool_ptr -> tx_byte_pool_available =  
                        pool_ptr -> tx_byte_pool_available + TX_UCHAR_POINTER_DIF(next_block_ptr, work_ptr);

                    /* Determine if the current pointer is before the search pointer.  */
                    if (work_ptr < (pool_ptr -> tx_byte_pool_search))
                    { 

                        /* Yes, update the search pointer.  */
                        pool_ptr -> tx_byte_pool_search =  work_ptr;
                    }
                }
            }
            
            /* Restore interrupts.  */
            TX_RESTORE

            /* Check for preemption.  */
            _tx_thread_system_preempt_check();
        }
        else
        {
        
            /* No, threads suspended, restore interrupts.  */
            TX_RESTORE
        }
    }

    /* Return completion status.  */
    return(status);
}

