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
/**   Trace                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#ifndef TX_SOURCE_CODE
#define TX_SOURCE_CODE
#endif


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_trace_event_filter                              PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets up the event filter, which allows the            */ 
/*    application to filter various trace events during run-time.         */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    event_filter_bits                     Trace filter event bit(s)     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */ 
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
/*  12-12-2008     William E. Lamie         Initial Version 5.2           */ 
/*  07-04-2009     William E. Lamie         Modified comment(s), and      */ 
/*                                            added conditional so that   */ 
/*                                            the function is only        */ 
/*                                            compiled once, resulting in */ 
/*                                            version 5.3                 */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), and added*/ 
/*                                            logic to access parameter,  */ 
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
UINT  _tx_trace_event_filter(ULONG event_filter_bits)
{

#ifdef TX_ENABLE_EVENT_TRACE

    /* Apply the specified filter by clearing the enable bits.  */
    _tx_trace_event_enable_bits =  _tx_trace_event_enable_bits & (~event_filter_bits);

    /* Return success.  */
    return(TX_SUCCESS);

#else

UINT        status;


    /* Access input arguments just for the sake of lint, MISRA, etc.  */
    if (event_filter_bits != ((ULONG) 0))
    {
    
        /* Trace not enabled, return an error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else
    {
    
        /* Trace not enabled, return an error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    
    /* Return completion status.  */
    return(status);
#endif
}

