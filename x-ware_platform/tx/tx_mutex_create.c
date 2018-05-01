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
/**   Mutex                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_thread.h"
#include "tx_trace.h"
#include "tx_mutex.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_mutex_create                                    PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function creates a mutex with optional priority inheritance as */ 
/*    specified in this call.                                             */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    mutex_ptr                             Pointer to mutex control block*/ 
/*    name_ptr                              Pointer to mutex name         */ 
/*    inherit                               Priority inheritance option   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    TX_SUCCESS                        Successful completion status      */ 
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
/*  12-12-2008     William E. Lamie         Modified comment(s), added    */ 
/*                                            filter option to trace      */ 
/*                                            insert, and made several    */ 
/*                                            optimizations, resulting    */ 
/*                                            in version 5.2              */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), changed  */ 
/*                                            memset to macro, merged     */ 
/*                                            event logging support, and  */ 
/*                                            eliminated created_count    */ 
/*                                            local variable, resulting   */ 
/*                                            in version 5.4              */ 
/*  07-15-2011     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.5    */ 
/*  11-01-2012     William E. Lamie         Modified comment(s), and      */ 
/*                                            setup thread mutex release  */ 
/*                                            function pointer,           */ 
/*                                            resulting in version 5.6    */ 
/*  05-01-2015     William E. Lamie         Modified comment(s), and      */ 
/*                                            modified code for MISRA     */ 
/*                                            compliance, resulting in    */ 
/*                                            version 5.7                 */ 
/*                                                                        */ 
/**************************************************************************/ 
UINT  _tx_mutex_create(TX_MUTEX *mutex_ptr, CHAR *name_ptr, UINT inherit)
{

TX_INTERRUPT_SAVE_AREA

TX_MUTEX        *next_mutex;
TX_MUTEX        *previous_mutex;


    /* Initialize mutex control block to all zeros.  */
    TX_MEMSET(mutex_ptr, 0, (sizeof(TX_MUTEX)));

    /* Setup the basic mutex fields.  */
    mutex_ptr -> tx_mutex_name =             name_ptr;
    mutex_ptr -> tx_mutex_inherit =          inherit;
    
    /* Disable interrupts to place the mutex on the created list.  */
    TX_DISABLE

    /* Setup the mutex ID to make it valid.  */
    mutex_ptr -> tx_mutex_id =  TX_MUTEX_ID;

    /* Setup the thread mutex release function pointer.  */
    _tx_thread_mutex_release =  &(_tx_mutex_thread_release);

    /* Place the mutex on the list of created mutexes.  First,
       check for an empty list.  */
    if (_tx_mutex_created_count == TX_EMPTY)
    {

        /* The created mutex list is empty.  Add mutex to empty list.  */
        _tx_mutex_created_ptr =                   mutex_ptr;
        mutex_ptr -> tx_mutex_created_next =      mutex_ptr;
        mutex_ptr -> tx_mutex_created_previous =  mutex_ptr;
    }
    else
    {

        /* This list is not NULL, add to the end of the list.  */
        next_mutex =      _tx_mutex_created_ptr;
        previous_mutex =  next_mutex -> tx_mutex_created_previous;

        /* Place the new mutex in the list.  */
        next_mutex -> tx_mutex_created_previous =  mutex_ptr;
        previous_mutex -> tx_mutex_created_next =  mutex_ptr;

        /* Setup this mutex's next and previous created links.  */
        mutex_ptr -> tx_mutex_created_previous =  previous_mutex;
        mutex_ptr -> tx_mutex_created_next =      next_mutex;    
    }

    /* Increment the ownership count.  */
    _tx_mutex_created_count++;
    
    /* Optional mutex create extended processing.  */
    TX_MUTEX_CREATE_EXTENSION(mutex_ptr)

    /* If trace is enabled, register this object.  */
    TX_TRACE_OBJECT_REGISTER(TX_TRACE_OBJECT_TYPE_MUTEX, mutex_ptr, name_ptr, inherit, 0)

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_MUTEX_CREATE, mutex_ptr, inherit, TX_POINTER_TO_ULONG_CONVERT(&next_mutex), 0, TX_TRACE_MUTEX_EVENTS)

    /* Log this kernel call.  */
    TX_EL_MUTEX_CREATE_INSERT

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return TX_SUCCESS.  */
    return(TX_SUCCESS);
}

