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
/**   Event Flags                                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_event_flags.h"
#ifdef TX_EVENT_FLAGS_ENABLE_PERFORMANCE_INFO
#include "tx_trace.h"
#endif


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_event_flags_performance_info_get                PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves performance information from the specified  */ 
/*    event flag group.                                                   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    group_ptr                         Pointer to event flag group       */ 
/*    sets                              Destination for the number of     */ 
/*                                        event flag sets on this group   */ 
/*    gets                              Destination for the number of     */ 
/*                                        event flag gets on this group   */ 
/*    suspensions                       Destination for the number of     */ 
/*                                        event flag suspensions on this  */ 
/*                                        group                           */ 
/*    timeouts                          Destination for number of timeouts*/ 
/*                                        on this event flag group        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Completion status                 */ 
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
/*  12-12-2008     William E. Lamie         Modified comment(s), and      */ 
/*                                            added filter option to      */ 
/*                                            trace insert, resulting     */ 
/*                                            in version 5.2              */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), removed  */ 
/*                                            compound conditionals, added*/ 
/*                                            logic to explicitly check   */ 
/*                                            for valid pointer, merged   */ 
/*                                            event logging support, and  */ 
/*                                            added code to ensure that   */ 
/*                                            input parameters are        */ 
/*                                            accessed in non-enabled     */ 
/*                                            case (default),             */ 
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
UINT  _tx_event_flags_performance_info_get(TX_EVENT_FLAGS_GROUP *group_ptr, ULONG *sets, ULONG *gets,
                    ULONG *suspensions, ULONG *timeouts)
{

#ifdef TX_EVENT_FLAGS_ENABLE_PERFORMANCE_INFO

TX_INTERRUPT_SAVE_AREA
UINT                    status;


    /* Determine if this is a legal request.  */
    if (group_ptr == TX_NULL)
    {
        
        /* Event flags group pointer is illegal, return error.  */
        status =  TX_PTR_ERROR;
    }
    
    /* Determine if the event group ID is invalid.  */
    else if (group_ptr -> tx_event_flags_group_id != TX_EVENT_FLAGS_ID)
    {
        
        /* Event flags group pointer is illegal, return error.  */
        status =  TX_PTR_ERROR;
    }
    else
    {

        /* Disable interrupts.  */
        TX_DISABLE

        /* If trace is enabled, insert this event into the trace buffer.  */
        TX_TRACE_IN_LINE_INSERT(TX_TRACE_EVENT_FLAGS_PERFORMANCE_INFO_GET, group_ptr, 0, 0, 0, TX_TRACE_EVENT_FLAGS_EVENTS)

        /* Log this kernel call.  */
        TX_EL_EVENT_FLAGS_PERFORMANCE_INFO_GET_INSERT

        /* Retrieve all the pertinent information and return it in the supplied
           destinations.  */

        /* Retrieve the number of set operations on this event flag group.  */
        if (sets != TX_NULL)
        {
    
            *sets =  group_ptr -> tx_event_flags_group_performance_set_count;
        }
    
        /* Retrieve the number of get operations on this event flag group.  */
        if (gets != TX_NULL)
        {
    
            *gets =  group_ptr -> tx_event_flags_group__performance_get_count;
        }
    
        /* Retrieve the number of thread suspensions on this event flag group.  */
        if (suspensions != TX_NULL)
        {
    
            *suspensions =  group_ptr -> tx_event_flags_group___performance_suspension_count;
        }
    
        /* Retrieve the number of thread timeouts on this event flag group.  */
        if (timeouts != TX_NULL)
        {
    
            *timeouts =  group_ptr -> tx_event_flags_group____performance_timeout_count;
        }
    
        /* Restore interrupts.  */
        TX_RESTORE
        
        /* Return successful completion.  */
        status =  TX_SUCCESS;
    }    
#else
UINT                    status;


    /* Access input arguments just for the sake of lint, MISRA, etc.  */
    if (group_ptr != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (sets != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (gets != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (suspensions != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (timeouts != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
#endif

    /* Return completion status.  */
    return(status);
}

