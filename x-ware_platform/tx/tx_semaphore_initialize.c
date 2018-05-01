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
#include "tx_semaphore.h"


#ifndef TX_INLINE_INITIALIZATION

/* Locate semaphore component data in this file.  */

/* Define the head pointer of the created semaphore list.  */

TX_SEMAPHORE *   _tx_semaphore_created_ptr;


/* Define the variable that holds the number of created semaphores. */

ULONG            _tx_semaphore_created_count;


#ifdef TX_SEMAPHORE_ENABLE_PERFORMANCE_INFO

/* Define the total number of semaphore puts.  */

ULONG            _tx_semaphore_performance_put_count;


/* Define the total number of semaphore gets.  */

ULONG            _tx_semaphore_performance_get_count;


/* Define the total number of semaphore suspensions.  */

ULONG            _tx_semaphore_performance_suspension_count;


/* Define the total number of semaphore timeouts.  */

ULONG            _tx_semaphore_performance_timeout_count;

#endif
#endif


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_semaphore_initialize                            PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function initializes the various control data structures for   */ 
/*    the semaphore component.                                            */ 
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
VOID  _tx_semaphore_initialize(VOID)
{

#ifndef TX_DISABLE_REDUNDANT_CLEARING

    /* Initialize the head pointer of the created semaphores list and the
       number of semaphores created.  */
    _tx_semaphore_created_ptr =        TX_NULL;
    _tx_semaphore_created_count =      TX_EMPTY;

#ifdef TX_SEMAPHORE_ENABLE_PERFORMANCE_INFO

    /* Initialize semaphore performance counters.  */
    _tx_semaphore_performance_put_count =         ((ULONG) 0);
    _tx_semaphore_performance_get_count =         ((ULONG) 0);
    _tx_semaphore_performance_suspension_count =  ((ULONG) 0);
    _tx_semaphore_performance_timeout_count =     ((ULONG) 0);
#endif
#endif
}

