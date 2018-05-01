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


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_timer_system_deactivate                         PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deactivates, or removes the timer from the active     */ 
/*    timer expiration list.  If the timer is already deactivated, this   */ 
/*    function just returns.                                              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    timer_ptr                         Pointer to timer control block    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    TX_SUCCESS                        Always returns success            */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _tx_thread_system_resume          Thread resume function            */ 
/*    _tx_timer_thread_entry            Timer thread processing           */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-12-2005     William E. Lamie         Initial Version 5.0           */ 
/*  04-02-2007     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.1    */ 
/*  12-12-2008     William E. Lamie         Modified comment(s), and made */ 
/*                                            several optimizations,      */ 
/*                                            resulting in version 5.2    */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), added    */ 
/*                                            logic to explicitly check   */ 
/*                                            for valid pointer, and      */ 
/*                                            removed unnecessary status  */ 
/*                                            return, resulting in        */ 
/*                                            version 5.4                 */ 
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
VOID  _tx_timer_system_deactivate(TX_TIMER_INTERNAL *timer_ptr)
{

TX_INTERRUPT_SAVE_AREA

TX_TIMER_INTERNAL   **list_head;
TX_TIMER_INTERNAL   *next_timer;           
TX_TIMER_INTERNAL   *previous_timer;           


    /* Disable interrupts.  */
    TX_DISABLE

    /* Pickup the list head pointer.  */
    list_head =  timer_ptr -> tx_timer_internal_list_head;

    /* Determine if the timer still needs deactivation.  */
    if (list_head != TX_NULL)
    {

        /* Deactivate the timer.  */

        /* Pickup the next active timer.  */
        next_timer =  timer_ptr -> tx_timer_internal_active_next;

        /* See if this is the only timer in the list.  */
        if (timer_ptr == next_timer)
        {

            /* Yes, the only timer on the list.  */

            /* Determine if the head pointer needs to be updated.  */
            if (*(list_head) == timer_ptr)
            {

                /* Update the head pointer.  */
                *(list_head) =  TX_NULL;
            }
        }
        else
        {

            /* At least one more timer is on the same expiration list.  */

            /* Update the links of the adjacent timers.  */
            previous_timer =                                   timer_ptr -> tx_timer_internal_active_previous;
            next_timer -> tx_timer_internal_active_previous =  previous_timer;
            previous_timer -> tx_timer_internal_active_next =  next_timer;

            /* Determine if the head pointer needs to be updated.  */
            if (*(list_head) == timer_ptr)
            {

                /* Update the next timer in the list with the list head pointer.  */
                next_timer -> tx_timer_internal_list_head =  list_head;

                /* Update the head pointer.  */
                *(list_head) =  next_timer;
            }
        }

        /* Clear the timer's list head pointer.  */
        timer_ptr -> tx_timer_internal_list_head =  TX_NULL;
    }

    /* Restore interrupts.  */
    TX_RESTORE
}

