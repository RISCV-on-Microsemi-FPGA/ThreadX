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


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_trace_object_unregister                         PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function unregisters a ThreadX system object from the trace    */ 
/*    registry area.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    object_pointer                        Address of system object      */ 
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
/*                                            modified code to ensure     */ 
/*                                            universal trace format,     */ 
/*                                            resulting in version 5.2    */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), added    */ 
/*                                            logic to access parameter,  */ 
/*                                            resulting in version 5.4    */ 
/*  07-15-2011     William E. Lamie         Modified comment(s), and      */ 
/*                                            added support for optimizing*/ 
/*                                            adding registry entries,    */ 
/*                                            resulting in version 5.5    */ 
/*  11-01-2012     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.6    */ 
/*  05-01-2015     William E. Lamie         Modified comment(s), and      */ 
/*                                            modified code for MISRA     */ 
/*                                            compliance, resulting in    */ 
/*                                            version 5.7                 */ 
/*                                                                        */ 
/**************************************************************************/ 
VOID  _tx_trace_object_unregister(VOID *object_ptr)
{

#ifdef TX_ENABLE_EVENT_TRACE

UINT                            i, entries;
UCHAR                           *work_ptr;
TX_TRACE_OBJECT_ENTRY           *entry_ptr;


    /* Determine if the registry area is setup.  */
    if (_tx_trace_registry_start_ptr != TX_NULL)
    {

        /* Registry is setup, proceed.  */

        /* Pickup the total entries.  */
        entries =  _tx_trace_total_registry_entries;

        /* Loop to find available entry.  */
        for (i = ((ULONG) 0); i < entries; i++)
        {

            /* Setup the registry entry pointer.  */
            work_ptr =  ((VOID *) _tx_trace_registry_start_ptr);
            work_ptr =  TX_UCHAR_POINTER_ADD(work_ptr, ((sizeof(TX_TRACE_OBJECT_ENTRY))*i));
            entry_ptr =  (TX_TRACE_OBJECT_ENTRY *) ((VOID *) work_ptr);

            /* Determine if this entry matches the object pointer... */
            if (entry_ptr -> tx_trace_object_entry_thread_pointer == TX_POINTER_TO_ULONG_CONVERT(object_ptr))
            {

                /* Mark this entry as available, but leave the other information so that old trace entries can 
                   still find it - if necessary!  */
                entry_ptr -> tx_trace_object_entry_available =  ((UCHAR) TX_TRUE);

                /* Increment the number of available registry entries.  */
                _tx_trace_available_registry_entries++;

                /* Adjust the search index to this position.  */
                _tx_trace_registry_search_start =  i;

                break;
            }
        }
    }
#else

TX_INTERRUPT_SAVE_AREA


    /* Access input arguments just for the sake of lint, MISRA, etc.  */
    if (object_ptr != TX_NULL)
    {
        
        /* NOP code.  */
        TX_DISABLE
        TX_RESTORE
    }
#endif
}

