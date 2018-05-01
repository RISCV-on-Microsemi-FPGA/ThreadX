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
/*    _tx_timer_system_activate                           PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function places the specified internal timer in the proper     */ 
/*    place in the timer expiration list.  If the timer is already active */ 
/*    this function does nothing.                                         */ 
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
/*    _tx_thread_system_suspend         Thread suspend function           */ 
/*    _tx_thread_system_ni_suspend      Non-interruptable suspend thread  */ 
/*    _tx_timer_thread_entry            Timer thread processing           */ 
/*    _tx_timer_activate                Application timer activate        */ 
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
/*  12-12-2009     William E. Lamie         Modified comment(s), removed  */ 
/*                                            compound conditionals, added*/ 
/*                                            explicit value checking, and*/ 
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
VOID  _tx_timer_system_activate(TX_TIMER_INTERNAL *timer_ptr)
{

TX_INTERRUPT_SAVE_AREA

TX_TIMER_INTERNAL           **timer_list;
TX_TIMER_INTERNAL           *next_timer;
TX_TIMER_INTERNAL           *previous_timer;
ULONG                       delta;
ULONG                       remaining_ticks;
ULONG                       expiration_time;


    /* Disable interrupts.  */
    TX_DISABLE

    /* Pickup the remaining ticks.  */
    remaining_ticks =  timer_ptr -> tx_timer_internal_remaining_ticks;

    /* Determine if there is a timer to activate.  */
    if (remaining_ticks != ((ULONG) 0))
    {
    
        /* Determine if the timer is set to wait forever.  */
        if (remaining_ticks != TX_WAIT_FOREVER)
        {
    
            /* Valid timer activate request.  */

            /* Determine if the timer still needs activation.  */
            if (timer_ptr -> tx_timer_internal_list_head == TX_NULL)
            {

                /* Activate the timer.  */

                /* Calculate the amount of time remaining for the timer.  */
                if (remaining_ticks > TX_TIMER_ENTRIES)
                {

                    /* Set expiration time to the maximum number of entries.  */
                    expiration_time =  TX_TIMER_ENTRIES - ((ULONG) 1);
                }
                else
                {

                    /* Timer value fits in the timer entries.  */

                    /* Set the expiration time.  */
                    expiration_time =  (remaining_ticks - ((ULONG) 1));
                }

                /* At this point, we are ready to put the timer on one of
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
                    timer_ptr -> tx_timer_internal_active_next =      timer_ptr;
                    timer_ptr -> tx_timer_internal_active_previous =  timer_ptr;

                    /* Setup the list head pointer.  */
                    *timer_list =  timer_ptr;
                }                
                else
                {

                    /* This list is not NULL, add current timer to the end. */
                    next_timer =                                        *timer_list;
                    previous_timer =                                    next_timer -> tx_timer_internal_active_previous;
                    previous_timer -> tx_timer_internal_active_next =   timer_ptr;
                    next_timer -> tx_timer_internal_active_previous =   timer_ptr;
                    timer_ptr -> tx_timer_internal_active_next =        next_timer;
                    timer_ptr -> tx_timer_internal_active_previous =    previous_timer;
                }

                /* Setup list head pointer.  */
                timer_ptr -> tx_timer_internal_list_head =  timer_list;
            }
        }
    }
    
    /* Restore interrupts.  */
    TX_RESTORE
}

