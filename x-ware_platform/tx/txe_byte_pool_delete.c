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
#include "tx_thread.h"
#include "tx_timer.h"
#include "tx_byte_pool.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _txe_byte_pool_delete                               PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the delete byte pool function    */ 
/*    call.                                                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pool_ptr                          Pointer to pool control block     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    TX_POOL_ERROR                     Invalid pool pointer              */ 
/*    TX_CALLER_ERROR                   Invalid caller of this function   */ 
/*    status                            Actual completion status          */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _tx_byte_pool_delete              Actual byte pool delete function  */ 
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
UINT  _txe_byte_pool_delete(TX_BYTE_POOL *pool_ptr)
{

UINT            status;                 
#ifndef TX_TIMER_PROCESS_IN_ISR
TX_THREAD       *thread_ptr;
#endif


#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Default status to success.  */
    status =  TX_SUCCESS;
#endif
    
    /* Check for an invalid byte pool pointer.  */
    if (pool_ptr == TX_NULL)
    {

        /* Byte pool pointer is invalid, return appropriate error code.  */
        status =  TX_POOL_ERROR;
    }

    /* Now check the pool ID.  */
    else if (pool_ptr -> tx_byte_pool_id != TX_BYTE_POOL_ID)
    {

        /* Byte pool pointer is invalid, return appropriate error code.  */
        status =  TX_POOL_ERROR;
    }

    /* Check for interrupt or initialization.  */
    else if (TX_THREAD_GET_SYSTEM_STATE() != ((ULONG) 0))
    {
    
        /* Invalid caller of this function, return appropriate error code.  */
        status =  TX_CALLER_ERROR;
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

        /* Determine if everything is okay.  */
        if (status == TX_SUCCESS)
        {
#endif

            /* Call actual byte pool delete function.  */
            status =  _tx_byte_pool_delete(pool_ptr);
            
#ifndef TX_TIMER_PROCESS_IN_ISR
        }
#endif
    }

    /* Return completion status.  */
    return(status);
}

