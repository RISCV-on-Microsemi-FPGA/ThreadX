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
#include "tx_thread.h"
#include "tx_timer.h"
#include "tx_semaphore.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _txe_semaphore_get                                  PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the semaphore get function call. */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    semaphore_ptr                     Pointer to semaphore control block*/ 
/*    wait_option                       Suspension option                 */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    TX_SEMAPHORE_ERROR                Invalid semaphore pointer         */ 
/*    TX_WAIT_ERROR                     Invalid wait option               */ 
/*    status                            Actual completion status          */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _tx_semaphore_get                 Actual get semaphore function     */ 
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
/*  12-12-2008     William E. Lamie         Modified comment(s), and added*/ 
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
UINT  _txe_semaphore_get(TX_SEMAPHORE *semaphore_ptr, ULONG wait_option)
{

UINT        status;                 

#ifndef TX_TIMER_PROCESS_IN_ISR
TX_THREAD   *current_thread;
#endif


    /* Default status to success.  */
    status =  TX_SUCCESS;

    /* Check for an invalid semaphore pointer.  */
    if (semaphore_ptr == TX_NULL)
    {

        /* Semaphore pointer is invalid, return appropriate error code.  */
        status =  TX_SEMAPHORE_ERROR;
    }
    
    /* Now check for invalid semaphore ID.  */
    else if (semaphore_ptr -> tx_semaphore_id != TX_SEMAPHORE_ID)
    {

        /* Semaphore pointer is invalid, return appropriate error code.  */
        status =  TX_SEMAPHORE_ERROR;
    }
    else
    {

        /* Check for a wait option error.  Only threads are allowed any form of 
           suspension.  */
        if (wait_option != TX_NO_WAIT)
        {

            /* Is the call from an ISR or Initialization?  */
            if (TX_THREAD_GET_SYSTEM_STATE() != ((ULONG) 0))
            {

                /* A non-thread is trying to suspend, return appropriate error code.  */
                status =  TX_WAIT_ERROR;
            }
       
#ifndef TX_TIMER_PROCESS_IN_ISR
            else
            {
            
                /* Pickup thread pointer.  */
                TX_THREAD_GET_CURRENT(current_thread)

                /* Is the current thread the timer thread?  */
                if (current_thread == &_tx_timer_thread)
                {

                    /* A non-thread is trying to suspend, return appropriate error code.  */
                    status =  TX_WAIT_ERROR;
                }
            }
#endif
        }
    }

    /* Determine if everything is okay.  */
    if (status == TX_SUCCESS)
    {

        /* Call actual get semaphore function.  */
        status =  _tx_semaphore_get(semaphore_ptr, wait_option);
    }

    /* Return completion status.  */
    return(status);
}

