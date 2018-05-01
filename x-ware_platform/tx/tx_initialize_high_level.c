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
/**   Initialize                                                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"

/* Determine if in-line initialization is required.  */
#ifdef TX_INLINE_INITIALIZATION
#define TX_INVOKE_INLINE_INITIALIZATION
#endif

#include "tx_initialize.h"
#include "tx_thread.h"
#include "tx_timer.h"
#include "tx_semaphore.h"
#include "tx_queue.h"
#include "tx_event_flags.h"
#include "tx_mutex.h"
#include "tx_block_pool.h"
#include "tx_byte_pool.h"


/* Define the unused memory pointer.  The value of the first available 
   memory address is placed in this variable in the low-level
   initialization function.  The content of this variable is passed 
   to the application's system definition function.  */

VOID     *_tx_initialize_unused_memory;


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_initialize_high_level                           PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is responsible for initializing all of the other      */ 
/*    components in the ThreadX real-time kernel.                         */
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
/*    _tx_thread_initialize             Initialize the thread control     */ 
/*                                        component                       */ 
/*    _tx_timer_initialize              Initialize the timer control      */ 
/*                                        component                       */ 
/*    _tx_semaphore_initialize          Initialize the semaphore control  */ 
/*                                        component                       */ 
/*    _tx_queue_initialize              Initialize the queue control      */ 
/*                                        component                       */ 
/*    _tx_event_flags_initialize        Initialize the event flags control*/ 
/*                                        component                       */ 
/*    _tx_block_pool_initialize         Initialize the block pool control */ 
/*                                        component                       */ 
/*    _tx_byte_pool_initialize          Initialize the byte pool control  */ 
/*                                        component                       */ 
/*    _tx_mutex_initialize              Initialize the mutex control      */ 
/*                                        component                       */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _tx_initialize_kernel_enter       Kernel entry function             */ 
/*    _tx_initialize_kernel_setup       Early kernel setup function that  */ 
/*                                        is optionally called by         */ 
/*                                        compiler's startup code.        */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-12-2005     William E. Lamie         Initial Version 5.0           */ 
/*  04-02-2007     William E. Lamie         Modified comment(s), and      */ 
/*                                            added logic to remove the   */ 
/*                                            timer logic, resulting      */ 
/*                                            in version 5.1              */ 
/*  12-12-2008     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.2    */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), and      */ 
/*                                            merged event logging        */ 
/*                                            support, resulting in       */ 
/*                                            version 5.4                 */ 
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
VOID    _tx_initialize_high_level(VOID)
{

    /* Initialize event tracing, if enabled.  */
    TX_TRACE_INITIALIZE

    /* Initialize the event log, if enabled.  */
    TX_EL_INITIALIZE

    /* Call the thread control initialization function.  */
    _tx_thread_initialize();

#ifndef TX_NO_TIMER

    /* Call the timer control initialization function.  */
    _tx_timer_initialize();
#endif

#ifndef TX_DISABLE_REDUNDANT_CLEARING

    /* Call the semaphore initialization function.  */
    _tx_semaphore_initialize();

    /* Call the queue initialization function.  */
    _tx_queue_initialize();

    /* Call the event flag initialization function.  */
    _tx_event_flags_initialize();

    /* Call the block pool initialization function.  */
    _tx_block_pool_initialize();

    /* Call the byte pool initialization function.  */
    _tx_byte_pool_initialize();

    /* Call the mutex initialization function.  */
    _tx_mutex_initialize();
#endif
}

