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
/**   Timer                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_thread.h"
#include "tx_timer.h"


/* Define the system clock value that is continually incremented by the 
   periodic timer interrupt processing.  */

volatile ULONG      _tx_timer_system_clock;


/* Define the current time slice value.  If non-zero, a time-slice is active.
   Otherwise, the time_slice is not active.  */

ULONG               _tx_timer_time_slice;


/* Define the time-slice expiration flag.  This is used to indicate that a time-slice
   has happened.  */

UINT                _tx_timer_expired_time_slice;


/* Define the thread and application timer entry list.  This list provides a direct access
   method for insertion of times less than TX_TIMER_ENTRIES.  */

TX_TIMER_INTERNAL   *_tx_timer_list[TX_TIMER_ENTRIES];


/* Define the boundary pointers to the list.  These are setup to easily manage
   wrapping the list.  */

TX_TIMER_INTERNAL   **_tx_timer_list_start;
TX_TIMER_INTERNAL   **_tx_timer_list_end;


/* Define the current timer pointer in the list.  This pointer is moved sequentially
   through the timer list by the timer interrupt handler.  */

TX_TIMER_INTERNAL   **_tx_timer_current_ptr;


/* Define the timer expiration flag.  This is used to indicate that a timer 
   has expired.  */

UINT                _tx_timer_expired;


/* Define the created timer list head pointer.  */

TX_TIMER            *_tx_timer_created_ptr;


/* Define the created timer count.  */

ULONG               _tx_timer_created_count;


#ifndef TX_TIMER_PROCESS_IN_ISR

/* Define the timer thread's control block.  */

TX_THREAD           _tx_timer_thread;


/* Define the variable that holds the timer thread's starting stack address.  */

VOID                *_tx_timer_stack_start;


/* Define the variable that holds the timer thread's stack size.  */

ULONG               _tx_timer_stack_size;


/* Define the variable that holds the timer thread's priority.  */

UINT                _tx_timer_priority;

/* Define the system timer thread's stack.   The default size is defined
   in tx_port.h.  */

ULONG               _tx_timer_thread_stack_area[(((UINT) TX_TIMER_THREAD_STACK_SIZE)+((sizeof(ULONG))- ((UINT) 1)))/(sizeof(ULONG))];

#else


/* Define the busy flag that will prevent nested timer ISR processing.  */

UINT                _tx_timer_processing_active;

#endif

#ifdef TX_TIMER_ENABLE_PERFORMANCE_INFO

/* Define the total number of timer activations.  */

ULONG               _tx_timer_performance_activate_count;


/* Define the total number of timer reactivations.  */

ULONG               _tx_timer_performance_reactivate_count;


/* Define the total number of timer deactivations.  */

ULONG               _tx_timer_performance_deactivate_count;


/* Define the total number of timer expirations.  */

ULONG               _tx_timer_performance_expiration_count;


/* Define the total number of timer expiration adjustments. These are required
   if the expiration time is greater than the size of the timer list. In such 
   cases, the timer is placed at the end of the list and then reactivated 
   as many times as necessary to finally achieve the resulting timeout. */

ULONG               _tx_timer_performance__expiration_adjust_count;

#endif


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_timer_initialize                                PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function initializes the various control data structures for   */ 
/*    the clock control component.                                        */ 
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
/*    _tx_thread_create                 Create the system timer thread    */ 
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
/*  12-12-2009     William E. Lamie         Modified comment(s), corrected*/ 
/*                                            memory initialize value to  */ 
/*                                            memset, changed memset to   */ 
/*                                            macro, and added safety     */ 
/*                                            critical error exception,   */ 
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
VOID  _tx_timer_initialize(VOID)
{

#ifndef TX_TIMER_PROCESS_IN_ISR
UINT    status;
#endif

#ifndef TX_DISABLE_REDUNDANT_CLEARING

    /* Initialize the system clock to 0.  */
    _tx_timer_system_clock =  ((ULONG) 0);

    /* Initialize the time-slice value to 0 to make sure it is disabled.  */
    _tx_timer_time_slice =  ((ULONG) 0);

    /* Clear the expired flags.  */
    _tx_timer_expired_time_slice =  TX_FALSE;
    _tx_timer_expired =             TX_FALSE;

    /* Initialize the thread and application timer management control structures.  */

    /* First, initialize the timer list.  */
    TX_MEMSET(&_tx_timer_list[0], 0, (sizeof(_tx_timer_list)));
#endif

    /* Initialize all of the list pointers.  */
    _tx_timer_list_start =   &_tx_timer_list[0];
    _tx_timer_current_ptr =  &_tx_timer_list[0];

    /* Set the timer list end pointer to one past the actual timer list.  This is done
       to make the timer interrupt handling in assembly language a little easier.  */
    _tx_timer_list_end =     &_tx_timer_list[TX_TIMER_ENTRIES-((ULONG) 1)];
    _tx_timer_list_end =     TX_TIMER_POINTER_ADD(_tx_timer_list_end, ((ULONG) 1));

#ifndef TX_TIMER_PROCESS_IN_ISR

    /* Setup the variables associated with the system timer thread's stack and 
       priority.  */
    _tx_timer_stack_start =  (VOID *) &_tx_timer_thread_stack_area[0];
    _tx_timer_stack_size =   ((ULONG) TX_TIMER_THREAD_STACK_SIZE);
    _tx_timer_priority =     ((UINT) TX_TIMER_THREAD_PRIORITY);

    /* Create the system timer thread.  This thread processes all of the timer 
       expirations and reschedules.  Its stack and priority are defined in the
       low-level initialization component.  */
    do
    {
      
        status =  _tx_thread_create(&_tx_timer_thread, (CHAR *) ((VOID *) "System Timer Thread"), _tx_timer_thread_entry, 
                    (ULONG) TX_TIMER_ID,  _tx_timer_stack_start, _tx_timer_stack_size, 
                    _tx_timer_priority, _tx_timer_priority, TX_NO_TIME_SLICE, TX_DONT_START);
      
#ifdef TX_SAFETY_CRITICAL

        /* Check return from thread create - if an error is detected throw an exception.  */
        if (status != TX_SUCCESS)
        {

            /* Raise safety critical exception.  */
            TX_SAFETY_CRITICAL_EXCEPTION(__FILE__, __LINE__, status);
        }
#endif
    } while (status != TX_SUCCESS);
        
#else

    /* Clear the timer interrupt processing active flag.  */
    _tx_timer_processing_active =  TX_FALSE;
#endif

#ifndef TX_DISABLE_REDUNDANT_CLEARING

    /* Initialize the head pointer of the created application timer list.  */
    _tx_timer_created_ptr =  TX_NULL;

    /* Set the created count to zero.  */
    _tx_timer_created_count =  TX_EMPTY;

#ifdef TX_TIMER_ENABLE_PERFORMANCE_INFO

    /* Initialize timer performance counters.  */
    _tx_timer_performance_activate_count =           ((ULONG) 0);
    _tx_timer_performance_reactivate_count =         ((ULONG) 0);
    _tx_timer_performance_deactivate_count =         ((ULONG) 0);
    _tx_timer_performance_expiration_count =         ((ULONG) 0);
    _tx_timer_performance__expiration_adjust_count =  ((ULONG) 0);
#endif
#endif
}

