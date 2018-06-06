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
/**   Byte Memory                                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"
#include "tx_byte_pool.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_byte_pool_info_get                              PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves information from the specified byte pool.   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pool_ptr                          Pointer to byte pool control block*/ 
/*    name                              Destination for the pool name     */ 
/*    available_bytes                   Number of free bytes in byte pool */ 
/*    fragments                         Number of fragments in byte pool  */ 
/*    first_suspended                   Destination for pointer of first  */ 
/*                                        thread suspended on byte pool   */ 
/*    suspended_count                   Destination for suspended count   */ 
/*    next_pool                         Destination for pointer to next   */ 
/*                                        byte pool on the created list   */ 
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
/*  12-12-2009     William E. Lamie         Modified comment(s), casted   */ 
/*                                            UINT counting values to     */ 
/*                                            ULONG where necessary,      */ 
/*                                            merged event logging        */ 
/*                                            support, and added logic to */ 
/*                                            explicitly check for valid  */ 
/*                                            pointer, resulting in       */ 
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
UINT  _tx_byte_pool_info_get(TX_BYTE_POOL *pool_ptr, CHAR **name, ULONG *available_bytes, 
                    ULONG *fragments, TX_THREAD **first_suspended, 
                    ULONG *suspended_count, TX_BYTE_POOL **next_pool)
{

TX_INTERRUPT_SAVE_AREA


    /* Disable interrupts.  */
    TX_DISABLE

    /* If trace is enabled, insert this event into the trace buffer.  */
    TX_TRACE_IN_LINE_INSERT(TX_TRACE_BYTE_POOL_INFO_GET, pool_ptr, 0, 0, 0, TX_TRACE_BYTE_POOL_EVENTS)

    /* Log this kernel call.  */
    TX_EL_BYTE_POOL_INFO_GET_INSERT

    /* Retrieve all the pertinent information and return it in the supplied
       destinations.  */

    /* Retrieve the name of the byte pool.  */
    if (name != TX_NULL)
    {
    
        *name =  pool_ptr -> tx_byte_pool_name;
    }

    /* Retrieve the number of available bytes in the byte pool.  */
    if (available_bytes != TX_NULL)
    {
    
        *available_bytes =  pool_ptr -> tx_byte_pool_available;
    }

    /* Retrieve the total number of bytes in the byte pool.  */
    if (fragments != TX_NULL)
    {
    
        *fragments =  (ULONG) pool_ptr -> tx_byte_pool_fragments;
    }

    /* Retrieve the first thread suspended on this byte pool.  */
    if (first_suspended != TX_NULL)
    {
    
        *first_suspended =  pool_ptr -> tx_byte_pool_suspension_list;
    }

    /* Retrieve the number of threads suspended on this byte pool.  */
    if (suspended_count != TX_NULL)
    {
    
        *suspended_count =  (ULONG) pool_ptr -> tx_byte_pool_suspended_count;
    }

    /* Retrieve the pointer to the next byte pool created.  */
    if (next_pool != TX_NULL)
    {
    
        *next_pool =  pool_ptr -> tx_byte_pool_created_next;
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Return completion status.  */
    return(TX_SUCCESS);
}

