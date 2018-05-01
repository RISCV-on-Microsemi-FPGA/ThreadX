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
#include "tx_initialize.h"
#include "tx_thread.h"
#include "tx_timer.h"
#include "tx_byte_pool.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _txe_byte_pool_create                               PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the create byte pool memory      */ 
/*    function.                                                           */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pool_ptr                          Pointer to pool control block     */ 
/*    name_ptr                          Pointer to byte pool name         */ 
/*    pool_start                        Address of beginning of pool area */ 
/*    pool_size                         Number of bytes in the byte pool  */ 
/*    pool_control_block_size           Size of byte pool control block   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    TX_POOL_ERROR                     Invalid byte pool pointer         */ 
/*    TX_PTR_ERROR                      Invalid pool starting address     */ 
/*    TX_SIZE_ERROR                     Invalid pool size                 */ 
/*    TX_CALLER_ERROR                   Invalid caller of this function   */ 
/*    status                            Actual completion status          */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _tx_byte_pool_create              Actual byte pool create function  */ 
/*    _tx_thread_system_preempt_check   Check for preemption              */ 
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
/*                                            changed logic to use a macro*/ 
/*                                            to get the system state, and*/ 
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
UINT  _txe_byte_pool_create(TX_BYTE_POOL *pool_ptr, CHAR *name_ptr, VOID *pool_start, ULONG pool_size, UINT pool_control_block_size)
{

TX_INTERRUPT_SAVE_AREA

UINT            status;                 
ULONG           i;
TX_BYTE_POOL    *next_pool;
#ifndef TX_TIMER_PROCESS_IN_ISR
TX_THREAD       *thread_ptr;
#endif


    /* Default status to success.  */
    status =  TX_SUCCESS;

    /* Check for an invalid byte pool pointer.  */
    if (pool_ptr == TX_NULL) 
    {

        /* Byte pool pointer is invalid, return appropriate error code.  */
        status =  TX_POOL_ERROR;
    }

    /* Now see if the pool control block size is valid.  */
    else if (pool_control_block_size != (sizeof(TX_BYTE_POOL)))
    {

        /* Byte pool pointer is invalid, return appropriate error code.  */
        status =  TX_POOL_ERROR;
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
        next_pool =   _tx_byte_pool_created_ptr;
        for (i = ((ULONG) 0); i < _tx_byte_pool_created_count; i++)
        {

            /* Determine if this byte pool matches the pool in the list.  */
            if (pool_ptr == next_pool)
            {

                break;
            }
            else
            {

                /* Move to the next pool.  */
                next_pool =  next_pool -> tx_byte_pool_created_next;
            }
        }

        /* Disable interrupts.  */
        TX_DISABLE

        /* Decrement the preempt disable flag.  */
        _tx_thread_preempt_disable--;
    
        /* Restore interrupts.  */
        TX_RESTORE

        /* Check for preemption.  */
        _tx_thread_system_preempt_check();

        /* At this point, check to see if there is a duplicate pool.  */
        if (pool_ptr == next_pool)
        {

            /* Pool is already created, return appropriate error code.  */
            status =  TX_POOL_ERROR;
        }

        /* Check for an invalid starting address.  */
        else if (pool_start == TX_NULL)
        {
    
            /* Null starting address pointer, return appropriate error.  */
            status =  TX_PTR_ERROR;
        }

        /* Check for invalid pool size.  */
        else if (pool_size < TX_BYTE_POOL_MIN)
        {

            /* Pool not big enough, return appropriate error.  */
            status =  TX_SIZE_ERROR;
        }
        else
        {

#ifndef TX_TIMER_PROCESS_IN_ISR

            /* Pickup thread pointer.  */
            TX_THREAD_GET_CURRENT(thread_ptr)

            /* Check for invalid caller of this function.  First check for a calling thread.  */
            if (thread_ptr == &_tx_timer_thread)
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

        /* Call actual byte pool create function.  */
        status =  _tx_byte_pool_create(pool_ptr, name_ptr, pool_start, pool_size);
    }

    /* Return completion status.  */
    return(status);
}

