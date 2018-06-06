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
#include "tx_trace.h"
#include "tx_timer.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_timer_deactivate                                PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deactivates the specified application timer.          */ 
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
/*                                            filter option to trace      */ 
/*                                            insert, and made several    */ 
/*                                            optimizations, resulting    */ 
/*                                            in version 5.2              */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), removed  */ 
/*                                            compound conditionals, added*/ 
/*                                            logic to explicitly check   */ 
/*                                            for valid pointer, merged   */ 
/*                                            event logging support, and  */ 
/*                                            added ULONG casting,        */ 
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
UINT  _tx_timer_deactivate(TX_TIMER *timer_ptr)
{
TX_INTERRUPT_SAVE_AREA

TX_TIMER_INTERNAL   *internal_ptr;           
TX_TIMER_INTERNAL   **list_head;
TX_TIMER_INTERNAL   *next_timer;           
TX_TIMER_INTERNAL   *previous_timer;           
ULONG               ticks_left;              

    
    /* Setup internal timer pointer.  */
    internal_ptr =  &(timer_ptr -> tx_timer_internal);

    /* Disable interrupts while the remaining time before expiration is
       calculated.  */
    TX_DISABLE

#ifdef TX_TIMER_ENABLE_PERFORMANCE_INFO

    /* Increment the total deactivations counter.  */
    _tx_timer_performance_deactivate_count++;

    /* Increment the number of deactivations on this timer.  */
    timer_ptr -> tx_timer_performance_deactivate_count++;
#endif

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_TIMER_DEACTIVATE, timer_ptr, TX_POINTER_TO_ULONG_CONVERT(&ticks_left), 0, 0, TX_TRACE_TIMER_EVENTS)

    /* Log this kernel call.  */
    TX_EL_TIMER_DEACTIVATE_INSERT

    /* Pickup the list head.  */
    list_head =  internal_ptr -> tx_timer_internal_list_head;

    /* Determine if the head pointer is within the timer expiration list.  */
    if (list_head >= _tx_timer_list_start)
    {

        /* Now check to make sure the list head is before the end of the list.  */
        if (list_head < _tx_timer_list_end)
        {

            /* This timer is active and has not yet expired.  */

            /* Calculate the amount of time that has elapsed since the timer
               was activated.  */

            /* Is this timer's entry after the current timer pointer?  */
            if (list_head >= _tx_timer_current_ptr)
            {

                /* Calculate ticks left to expiration - just the difference between this 
                   timer's entry and the current timer pointer.  */
                ticks_left =  (ULONG) (TX_TIMER_POINTER_DIF(list_head,_tx_timer_current_ptr)) + ((ULONG) 1);
            }
            else
            {

                /* Calculate the ticks left with a wrapped list condition.  */
                ticks_left =  (ULONG) (TX_TIMER_POINTER_DIF(list_head,_tx_timer_list_start));
    
                ticks_left =  ticks_left + (ULONG) ((TX_TIMER_POINTER_DIF(_tx_timer_list_end, _tx_timer_current_ptr)) + ((ULONG) 1));
            }

            /* Adjust the remaining ticks accordingly.  */
            if (internal_ptr -> tx_timer_internal_remaining_ticks > TX_TIMER_ENTRIES)
            {
            
                /* Subtract off the last full pass through the timer list and add the
                   time left.  */
                internal_ptr -> tx_timer_internal_remaining_ticks =  
                        (internal_ptr -> tx_timer_internal_remaining_ticks - TX_TIMER_ENTRIES) + ticks_left;
            }
            else
            {
    
                /* Just put the ticks left into the timer's remaining ticks.  */
                internal_ptr -> tx_timer_internal_remaining_ticks =  ticks_left;
            }
        }
    }

    /* Determine if the timer still needs deactivation.  */
    if (list_head != TX_NULL)
    {

        /* Pickup the next timer.  */
        next_timer =  internal_ptr -> tx_timer_internal_active_next;

        /* See if this is the only timer in the list.  */
        if (internal_ptr == next_timer)
        {

            /* Yes, the only timer on the list.  */

            /* Determine if the head pointer needs to be updated.  */
            if (*(list_head) == internal_ptr)
            {

                /* Update the head pointer.  */
                *(list_head) =  TX_NULL;
            }
        }
        else
        {

            /* At least one more timer is on the same expiration list.  */

            /* Update the links of the adjacent timers.  */
            previous_timer =                                   internal_ptr -> tx_timer_internal_active_previous;
            next_timer -> tx_timer_internal_active_previous =  previous_timer;
            previous_timer -> tx_timer_internal_active_next =  next_timer;

            /* Determine if the head pointer needs to be updated.  */
            if (*(list_head) == internal_ptr)
            {

                /* Update the next timer in the list with the list head 
                   pointer.  */
                next_timer -> tx_timer_internal_list_head =  list_head;

                /* Update the head pointer.  */
                *(list_head) =  next_timer;
            }
        }

        /* Clear the timer's list head pointer.  */
        internal_ptr -> tx_timer_internal_list_head =  TX_NULL;
    }

    /* Restore interrupts to previous posture.  */
    TX_RESTORE

    /* Return TX_SUCCESS.  */
    return(TX_SUCCESS);
}

