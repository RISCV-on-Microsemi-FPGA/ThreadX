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
/**   Byte Memory                                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_byte_pool.h"
#ifdef TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO
#include "tx_trace.h"
#endif

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_byte_pool_performance_system_info_get           PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves byte pool performance information.          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    allocates                         Destination for total number of   */ 
/*                                        allocates                       */ 
/*    releases                          Destination for total number of   */ 
/*                                        releases                        */ 
/*    fragments_searched                Destination for total number of   */ 
/*                                        fragments searched during       */ 
/*                                        allocation                      */ 
/*    merges                            Destination for total number of   */ 
/*                                        adjacent free fragments merged  */ 
/*    splits                            Destination for total number of   */ 
/*                                        fragments split during          */ 
/*                                        allocation                      */ 
/*    suspensions                       Destination for total number of   */ 
/*                                        suspensions                     */
/*    timeouts                          Destination for total number of   */ 
/*                                        timeouts                        */ 
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
/*  12-12-2009     William E. Lamie         Modified comment(s), added    */ 
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
UINT  _tx_byte_pool_performance_system_info_get(ULONG *allocates, ULONG *releases,
                    ULONG *fragments_searched, ULONG *merges, ULONG *splits, ULONG *suspensions, ULONG *timeouts)
{

#ifdef TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO

TX_INTERRUPT_SAVE_AREA


    /* Disable interrupts.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_BYTE_POOL__PERFORMANCE_SYSTEM_INFO_GET, 0, 0, 0, 0, TX_TRACE_BYTE_POOL_EVENTS)

    /* Log this kernel call.  */
    TX_EL_BYTE_POOL_PERFORMANCE_SYSTEM_INFO_GET_INSERT

    /* Retrieve all the pertinent information and return it in the supplied
       destinations.  */

    /* Retrieve the total number of byte pool allocates.  */
    if (allocates != TX_NULL)
    {
    
        *allocates =  _tx_byte_pool_performance_allocate_count;
    }

    /* Retrieve the total number of byte pool releases.  */
    if (releases != TX_NULL)
    {
    
        *releases =  _tx_byte_pool_performance_release_count;
    }

    /* Retrieve the total number of byte pool fragments searched.  */
    if (fragments_searched != TX_NULL)
    {
    
        *fragments_searched =  _tx_byte_pool_performance_search_count;
    }

    /* Retrieve the total number of byte pool fragments merged.  */
    if (merges != TX_NULL)
    {
    
        *merges =  _tx_byte_pool_performance_merge_count;
    }

    /* Retrieve the total number of byte pool fragment splits.  */
    if (splits != TX_NULL)
    {
    
        *splits =  _tx_byte_pool_performance_split_count;
    }

    /* Retrieve the total number of byte pool suspensions.  */
    if (suspensions != TX_NULL)
    {
    
        *suspensions =  _tx_byte_pool_performance_suspension_count;
    }
    
    /* Retrieve the total number of byte pool timeouts.  */
    if (timeouts != TX_NULL)
    {
    
        *timeouts =  _tx_byte_pool_performance_timeout_count;
    }
    
    /* Restore interrupts.  */
    TX_RESTORE

    /* Return completion status.  */
    return(TX_SUCCESS);
    
#else

UINT        status;


    /* Access input arguments just for the sake of lint, MISRA, etc.  */
    if (allocates != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (releases != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (fragments_searched != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (merges != TX_NULL)
    {

        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (splits != TX_NULL)
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
    
    /* Return completion status.  */
    return(status);
#endif
}

