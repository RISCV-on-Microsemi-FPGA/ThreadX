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
/**   Thread                                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"
#include "tx_thread.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_thread_delete                                   PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function handles application delete thread requests.  The      */ 
/*    thread to delete must be in a terminated or completed state,        */ 
/*    otherwise this function just returns an error code.                 */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    thread_ptr                            Pointer to thread to suspend  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Return completion status      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
/*                                            insert, moved extension     */ 
/*                                            processing to interrupt     */ 
/*                                            enabled area, and made      */ 
/*                                            several optimizations,      */ 
/*                                            resulting in version 5.2    */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), merged   */ 
/*                                            event logging support,      */ 
/*                                            removed compound            */ 
/*                                            conditionals, and           */ 
/*                                            eliminated created_count    */ 
/*                                            local variable, resulting   */ 
/*                                            in version 5.4              */ 
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
UINT  _tx_thread_delete(TX_THREAD *thread_ptr)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD       *next_thread;
TX_THREAD       *previous_thread;
UINT            status;


    /* Default status to success.  */
    status =  TX_SUCCESS;
    
    /* Lockout interrupts while the thread is being deleted.  */
    TX_DISABLE

    /* Check for proper status of this thread to delete.  */
    if (thread_ptr -> tx_thread_state != TX_COMPLETED) 
    {

        /* Now check for terminated state.  */
        if (thread_ptr -> tx_thread_state != TX_TERMINATED)
        {

            /* Restore interrupts.  */
            TX_RESTORE

            /* Thread not completed or terminated - return an error!  */
            status =  TX_DELETE_ERROR;
        }
    }

    /* Determine if the delete operation is okay.  */
    if (status == TX_SUCCESS)
    {
    
        /* Yes, continue with deleting the thread.  */

        /* Perform any additional activities for tool or user purpose.  */
        TX_THREAD_DELETE_EXTENSION(thread_ptr)

        /* If trace is enabled, insert this event into the trace buffer.  */
        TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_DELETE, thread_ptr, TX_POINTER_TO_ULONG_CONVERT(&next_thread), 0, 0, TX_TRACE_THREAD_EVENTS)

        /* If trace is enabled, unregister this object.  */
        TX_TRACE_OBJECT_UNREGISTER(thread_ptr)

        /* Log this kernel call.  */
        TX_EL_THREAD_DELETE_INSERT

        /* Unregister thread in the thread array structure.  */
        TX_EL_THREAD_UNREGISTER(thread_ptr)

        /* Clear the thread ID to make it invalid.  */
        thread_ptr -> tx_thread_id =  TX_CLEAR_ID;

        /* Decrement the number of created threads.  */
        _tx_thread_created_count--;
        
        /* See if the thread is the only one on the list.  */
        if (_tx_thread_created_count == TX_EMPTY)
        {

            /* Only created thread, just set the created list to NULL.  */
            _tx_thread_created_ptr =  TX_NULL;
        }
        else
        {

            /* Otherwise, not the only created thread, link-up the neighbors.  */
            next_thread =                                thread_ptr -> tx_thread_created_next;
            previous_thread =                            thread_ptr -> tx_thread_created_previous;
            next_thread -> tx_thread_created_previous =  previous_thread;
            previous_thread -> tx_thread_created_next =  next_thread;

            /* See if we have to update the created list head pointer.  */
            if (_tx_thread_created_ptr == thread_ptr)
            {
                        
                /* Yes, move the head pointer to the next link. */
                _tx_thread_created_ptr =  next_thread; 
            }
        }

        /* Restore interrupts.  */
        TX_RESTORE
    }

    /* Return completion status.  */
    return(status);
}

