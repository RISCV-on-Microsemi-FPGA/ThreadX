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
#include "tx_trace.h"
#include "tx_queue.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_queue_create                                    PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function creates a message queue.  The message size and depth  */ 
/*    of the queue is specified by the caller.                            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    queue_ptr                         Pointer to queue control block    */ 
/*    name_ptr                          Pointer to queue name             */ 
/*    message_size                      Size of each queue message        */ 
/*    queue_start                       Starting address of the queue area*/ 
/*    queue_size                        Number of bytes in the queue      */ 
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
/*                                            parameter to trace          */ 
/*                                            registry, added filter      */ 
/*                                            option to trace insert,     */ 
/*                                            and made several            */ 
/*                                            optimizations, resulting    */ 
/*                                            in version 5.2              */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), changed  */ 
/*                                            memset to macro, casted     */ 
/*                                            ULONG counting values to    */ 
/*                                            UINT where necessary,       */ 
/*                                            merged event logging        */ 
/*                                            support, and eliminated     */ 
/*                                            created_count local         */ 
/*                                            variable, resulting         */ 
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
UINT  _tx_queue_create(TX_QUEUE *queue_ptr, CHAR *name_ptr, UINT message_size, 
                        VOID *queue_start, ULONG queue_size)
{

TX_INTERRUPT_SAVE_AREA

UINT            capacity;                       
UINT            used_words;                     
TX_QUEUE        *next_queue;
TX_QUEUE        *previous_queue;


    /* Initialize queue control block to all zeros.  */
    TX_MEMSET(queue_ptr, 0, (sizeof(TX_QUEUE)));

    /* Setup the basic queue fields.  */
    queue_ptr -> tx_queue_name =             name_ptr;
    
    /* Save the message size in the control block.  */
    queue_ptr -> tx_queue_message_size =  message_size;

    /* Determine how many messages will fit in the queue area and the number
       of ULONGs used.  */
    capacity =    (UINT) (queue_size / ((ULONG) (((ULONG) message_size) * (sizeof(ULONG)))));
    used_words =  capacity * message_size;

    /* Save the starting address and calculate the ending address of 
       the queue.  Note that the ending address is really one past the
       end!  */
    queue_ptr -> tx_queue_start =  (ULONG *) queue_start;
    queue_ptr -> tx_queue_end =    TX_ULONG_POINTER_ADD(queue_start, used_words);

    /* Set the read and write pointers to the beginning of the queue
       area.  */
    queue_ptr -> tx_queue_read =   (ULONG *) queue_start;
    queue_ptr -> tx_queue_write =  (ULONG *) queue_start;

    /* Setup the number of enqueued messages and the number of message
       slots available in the queue.  */
    queue_ptr -> tx_queue_available_storage =  (UINT) capacity;
    queue_ptr -> tx_queue_capacity =           (UINT) capacity;

    /* Disable interrupts to put the queue on the created list.  */
    TX_DISABLE

    /* Setup the queue ID to make it valid.  */
    queue_ptr -> tx_queue_id =  TX_QUEUE_ID;

    /* Place the queue on the list of created queues.  First,
       check for an empty list.  */
    if (_tx_queue_created_count == TX_EMPTY)
    {

        /* The created queue list is empty.  Add queue to empty list.  */
        _tx_queue_created_ptr =                   queue_ptr;
        queue_ptr -> tx_queue_created_next =      queue_ptr;
        queue_ptr -> tx_queue_created_previous =  queue_ptr;
    }
    else
    {

        /* This list is not NULL, add to the end of the list.  */
        next_queue =      _tx_queue_created_ptr;
        previous_queue =  next_queue -> tx_queue_created_previous;

        /* Place the new queue in the list.  */
        next_queue -> tx_queue_created_previous =  queue_ptr;
        previous_queue -> tx_queue_created_next =  queue_ptr;

        /* Setup this queues's created links.  */
        queue_ptr -> tx_queue_created_previous =  previous_queue;
        queue_ptr -> tx_queue_created_next =      next_queue;    
    }

    /* Increment the created queue count.  */
    _tx_queue_created_count++;

    /* Optional queue create extended processing.  */
    TX_QUEUE_CREATE_EXTENSION(queue_ptr)

    /* If trace is enabled, register this object.  */
    TX_TRACE_OBJECT_REGISTER(TX_TRACE_OBJECT_TYPE_QUEUE, queue_ptr, name_ptr, queue_size, message_size)

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_QUEUE_CREATE, queue_ptr, message_size, TX_POINTER_TO_ULONG_CONVERT(queue_start), queue_size, TX_TRACE_QUEUE_EVENTS)

    /* Log this kernel call.  */
    TX_EL_QUEUE_CREATE_INSERT

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return TX_SUCCESS.  */
    return(TX_SUCCESS);
}

