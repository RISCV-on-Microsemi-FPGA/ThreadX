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
/*    _tx_thread_system_preempt_check                     PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for preemption that could have occurred as a   */ 
/*    result scheduling activities occurring while the preempt disable    */ 
/*    flag was set.                                                       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _tx_thread_system_return              Return to the system          */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Other ThreadX Components                                            */
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
/*                                            added null next thread      */ 
/*                                            check, added stack check    */ 
/*                                            macro, and optimized flag   */ 
/*                                            processing, resulting       */ 
/*                                            in version 5.2              */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), removed  */ 
/*                                            compound conditionals, added*/ 
/*                                            logic to explicitly check   */ 
/*                                            for valid pointer, changed  */ 
/*                                            logic to use a macro to get */ 
/*                                            the system state, and       */ 
/*                                            adjusted logic to check     */ 
/*                                            for system state or preempt */ 
/*                                            disable, resulting in       */ 
/*                                            version 5.4                 */ 
/*  07-15-2011     William E. Lamie         Modified comment(s), and      */ 
/*                                            removed redundant           */  
/*                                            conditional in stack        */ 
/*                                            checking, resulting in      */ 
/*                                            version 5.5                 */ 
/*  11-01-2012     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.6    */ 
/*  05-01-2015     William E. Lamie         Modified comment(s), and      */ 
/*                                            modified code for MISRA     */ 
/*                                            compliance, resulting in    */ 
/*                                            version 5.7                 */ 
/*                                                                        */ 
/**************************************************************************/ 
VOID  _tx_thread_system_preempt_check(VOID)
{

ULONG           combined_flags;
TX_THREAD       *current_thread;
TX_THREAD       *thread_ptr;


    /* Combine the system state and preempt disable flags into one for comparison.  */
    TX_THREAD_SYSTEM_RETURN_CHECK(combined_flags)

    /* Determine if we are in a system state (ISR or Initialization) or internal preemption is disabled.  */
    if (combined_flags == ((ULONG) 0))
    {
    
        /* No, at thread execution level so continue checking for preemption.  */

        /* Pickup thread pointer.  */
        TX_THREAD_GET_CURRENT(current_thread)

        /* Pickup the next execute pointer.  */
        thread_ptr =  _tx_thread_execute_ptr;

        /* Determine if preemption should take place.  */
        if (current_thread != thread_ptr) 
        {

#ifdef TX_ENABLE_STACK_CHECKING 

            /* Check this thread's stack.  */
            TX_THREAD_STACK_CHECK(thread_ptr)
#endif


#ifdef TX_THREAD_ENABLE_PERFORMANCE_INFO

            /* Determine if an idle system return is present.  */
            if (thread_ptr == TX_NULL)
            {

                /* Yes, increment the return to idle return count.  */
                _tx_thread_performance_idle_return_count++;
            }
            else
            {

                /* No, there is another thread ready to run and will be scheduled upon return.  */
                _tx_thread_performance_non_idle_return_count++;
            }
#endif

            /* Return to the system so the higher priority thread can be scheduled.  */
            _tx_thread_system_return();
        }
    }
}

