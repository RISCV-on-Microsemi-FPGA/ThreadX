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
/*    _tx_timer_delete                                    PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deletes the specified application timer.              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    timer_ptr                         Pointer to timer control block    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    TX_SUCCESS                        Successful completion status      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _tx_timer_system_deactivate       Timer deactivation function       */ 
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
/*  12-12-2009     William E. Lamie         Modified comment(s), merged   */ 
/*                                            event logging support,      */ 
/*                                            eliminated created_count    */ 
/*                                            local variable, and         */ 
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
UINT  _tx_timer_delete(TX_TIMER *timer_ptr)
{

TX_INTERRUPT_SAVE_AREA

TX_TIMER        *next_timer;
TX_TIMER        *previous_timer;


    /* Determine if the timer needs to be deactivated.  */
    if (timer_ptr -> tx_timer_internal.tx_timer_internal_list_head != TX_NULL)
    {

        /* Yes, deactivate the timer before it is deleted.  */
        _tx_timer_system_deactivate(&(timer_ptr -> tx_timer_internal));
    }

    /* Disable interrupts to remove the timer from the created list.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_TIMER_DELETE, timer_ptr, 0, 0, 0, TX_TRACE_TIMER_EVENTS)

    /* Optional timer delete extended processing.  */
    TX_TIMER_DELETE_EXTENSION(timer_ptr)

    /* If trace is enabled, unregister this object.  */
    TX_TRACE_OBJECT_UNREGISTER(timer_ptr)

    /* Log this kernel call.  */
    TX_EL_TIMER_DELETE_INSERT

    /* Clear the timer ID to make it invalid.  */
    timer_ptr -> tx_timer_id =  TX_CLEAR_ID;

    /* Decrement the number of created timers.  */
    _tx_timer_created_count--;
    
    /* See if the timer is the only one on the list.  */
    if (_tx_timer_created_count == TX_EMPTY)
    {

        /* Only created timer, just set the created list to NULL.  */
        _tx_timer_created_ptr =  TX_NULL;
    }
    else
    {

        /* Link-up the neighbors.  */
        next_timer =                               timer_ptr -> tx_timer_created_next;
        previous_timer =                           timer_ptr -> tx_timer_created_previous;
        next_timer -> tx_timer_created_previous =  previous_timer;
        previous_timer -> tx_timer_created_next =  next_timer;

        /* See if we have to update the created list head pointer.  */
        if (_tx_timer_created_ptr == timer_ptr)
        {
                    
            /* Yes, move the head pointer to the next link. */
            _tx_timer_created_ptr =  next_timer; 
        }
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return TX_SUCCESS.  */
    return(TX_SUCCESS);
}

