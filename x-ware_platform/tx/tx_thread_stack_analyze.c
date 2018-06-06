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
/*    _tx_thread_stack_analyze                            PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function analyzes the stack to calculate the highest stack     */ 
/*    pointer in the thread's stack. This can then be used to derive the  */ 
/*    minimum amount of stack left for any given thread.                  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    thread_ptr                            Thread control block pointer  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    ThreadX internal code                                               */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-12-2005     William E. Lamie         Initial Version 5.0           */ 
/*  04-02-2007     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.1    */ 
/*  12-12-2008     William E. Lamie         Modified comment(s), and      */ 
/*                                            made optimization,          */ 
/*                                            resulting in version 5.2    */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), removed  */ 
/*                                            compound conditionals, added*/ 
/*                                            logic to explicitly check   */ 
/*                                            for valid pointer, and      */ 
/*                                            added ULONG cast, resulting */ 
/*                                            in version 5.4              */ 
/*  07-15-2011     William E. Lamie         Modified comment(s), and      */ 
/*                                            added logic to check for    */ 
/*                                            thread validity before      */ 
/*                                            updating highest stack      */ 
/*                                            pointer, resulting in       */ 
/*                                            version 5.5                 */ 
/*  11-01-2012     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.6    */ 
/*  05-01-2015     William E. Lamie         Modified comment(s), and      */ 
/*                                            modified code for MISRA     */ 
/*                                            compliance, resulting in    */ 
/*                                            version 5.7                 */ 
/*                                                                        */ 
/**************************************************************************/ 
VOID  _tx_thread_stack_analyze(TX_THREAD *thread_ptr)
{

TX_INTERRUPT_SAVE_AREA

ULONG       *stack_ptr;
ULONG       *stack_lowest;
ULONG       *stack_highest;
ULONG       size;


    /* Disable interrupts.  */
    TX_DISABLE

    /* Determine if the thread pointer is NULL.  */
    if (thread_ptr != TX_NULL)
    {

        /* Determine if the thread ID is invalid.  */
        if (thread_ptr -> tx_thread_id == TX_THREAD_ID)
        {

            /* Pickup the current stack variables.  */
            stack_lowest =   (ULONG *) thread_ptr -> tx_thread_stack_start;
    
            /* Determine if the pointer is null.  */
            if (stack_lowest != TX_NULL)
            {

                /* Pickup the highest stack pointer.  */        
                stack_highest =  (ULONG *) thread_ptr -> tx_thread_stack_highest_ptr;

                /* Determine if the pointer is null.  */
                if (stack_highest != TX_NULL)
                {
    
                    /* Restore interrupts.  */
                    TX_RESTORE

                    /* We need to binary search the remaining stack for missing 0xEFEFEFEF 32-bit data pattern. 
                       This is a best effort algorithm to find the highest stack usage. */
                    do
                    {

                        /* Calculate the size again. */
                        size =  (ULONG) (TX_ULONG_POINTER_DIF(stack_highest, stack_lowest))/((ULONG) 2);
                        stack_ptr =  TX_ULONG_POINTER_ADD(stack_lowest, size);

                        /* Determine if the pattern is still there.  */
                        if (*stack_ptr != TX_STACK_FILL)
                        {

                            /* Update the stack highest, since we need to look in the upper half now.  */
                            stack_highest =  stack_ptr;
                        }
                        else
                        {

                            /* Update the stack lowest, since we need to look in the lower half now.  */
                            stack_lowest =  stack_ptr;
                        }

                    } while(size > ((ULONG) 1));

                    /* Position to first used word - at this point we are within a few words.  */
                    while (*stack_ptr == TX_STACK_FILL)
                    {
            
                        /* Position to next word in stack.  */
                        stack_ptr =  TX_ULONG_POINTER_ADD(stack_ptr, 1);
                    }

                    /* Disable interrupts.  */
                    TX_DISABLE

                    /* Check to see if the thread is still created.  */
                    if (thread_ptr -> tx_thread_id == TX_THREAD_ID)
                    {

                        /* Yes, thread is still created.  */
        
                        /* Now check the new highest stack pointer is past the stack start.  */
                        if (stack_ptr > ((ULONG *) thread_ptr -> tx_thread_stack_start))
                        {
        
                            /* Yes, now check that the new highest stack pointer is less than the previous highest stack pointer.  */
                            if (stack_ptr < ((ULONG *) thread_ptr -> tx_thread_stack_highest_ptr))
                            {
            
                                /* Yes, is the current highest stack pointer pointing at used memory?  */
                                if (*stack_ptr != TX_STACK_FILL)
                                {
        
                                    /* Yes, setup the highest stack usage.  */
                                    thread_ptr -> tx_thread_stack_highest_ptr =  stack_ptr;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /* Restore interrupts.  */
    TX_RESTORE
}

