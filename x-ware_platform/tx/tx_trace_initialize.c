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

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"
#ifdef TX_ENABLE_EVENT_TRACE


/* Define the pointer to the start of the trace buffer control structure.   */

TX_TRACE_HEADER                   *_tx_trace_header_ptr;


/* Define the pointer to the start of the trace object registry area in the trace buffer.  */

TX_TRACE_OBJECT_ENTRY             *_tx_trace_registry_start_ptr;


/* Define the pointer to the end of the trace object registry area in the trace buffer.  */

TX_TRACE_OBJECT_ENTRY             *_tx_trace_registry_end_ptr;


/* Define the pointer to the starting entry of the actual trace event area of the trace buffer.  */

TX_TRACE_BUFFER_ENTRY             *_tx_trace_buffer_start_ptr;


/* Define the pointer to the ending entry of the actual trace event area of the trace buffer.  */

TX_TRACE_BUFFER_ENTRY             *_tx_trace_buffer_end_ptr;


/* Define the pointer to the current entry of the actual trace event area of the trace buffer.  */

TX_TRACE_BUFFER_ENTRY             *_tx_trace_buffer_current_ptr;


/* Define the trace event enable bits, where each bit represents a type of event that can be enabled 
   or disabled dynamically by the application.  */ 

ULONG                            _tx_trace_event_enable_bits;


/* Define a counter that is used in environments that don't have a timer source. This counter
   is incremented on each use giving each event a unique timestamp.  */

ULONG                             _tx_trace_simulated_time;


/* Define the function pointer used to call the application when the trace buffer wraps. If NULL, 
   the application has not registered a callback function.  */
   
VOID                             (*_tx_trace_full_notify_function)(VOID *buffer);


/* Define the total number of registry entries.  */

ULONG                             _tx_trace_total_registry_entries;


/* Define a counter that is used to track the number of available registry entries.  */

ULONG                             _tx_trace_available_registry_entries;


/* Define an index that represents the start of the registry search.  */

ULONG                             _tx_trace_registry_search_start;

#endif


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_trace_initialize                                PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function initializes the various control data structures for   */ 
/*    the trace component.                                                */ 
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
/*    None                                                                */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _tx_initialize_high_level         High level initialization         */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-12-2005     William E. Lamie         Initial Version 5.0           */ 
/*  04-02-2007     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.1    */ 
/*  12-12-2008     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.2    */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s),          */ 
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
VOID  _tx_trace_initialize(VOID)
{

#ifdef TX_ENABLE_EVENT_TRACE
#ifndef TX_DISABLE_REDUNDANT_CLEARING

    /* Initialize all the pointers to the trace buffer to NULL.  */
    _tx_trace_header_ptr =          TX_NULL;
    _tx_trace_registry_start_ptr =  TX_NULL;
    _tx_trace_registry_end_ptr =    TX_NULL;
    _tx_trace_buffer_start_ptr =    TX_NULL;
    _tx_trace_buffer_end_ptr =      TX_NULL;
    _tx_trace_buffer_current_ptr =  TX_NULL;
#endif
#endif
}

