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
/**   Queue                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_queue.h"
#ifdef TX_QUEUE_ENABLE_PERFORMANCE_INFO
#include "tx_trace.h"
#endif


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_queue_performance_info_get                      PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves performance information from the specified  */ 
/*    queue.                                                              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    queue_ptr                         Pointer to queue control block    */ 
/*    messages_sent                     Destination for messages sent     */ 
/*    messages_received                 Destination for messages received */ 
/*    empty_suspensions                 Destination for number of empty   */ 
/*                                        queue suspensions               */ 
/*    full_suspensions                  Destination for number of full    */ 
/*                                        queue suspensions               */ 
/*    full_errors                       Destination for queue full errors */ 
/*                                        returned - no suspension        */ 
/*    timeouts                          Destination for number of timeouts*/ 
/*                                        on this queue                   */ 
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
UINT  _tx_queue_performance_info_get(TX_QUEUE *queue_ptr, ULONG *messages_sent, ULONG *messages_received,
                    ULONG *empty_suspensions, ULONG *full_suspensions, ULONG *full_errors, ULONG *timeouts)
{

#ifdef TX_QUEUE_ENABLE_PERFORMANCE_INFO

TX_INTERRUPT_SAVE_AREA
UINT                    status;


    /* Determine if this is a legal request.  */
    if (queue_ptr == TX_NULL)
    {
        
        /* Queue pointer is illegal, return error.  */
        status =  TX_PTR_ERROR;
    }
    
    /* Determine if the queue ID is invalid.  */
    else if (queue_ptr -> tx_queue_id != TX_QUEUE_ID)
    {
        
        /* Queue pointer is illegal, return error.  */
        status =  TX_PTR_ERROR;
    }
    else
    {

        /* Disable interrupts.  */
        TX_DISABLE

        /* If trace is enabled, insert this event into the trace buffer.  */
        TX_TRACE_IN_LINE_INSERT(TX_TRACE_QUEUE_PERFORMANCE_INFO_GET, queue_ptr, 0, 0, 0, TX_TRACE_QUEUE_EVENTS)

        /* Log this kernel call.  */
        TX_EL_QUEUE_PERFORMANCE_INFO_GET_INSERT

        /* Retrieve all the pertinent information and return it in the supplied
           destinations.  */

        /* Retrieve the number of messages sent to this queue.  */
        if (messages_sent != TX_NULL)
        {
    
            *messages_sent =  queue_ptr -> tx_queue_performance_messages_sent_count;
        }
    
        /* Retrieve the number of messages received from this queue.  */
        if (messages_received != TX_NULL)
        {
    
            *messages_received =  queue_ptr -> tx_queue_performance_messages_received_count;
        }
    
        /* Retrieve the number of empty queue suspensions on this queue.  */
        if (empty_suspensions != TX_NULL)
        {
    
            *empty_suspensions =  queue_ptr -> tx_queue_performance_empty_suspension_count;
        }
    
        /* Retrieve the number of full queue suspensions on this queue.  */
        if (full_suspensions != TX_NULL)
        {
    
            *full_suspensions =  queue_ptr -> tx_queue_performance_full_suspension_count;
        }
    
        /* Retrieve the number of full errors (no suspension!) on this queue.  */
        if (full_errors != TX_NULL)
        {
    
            *full_errors =  queue_ptr -> tx_queue_performance_full_error_count;
        }
    
        /* Retrieve the number of timeouts on this queue.  */
        if (timeouts != TX_NULL)
        {
    
            *timeouts =  queue_ptr -> tx_queue_performance_timeout_count;
        }
    
        /* Restore interrupts.  */
        TX_RESTORE

        /* Return completion status.  */
        status =  TX_SUCCESS;
    }
#else
UINT                    status;


    /* Access input arguments just for the sake of lint, MISRA, etc.  */
    if (queue_ptr != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (messages_sent != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (messages_received != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (empty_suspensions != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (full_suspensions != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (full_errors != TX_NULL)
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

