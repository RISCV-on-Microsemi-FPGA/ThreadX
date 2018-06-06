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
/*    _tx_thread_reset                                    PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function prepares the thread to run again from the entry       */ 
/*    point specified during thread creation. The application must        */ 
/*    call tx_thread_resume after this call completes for the thread      */ 
/*    to actually run.                                                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    thread_ptr                            Pointer to thread to reset    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Service return status         */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _tx_thread_stack_build                Build initial thread stack    */ 
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
/*                                            insert, and added macro to  */ 
/*                                            get current thread,         */ 
/*                                            resulting in version 5.2    */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), merged   */ 
/*                                            event logging support, and  */ 
/*                                            removed compound            */ 
/*                                            conditionals, resulting in  */ 
/*                                            version 5.4                 */ 
/*  07-15-2011     William E. Lamie         Modified comment(s), and      */ 
/*                                            changed memset to TX_MEMSET */ 
/*                                            so it can be redefined,     */ 
/*                                            resulting in version 5.5    */ 
/*  11-01-2012     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.6    */ 
/*  05-01-2015     William E. Lamie         Modified comment(s), and      */ 
/*                                            modified code for MISRA     */ 
/*                                            compliance, resulting in    */ 
/*                                            version 5.7                 */ 
/*                                                                        */ 
/**************************************************************************/ 
UINT  _tx_thread_reset(TX_THREAD *thread_ptr)
{

TX_INTERRUPT_SAVE_AREA

TX_THREAD       *current_thread;
UINT            status;


    /* Default a successful completion status.  */
    status =  TX_SUCCESS;

    /* Disable interrupts.  */
    TX_DISABLE

    /* Pickup thread pointer.  */
    TX_THREAD_GET_CURRENT(current_thread)

    /* Check for a call from the current thread, which is not allowed!  */
    if (current_thread == thread_ptr)
    {

        /* Restore interrupts.  */
        TX_RESTORE

        /* Thread not completed or terminated - return an error!  */
        status =  TX_NOT_DONE;
    }
    else
    {

        /* Check for proper status of this thread to reset.  */
        if (thread_ptr -> tx_thread_state != TX_COMPLETED) 
        {

            /* Now check for terminated state.  */
            if (thread_ptr -> tx_thread_state != TX_TERMINATED)
            {

                /* Thread not completed or terminated - return an error!  */
                status =  TX_NOT_DONE;
            }
        }
    }

    /* Is the request valid?  */
    if (status == TX_SUCCESS)
    {

        /* Modify the thread status to prevent additional reset calls.  */
        thread_ptr -> tx_thread_state =  TX_NOT_DONE;

        /* Restore interrupts.  */
        TX_RESTORE

#ifndef TX_DISABLE_STACK_FILLING

        /* Set the thread stack to a pattern prior to creating the initial
           stack frame.  This pattern is used by the stack checking routines
           to see how much has been used.  */
        TX_MEMSET(thread_ptr -> tx_thread_stack_start, ((UCHAR) TX_STACK_FILL), thread_ptr -> tx_thread_stack_size);
#endif

        /* Call the target specific stack frame building routine to build the 
           thread's initial stack and to setup the actual stack pointer in the
           control block.  */
        _tx_thread_stack_build(thread_ptr, _tx_thread_shell_entry);

        /* Disable interrupts.  */
        TX_DISABLE

        /* Finally, move into a suspended state to allow for the thread to be resumed.  */
        thread_ptr -> tx_thread_state =  TX_SUSPENDED;

        /* If trace is enabled, insert this event into the trace buffer.  */
        TX_TRACE_IN_LINE_INSERT(TX_TRACE_THREAD_RESET, thread_ptr, thread_ptr -> tx_thread_state, 0, 0, TX_TRACE_THREAD_EVENTS)

        /* Log this kernel call.  */
        TX_EL_THREAD_RESET_INSERT

        /* Log the thread status change.  */
        TX_EL_THREAD_STATUS_CHANGE_INSERT(thread_ptr, TX_SUSPENDED)
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return completion status to caller.  */
    return(status);
}

