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
/**   Semaphore                                                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"
#include "tx_semaphore.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_semaphore_create                                PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function creates a counting semaphore with the initial count   */ 
/*    specified in this call.                                             */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    semaphore_ptr                     Pointer to semaphore control block*/ 
/*    name_ptr                          Pointer to semaphore name         */ 
/*    initial_count                     Initial semaphore count           */ 
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
/*  11-01-2012     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.6    */ 
/*  05-01-2015     William E. Lamie         Modified comment(s), and      */ 
/*                                            modified code for MISRA     */ 
/*                                            compliance, resulting in    */ 
/*                                            version 5.7                 */ 
/*                                                                        */ 
/**************************************************************************/ 
UINT  _tx_semaphore_create(TX_SEMAPHORE *semaphore_ptr, CHAR *name_ptr, ULONG initial_count)
{

TX_INTERRUPT_SAVE_AREA

TX_SEMAPHORE    *next_semaphore;
TX_SEMAPHORE    *previous_semaphore;


    /* Initialize semaphore control block to all zeros.  */
    TX_MEMSET(semaphore_ptr, 0, (sizeof(TX_SEMAPHORE)));

    /* Setup the basic semaphore fields.  */
    semaphore_ptr -> tx_semaphore_name =             name_ptr;
    semaphore_ptr -> tx_semaphore_count =            initial_count;
    
    /* Disable interrupts to place the semaphore on the created list.  */
    TX_DISABLE

    /* Setup the semaphore ID to make it valid.  */
    semaphore_ptr -> tx_semaphore_id =  TX_SEMAPHORE_ID;

    /* Place the semaphore on the list of created semaphores.  First,
       check for an empty list.  */
    if (_tx_semaphore_created_count == TX_EMPTY)
    {

        /* The created semaphore list is empty.  Add semaphore to empty list.  */
        _tx_semaphore_created_ptr =                       semaphore_ptr;
        semaphore_ptr -> tx_semaphore_created_next =      semaphore_ptr;
        semaphore_ptr -> tx_semaphore_created_previous =  semaphore_ptr;
    }
    else
    {

        /* This list is not NULL, add to the end of the list.  */
        next_semaphore =      _tx_semaphore_created_ptr;
        previous_semaphore =  next_semaphore -> tx_semaphore_created_previous;

        /* Place the new semaphore in the list.  */
        next_semaphore -> tx_semaphore_created_previous =  semaphore_ptr;
        previous_semaphore -> tx_semaphore_created_next =  semaphore_ptr;

        /* Setup this semaphore's next and previous created links.  */
        semaphore_ptr -> tx_semaphore_created_previous =  previous_semaphore;
        semaphore_ptr -> tx_semaphore_created_next =      next_semaphore;    
    }
    
    /* Increment the created count.  */
    _tx_semaphore_created_count++;

    /* Optional semaphore create extended processing.  */
    TX_SEMAPHORE_CREATE_EXTENSION(semaphore_ptr)

    /* If trace is enabled, register this object.  */
    TX_TRACE_OBJECT_REGISTER(TX_TRACE_OBJECT_TYPE_SEMAPHORE, semaphore_ptr, name_ptr, initial_count, 0)

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_SEMAPHORE_CREATE, semaphore_ptr, initial_count, TX_POINTER_TO_ULONG_CONVERT(&next_semaphore), 0, TX_TRACE_SEMAPHORE_EVENTS)

    /* Log this kernel call.  */
    TX_EL_SEMAPHORE_CREATE_INSERT

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return TX_SUCCESS.  */
    return(TX_SUCCESS);
}

