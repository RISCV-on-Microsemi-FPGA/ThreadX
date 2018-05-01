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
#include "tx_mutex.h"
#ifdef TX_MUTEX_ENABLE_PERFORMANCE_INFO
#include "tx_trace.h"
#endif


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_mutex_performance_system_info_get               PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves system mutex performance information.       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    puts                              Destination for total number of   */
/*                                        mutex puts                      */ 
/*    gets                              Destination for total number of   */ 
/*                                        mutex gets                      */ 
/*    suspensions                       Destination for total number of   */ 
/*                                        mutex suspensions               */ 
/*    timeouts                          Destination for total number of   */ 
/*                                        mutex timeouts                  */ 
/*    inversions                        Destination for total number of   */ 
/*                                        mutex priority inversions       */ 
/*    inheritances                      Destination for total number of   */ 
/*                                        mutex priority inheritances     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Completion status                 */ 
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
/*                                            added filter option to      */ 
/*                                            trace insert, resulting     */ 
/*                                            in version 5.2              */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), added    */ 
/*                                            logic to explicitly check   */ 
/*                                            for valid pointer, merged   */ 
/*                                            event logging support, and  */ 
/*                                            added code to ensure that   */ 
/*                                            input parameters are        */ 
/*                                            accessed in non-enabled     */ 
/*                                            case (default),             */ 
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
UINT  _tx_mutex_performance_system_info_get(ULONG *puts, ULONG *gets, ULONG *suspensions, 
                                ULONG *timeouts, ULONG *inversions, ULONG *inheritances)
{

#ifdef TX_MUTEX_ENABLE_PERFORMANCE_INFO

TX_INTERRUPT_SAVE_AREA


    /* Disable interrupts.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_MUTEX_PERFORMANCE_SYSTEM_INFO_GET, 0, 0, 0, 0, TX_TRACE_MUTEX_EVENTS)

    /* Log this kernel call.  */
    TX_EL_MUTEX_PERFORMANCE_SYSTEM_INFO_GET_INSERT

    /* Retrieve all the pertinent information and return it in the supplied
       destinations.  */

    /* Retrieve the total number of mutex puts.  */
    if (puts != TX_NULL)
    {
    
        *puts =  _tx_mutex_performance_put_count;
    }
    
    /* Retrieve the total number of mutex gets.  */
    if (gets != TX_NULL)
    {
    
        *gets =  _tx_mutex_performance_get_count;
    }
    
    /* Retrieve the total number of mutex suspensions.  */
    if (suspensions != TX_NULL)
    {
    
        *suspensions =  _tx_mutex_performance_suspension_count;
    }
    
    /* Retrieve the total number of mutex timeouts.  */
    if (timeouts != TX_NULL)
    {
    
        *timeouts =  _tx_mutex_performance_timeout_count;
    }
    
    /* Retrieve the total number of mutex priority inversions.  */
    if (inversions != TX_NULL)
    {
    
        *inversions =  _tx_mutex_performance_priority_inversion_count;
    }
    
    /* Retrieve the total number of mutex priority inheritances.  */
    if (inheritances != TX_NULL)
    {
    
        *inheritances =  _tx_mutex_performance__priority_inheritance_count;
    }
    
    /* Restore interrupts.  */
    TX_RESTORE

    /* Return completion status.  */
    return(TX_SUCCESS);
    
#else

UINT        status;


    /* Access input arguments just for the sake of lint, MISRA, etc.  */
    if (puts != TX_NULL)
    {
    
        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (gets != TX_NULL)
    {
    
        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (suspensions != TX_NULL)
    {
    
        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (timeouts != TX_NULL)
    {
    
        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (inversions != TX_NULL)
    {
    
        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else if (inheritances != TX_NULL)
    {
    
        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }
    else
    {
    
        /* Not enabled, return error.  */
        status =  TX_FEATURE_NOT_ENABLED;
    }

    /* Return completion status.  */
    return(status);
#endif
}

