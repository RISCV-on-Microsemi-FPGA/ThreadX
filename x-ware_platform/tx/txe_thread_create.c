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
#include "tx_initialize.h"
#include "tx_thread.h"
#include "tx_timer.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _txe_thread_create                                  PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the thread create function call. */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    thread_ptr                            Thread control block pointer  */ 
/*    name                                  Pointer to thread name string */ 
/*    entry_function                        Entry function of the thread  */ 
/*    entry_input                           32-bit input value to thread  */ 
/*    stack_start                           Pointer to start of stack     */ 
/*    stack_size                            Stack size in bytes           */ 
/*    priority                              Priority of thread (0-31)     */ 
/*    preempt_threshold                     Preemption threshold          */
/*    time_slice                            Thread time-slice value       */ 
/*    auto_start                            Automatic start selection     */ 
/*    thread_control_block_size             Size of thread control block  */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    TX_THREAD_ERROR                       Invalid thread pointer        */ 
/*    TX_PTR_ERROR                          Invalid entry point or stack  */ 
/*                                            address                     */ 
/*    TX_SIZE_ERROR                         Invalid stack size -too small */ 
/*    TX_PRIORITY_ERROR                     Invalid thread priority       */ 
/*    TX_THRESH_ERROR                       Invalid preemption threshold  */ 
/*    status                                Actual completion status      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _tx_thread_create                     Actual thread create function */ 
/*    _tx_thread_system_preempt_check       Check for preemption          */ 
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
/*  12-12-2008     William E. Lamie         Modified comment(s), made     */ 
/*                                            optimization to timer       */ 
/*                                            thread checking, and added  */ 
/*                                            macro to get current thread,*/ 
/*                                            resulting in version 5.2    */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), removed  */ 
/*                                            compound conditionals, added*/ 
/*                                            explicit value checking,    */ 
/*                                            added logic to explicitly   */ 
/*                                            check for valid pointer,    */ 
/*                                            changed logic to use a macro*/ 
/*                                            to get the system state, and*/ 
/*                                            added void pointer cast in  */ 
/*                                            pointer type conversions,   */ 
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
UINT    _txe_thread_create(TX_THREAD *thread_ptr, CHAR *name_ptr, 
                VOID (*entry_function)(ULONG id), ULONG entry_input,
                VOID *stack_start, ULONG stack_size, 
                UINT priority, UINT preempt_threshold,
                ULONG time_slice, UINT auto_start, UINT thread_control_block_size)
{

TX_INTERRUPT_SAVE_AREA

UINT            status;
UINT            break_flag;
ULONG           i;
TX_THREAD       *next_thread;
VOID            *stack_end;
#ifndef TX_TIMER_PROCESS_IN_ISR
TX_THREAD       *current_thread;
#endif


    /* Default status to success.  */
    status =  TX_SUCCESS;

    /* Check for an invalid thread pointer.  */
    if (thread_ptr == TX_NULL)
    {

        /* Thread pointer is invalid, return appropriate error code.  */
        status =  TX_THREAD_ERROR;
    }

    /* Now check for invalid thread control block size.  */
    else if (thread_control_block_size != (sizeof(TX_THREAD)))
    {

        /* Thread pointer is invalid, return appropriate error code.  */
        status =  TX_THREAD_ERROR;
    }
    else
    {

        /* Disable interrupts.  */
        TX_DISABLE

        /* Increment the preempt disable flag.  */
        _tx_thread_preempt_disable++;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Next see if it is already in the created list.  */
        break_flag =   TX_FALSE;
        next_thread =  _tx_thread_created_ptr;
        stack_end   =  (VOID *) (TX_UCHAR_POINTER_ADD(stack_start, (stack_size - ((ULONG) 1))));
        for (i = ((ULONG) 0); i < _tx_thread_created_count; i++)
        {
        
            /* Determine if this thread matches the thread in the list.  */
            if (thread_ptr == next_thread)
            {
        
                /* Set the break flag.  */
                break_flag =  TX_TRUE;
            }
            
            /* Determine if we need to break the loop.  */
            if (break_flag == TX_TRUE)
            {
            
                /* Yes, break out of the loop.  */
                break;
            }

            /* Check the stack pointer to see if it overlaps with this thread's stack.  */
            if (((UCHAR *) ((VOID *) stack_start)) >= ((UCHAR *) ((VOID *) next_thread -> tx_thread_stack_start)))
            {

                if (((UCHAR *) ((VOID *) stack_start)) < ((UCHAR *) ((VOID *) next_thread -> tx_thread_stack_end)))
                {
        
                    /* This stack overlaps with an existing thread, clear the stack pointer to 
                       force a stack error below.  */
                    stack_start =  TX_NULL;
                    
                    /* Set the break flag.  */
                    break_flag =  TX_TRUE;
                }
            }

            /* Check the end of the stack to see if it is inside this thread's stack area as well.  */
            if (((UCHAR *) ((VOID *) stack_end)) >= ((UCHAR *) ((VOID *) next_thread -> tx_thread_stack_start)))
            {

                if (((UCHAR *) ((VOID *) stack_end)) < ((UCHAR *) ((VOID *) next_thread -> tx_thread_stack_end)))
                {
        
                    /* This stack overlaps with an existing thread, clear the stack pointer to 
                       force a stack error below.  */
                    stack_start =  TX_NULL;
                    
                    /* Set the break flag.  */
                    break_flag =  TX_TRUE;
                }
            }

            /* Move to the next thread.  */
            next_thread =  next_thread -> tx_thread_created_next;
        }

        /* Disable interrupts.  */
        TX_DISABLE

        /* Decrement the preempt disable flag.  */
        _tx_thread_preempt_disable--;
    
        /* Restore interrupts.  */
        TX_RESTORE

        /* Check for preemption.  */
        _tx_thread_system_preempt_check();

        /* At this point, check to see if there is a duplicate thread.  */
        if (thread_ptr == next_thread)
        {

            /* Thread is already created, return appropriate error code.  */
            status =  TX_THREAD_ERROR;
        }

        /* Check for invalid starting address of stack.  */
        else if (stack_start == TX_NULL)
        {

            /* Invalid stack or entry point, return appropriate error code.  */
            status =  TX_PTR_ERROR;
        }

        /* Check for invalid thread entry point.  */
        else if (entry_function == TX_NULL)
        {

            /* Invalid stack or entry point, return appropriate error code.  */
            status =  TX_PTR_ERROR;
        }

        /* Check the stack size.  */
        else if (stack_size < ((ULONG) TX_MINIMUM_STACK))
        {

            /* Stack is not big enough, return appropriate error code.  */
            status =  TX_SIZE_ERROR;
        }

        /* Check the priority specified.  */
        else if (priority >= ((UINT) TX_MAX_PRIORITIES))
        {

            /* Invalid priority selected, return appropriate error code.  */
            status =  TX_PRIORITY_ERROR;
        }

        /* Check preemption threshold. */
        else if (preempt_threshold > priority)
        {

            /* Invalid preempt threshold, return appropriate error code.  */
            status =  TX_THRESH_ERROR;
        }

        /* Check the start selection.  */
        else if (auto_start > TX_AUTO_START)
        {

            /* Invalid auto start selection, return appropriate error code.  */
            status =  TX_START_ERROR;
        }
        else
        {

#ifndef TX_TIMER_PROCESS_IN_ISR

            /* Pickup thread pointer.  */
            TX_THREAD_GET_CURRENT(current_thread)

            /* Check for invalid caller of this function.  First check for a calling thread.  */
            if (current_thread == &_tx_timer_thread)
            {

                /* Invalid caller of this function, return appropriate error code.  */
                status =  TX_CALLER_ERROR;
            }
#endif

            /* Check for interrupt call.  */
            if (TX_THREAD_GET_SYSTEM_STATE() != ((ULONG) 0))
            {
    
                /* Now, make sure the call is from an interrupt and not initialization.  */
                if (TX_THREAD_GET_SYSTEM_STATE() < TX_INITIALIZE_IN_PROGRESS)
                {
            
                    /* Invalid caller of this function, return appropriate error code.  */
                    status =  TX_CALLER_ERROR;
                }
            }
        }
    }

    /* Determine if everything is okay.  */
    if (status == TX_SUCCESS)
    {

        /* Call actual thread create function.  */
        status =  _tx_thread_create(thread_ptr, name_ptr, entry_function, entry_input,
                        stack_start, stack_size, priority, preempt_threshold,
                        time_slice, auto_start);
    }

    /* Return completion status.  */
    return(status);
}

