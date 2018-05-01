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
#ifdef TX_ENABLE_EVENT_TRACE
#include "tx_trace.h"
#endif


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_timer_activate                                  PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function activates the specified application timer.            */ 
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
/*    _tx_timer_system_activate         Actual timer activation function  */ 
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
/*  12-12-2008     William E. Lamie         Modified comment(s), and      */ 
/*                                            added filter option to      */ 
/*                                            trace insert, resulting     */ 
/*                                            in version 5.2              */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), removed  */ 
/*                                            compound conditionals, added*/ 
/*                                            logic to explicitly check   */ 
/*                                            for valid ticks, merged     */ 
/*                                            event logging support, and  */ 
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
UINT  _tx_timer_activate(TX_TIMER *timer_ptr)
{

#ifdef TX_ENABLE_EVENT_TRACE
TX_INTERRUPT_SAVE_AREA
#else
#ifdef TX_ENABLE_EVENT_LOGGING
TX_INTERRUPT_SAVE_AREA
#endif
#endif

UINT        status;


#ifdef TX_ENABLE_EVENT_TRACE
    
    /* Disable interrupts to put the timer on the created list.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_TIMER_ACTIVATE, timer_ptr, 0, 0, 0, TX_TRACE_TIMER_EVENTS)

    /* Restore interrupts.  */
    TX_RESTORE
#endif

#ifdef TX_ENABLE_EVENT_LOGGING
    
    /* Disable interrupts to put the timer on the created list.  */
    TX_DISABLE

    /* Log this kernel call.  */
    TX_EL_TIMER_ACTIVATE_INSERT

    /* Restore interrupts.  */
    TX_RESTORE
#endif

    /* Check for an already active timer.  */ 
    if (timer_ptr -> tx_timer_internal.tx_timer_internal_list_head != TX_NULL)
    {

        /* Timer is already active, return an error.  */
        status =  TX_ACTIVATE_ERROR;
    }

    /* Check for a timer with a zero expiration.  */
    else if (timer_ptr -> tx_timer_internal.tx_timer_internal_remaining_ticks == ((ULONG) 0))
    {

        /* Timer is being activated with a zero expiration.  */
        status =  TX_ACTIVATE_ERROR;
    }
    else
    {

#ifdef TX_TIMER_ENABLE_PERFORMANCE_INFO

        /* Increment the total activations counter.  */
        _tx_timer_performance_activate_count++;

        /* Increment the number of activations on this timer.  */
        timer_ptr -> tx_timer_performance_activate_count++;
#endif

        /* Call actual activation function.  */
        _tx_timer_system_activate(&(timer_ptr -> tx_timer_internal));

        /* Return a successful status.  */
        status =  TX_SUCCESS;
    }

    /* Return completion status.  */
    return(status);
}

