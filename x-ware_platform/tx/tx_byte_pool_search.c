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
/**   Byte Pool                                                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_thread.h"
#include "tx_byte_pool.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_byte_pool_search                                PORTABLE C      */ 
/*                                                           5.7          */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function searches a byte pool for a memory block to satisfy    */ 
/*    the requested number of bytes.  Merging of adjacent free blocks     */ 
/*    takes place during the search and a split of the block that         */ 
/*    satisfies the request may occur before this function returns.       */ 
/*                                                                        */ 
/*    It is assumed that this function is called with interrupts enabled  */ 
/*    and with the tx_pool_owner field set to the thread performing the   */ 
/*    search.  Also note that the search can occur during allocation and  */ 
/*    release of a memory block.                                          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pool_ptr                          Pointer to pool control block     */ 
/*    memory_size                       Number of bytes required          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    UCHAR *                           Pointer to the allocated memory,  */ 
/*                                        if successful.  Otherwise, a    */ 
/*                                        NULL is returned                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _tx_byte_allocate                 Allocate bytes of memory          */ 
/*    _tx_byte_release                  Release bytes of memory           */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-12-2005     William E. Lamie         Initial Version 5.0           */ 
/*  04-02-2007     William E. Lamie         Modified comment(s), and      */ 
/*                                            added optimization for      */ 
/*                                            memory search pointer       */ 
/*                                            update, resulting in        */ 
/*                                            version 5.1                 */ 
/*  12-12-2008     William E. Lamie         Modified comment(s), added    */ 
/*                                            macro to get current thread,*/ 
/*                                            and made several            */ 
/*                                            optimizations, resulting    */ 
/*                                            in version 5.2              */ 
/*  07-04-2009     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.3    */ 
/*  12-12-2009     William E. Lamie         Modified comment(s), added    */ 
/*                                            explicit checks for examined*/ 
/*                                            blocks and available bytes, */ 
/*                                            added void pointer cast in  */ 
/*                                            pointer type conversions,   */ 
/*                                            changed some counting       */ 
/*                                            variables to type UINT,     */ 
/*                                            and added parentheses in    */ 
/*                                            calculation of available    */ 
/*                                            bytes, resulting in         */ 
/*                                            version 5.4                 */ 
/*  07-15-2011     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.5    */ 
/*  11-01-2012     William E. Lamie         Modified comment(s),          */ 
/*                                            resulting in version 5.6    */ 
/*  05-01-2015     William E. Lamie         Modified comment(s), modified */ 
/*                                            code for MISRA compliance,  */ 
/*                                            added code to assert pool   */ 
/*                                            ownership after protection  */ 
/*                                            is obtained to ensure no    */ 
/*                                            changes to the pool or the  */ 
/*                                            pool search pointer are     */ 
/*                                            made without ownership,     */ 
/*                                            resulting in version 5.7    */ 
/*                                                                        */ 
/**************************************************************************/ 
VOID  *_tx_byte_pool_search(TX_BYTE_POOL *pool_ptr, ULONG memory_size)
{

TX_INTERRUPT_SAVE_AREA

UCHAR           *current_ptr;                
UCHAR           *next_ptr;                   
UCHAR           *search_ptr;
ULONG           available_bytes;            
UINT            examine_blocks;             
UINT            first_free_block_found =  TX_FALSE;
TX_THREAD       *thread_ptr;


    /* Disable interrupts.  */
    TX_DISABLE

    /* First, determine if there are enough bytes in the pool.  */
    if (memory_size >= pool_ptr -> tx_byte_pool_available)
    {

        /* Restore interrupts.  */
        TX_RESTORE

        /* Not enough memory, return a NULL pointer.  */
        current_ptr =  TX_NULL;
    }
    else
    {

        /* Pickup thread pointer.  */
        TX_THREAD_GET_CURRENT(thread_ptr)

        /* Setup ownership of the byte pool.  */
        pool_ptr -> tx_byte_pool_owner =  thread_ptr;

        /* Walk through the memory pool in search for a large enough block.  */
        search_ptr =       pool_ptr -> tx_byte_pool_search;
        current_ptr =      search_ptr;
        examine_blocks =   pool_ptr -> tx_byte_pool_fragments + ((UINT) 1);
        available_bytes =  ((ULONG) 0);
        do
        {


#ifdef TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO

            /* Increment the total fragment search counter.  */
            _tx_byte_pool_performance_search_count++;

            /* Increment the number of fragments searched on this pool.  */
            pool_ptr -> tx_byte_pool_performance_search_count++;
#endif

            /* Check to see if this block is free.  */
            if (*((ULONG *) ((VOID *) (TX_UCHAR_POINTER_ADD(current_ptr, (sizeof(UCHAR *)))))) == TX_BYTE_BLOCK_FREE)
            {

                /* Determine if this is the first free block.  */
                if (first_free_block_found == TX_FALSE)
                {
                
                    /* This is the first free block.  */
            
                    /* Now determine if the search pointer needs adjustment.  */
                    if (search_ptr != current_ptr)
                    {

                        /* The search pointer needs to be changed. Adjust the
                           search pointer to this first free block.  */
                        search_ptr =  current_ptr;
                    }

                    /* Set the flag to indicate we have found the first free
                       block.  */
                    first_free_block_found =  TX_TRUE;
                }

                /* Block is free, see if it is large enough.  */

                /* Pickup the next block's pointer.  */
                next_ptr =  *((UCHAR **) ((VOID *) current_ptr));

                /* Calculate the number of bytes available in this block.  */
                available_bytes =   TX_UCHAR_POINTER_DIF(next_ptr, current_ptr);
                available_bytes =   available_bytes - ((sizeof(UCHAR *)) + (sizeof(ULONG)));

                /* If this is large enough, we are done because our first-fit algorithm
                   has been satisfied!  */
                if (available_bytes >= memory_size)
                {
                    /* Get out of the search loop!  */
                    break;
                }
                else
                {

                    /* Clear the available bytes variable.  */
                    available_bytes =  ((ULONG) 0);

                    /* Not enough memory, check to see if the neighbor is 
                       free and can be merged.  */
                    if (*((ULONG *) ((VOID *) (TX_UCHAR_POINTER_ADD(next_ptr, (sizeof(UCHAR *)))))) == TX_BYTE_BLOCK_FREE)
                    {

                        /* Yes, neighbor block can be merged!  This is quickly accomplished
                           by updating the current block with the next blocks pointer.  */
                        *((UCHAR **) ((VOID *) current_ptr)) =  *((UCHAR **) ((VOID *) next_ptr));

                        /* Reduce the fragment total.  We don't need to increase the bytes
                           available because all free headers are also included in the available
                           count.  */
                        pool_ptr -> tx_byte_pool_fragments--;

#ifdef TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO

                        /* Increment the total merge counter.  */
                        _tx_byte_pool_performance_merge_count++;

                        /* Increment the number of blocks merged on this pool.  */
                        pool_ptr -> tx_byte_pool_performance_merge_count++;
#endif

                        /* See if the search pointer is affected.  */
                        if (pool_ptr -> tx_byte_pool_search ==  next_ptr)
                        {
                    
                            /* Yes, update the search pointer.   */
                            pool_ptr -> tx_byte_pool_search =  current_ptr;
                        }
                    }
                    else
                    {

                        /* Neighbor is not free so we can skip over it!  */
                        current_ptr =  *((UCHAR **) ((VOID *) next_ptr));

                        /* Decrement the examined block count to account for this one.  */
                        if (examine_blocks != ((UINT) 0))
                        {

                            examine_blocks--;

#ifdef TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO

                            /* Increment the total fragment search counter.  */
                            _tx_byte_pool_performance_search_count++;

                            /* Increment the number of fragments searched on this pool.  */
                            pool_ptr -> tx_byte_pool_performance_search_count++;
#endif
                        }
                    }
                }
            }
            else
            {

                /* Block is not free, move to next block.  */
                current_ptr =  *((UCHAR **) ((VOID *) current_ptr));
            } 

            /* Another block has been searched... decrement counter.  */
            if (examine_blocks != ((UINT) 0))
            {

                examine_blocks--;
            }

            /* Restore interrupts temporarily.  */
            TX_RESTORE

            /* Disable interrupts.  */
            TX_DISABLE

            /* Determine if anything has changed in terms of pool ownership.  */
            if (pool_ptr -> tx_byte_pool_owner != thread_ptr)
            {

                /* Pool changed ownership in the brief period interrupts were
                   enabled.  Reset the search.  */
                current_ptr =      pool_ptr -> tx_byte_pool_search;
                search_ptr =       current_ptr;
                examine_blocks =   pool_ptr -> tx_byte_pool_fragments + ((UINT) 1);

                /* Setup our ownership again.  */
                pool_ptr -> tx_byte_pool_owner =  thread_ptr;
            }
        } while(examine_blocks != ((UINT) 0));

        /* Determine if a block was found.  If so, determine if it needs to be
           split.  */
        if (available_bytes != ((ULONG) 0))
        {

            /* Determine if we need to split this block.  */
            if ((available_bytes - memory_size) >= ((ULONG) TX_BYTE_BLOCK_MIN))
            {

                /* Split the block.  */
                next_ptr =  TX_UCHAR_POINTER_ADD(current_ptr, (memory_size + ((sizeof(UCHAR *)) + (sizeof(ULONG)))));

                /* Setup the new free block.  */
                *((UCHAR **) ((VOID *) next_ptr)) =  *((UCHAR **) ((VOID *) current_ptr));
                *((ULONG *) ((VOID *) (TX_UCHAR_POINTER_ADD(next_ptr, (sizeof(UCHAR *)))))) =  TX_BYTE_BLOCK_FREE;

                /* Increase the total fragment counter.  */
                pool_ptr -> tx_byte_pool_fragments++;

                /* Update the current pointer to point at the newly created block.  */
                *((UCHAR **) ((VOID *) current_ptr)) = next_ptr;
    
                /* Set available equal to memory size for subsequent calculation.  */
                available_bytes =  memory_size;

#ifdef TX_BYTE_POOL_ENABLE_PERFORMANCE_INFO

                /* Increment the total split counter.  */
                _tx_byte_pool_performance_split_count++;

                /* Increment the number of blocks split on this pool.  */
                pool_ptr -> tx_byte_pool_performance_split_count++;
#endif
            }

            /* In any case, mark the current block as allocated.  */
            *((TX_BYTE_POOL **) ((VOID *) (TX_UCHAR_POINTER_ADD(current_ptr, (sizeof(UCHAR *)))))) =  pool_ptr;

            /* Reduce the number of available bytes in the pool.  */
            pool_ptr -> tx_byte_pool_available =  (pool_ptr -> tx_byte_pool_available - available_bytes) - ((sizeof(UCHAR *)) + (sizeof(ULONG)));

            /* Determine if the search pointer needs to be updated. This is only done
               if the search pointer matches the block to be returned.  */
            if (current_ptr == search_ptr)
            {

                /* Yes, update the search pointer to the next block.  */
                search_ptr =  *((UCHAR **) ((VOID *) current_ptr));
            }

            /* Update the search pointer.  */
            pool_ptr -> tx_byte_pool_search =  search_ptr;

            /* Restore interrupts.  */
            TX_RESTORE

            /* Adjust the pointer for the application.  */
            current_ptr =  TX_UCHAR_POINTER_ADD(current_ptr, (((sizeof(UCHAR *)) + (sizeof(ULONG)))));
        }
        else
        {

            /* Update the search pointer.  */
            pool_ptr -> tx_byte_pool_search =  search_ptr;

            /* Restore interrupts.  */
            TX_RESTORE

            /* Set current pointer to NULL to indicate nothing was found.  */
            current_ptr =  TX_NULL;
        }
    }

    /* Return the search pointer.  */
    return(current_ptr);
}

