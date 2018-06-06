/**************************************************************************/ 
/*                                                                        */ 
/*            Copyright (c) 1996-2017 by Express Logic Inc.               */
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
/*  11423 West Bernardo Court               http://www.expresslogic.com   */
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

/* #define TX_SOURCE_CODE  */


/* Include necessary system files.  */

/*  #include "tx_api.h"
    #include "tx_thread.h"
    #include "tx_timer.h"  */

    .text
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_thread_system_return                         RISC-V/SoftConsole */
/*                                                           5.0          */
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*    Tom van Leeuwen, Technolution B.V.                                  */
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is target processor specific.  It is used to transfer */ 
/*    control from a thread back to the system.  Only a minimal context   */ 
/*    is saved since the compiler assumes temp registers are going to get */ 
/*    slicked by a function call anyway.                                  */ 
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
/*    _tx_thread_schedule                   Thread scheduling loop        */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    ThreadX components                                                  */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-12-2017     Tom van Leeuwen          Initial Version 5.0           */
/*                                                                        */ 
/**************************************************************************/ 
/* VOID   _tx_thread_system_return(VOID)
{  */
    .globl  _tx_thread_system_return
_tx_thread_system_return:

    /* Save minimal context on the stack.  */

    addi    sp, sp, -56                      # Allocate space on the stack


    sw      x0, (sp)        // Solicited stack type
    sw      x1, 0x34(sp)    // RA
    sw      x8, 0x30(sp)    // s0
    sw      x9, 0x2C(sp)    // s1
    sw      x18, 0x28(sp)   // s2
    sw      x19, 0x24(sp)   // s3
    sw      x20, 0x20(sp)   // s4
    sw      x21, 0x1C(sp)   // s5
    sw      x22, 0x18(sp)   // s6
    sw      x23, 0x14(sp)   // s7
    sw      x24, 0x10(sp)   // s8
    sw      x25, 0x0C(sp)   // s9
    sw      x26, 0x08(sp)   // s10
    sw      x27, 0x04(sp)   // s11
    csrr    t0, mstatus     // mstatus
    sw      t0, 0x38(sp)    // mstatus


   /* Lockout interrupts. - will be enabled in _tx_thread_schedule  */
    csrci   mstatus, 0xF 
    
#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY

    call    _tx_execution_thread_exit       # Call the thread execution exit function
#endif
    
    la      t0, _tx_thread_current_ptr      # Pickup address of pointer
    lw      t1, (t0)                        # Pickup current thread pointer
    la      t2,_tx_thread_system_stack_ptr  # Pickup stack pointer address

    /* Save current stack and switch to system stack.  */
    /* _tx_thread_current_ptr -> tx_thread_stack_ptr =  SP;
    SP = _tx_thread_system_stack_ptr;  */

    sw      sp, 8(t1)                       # Save stack pointer
    lw      sp, (t2)                        # Switch to system stack

    /* Determine if the time-slice is active.  */
    /* if (_tx_timer_time_slice)
    {  */

    la      t4, _tx_timer_time_slice        # Pickup time slice variable addr
    lw      t3, (t4)                        # Pickup time slice value
    la      t2, _tx_thread_schedule         # Pickup address of scheduling loop
    beqz    t3, _tx_thread_dont_save_ts     # If no time-slice, don't save it

        /* Save time-slice for the thread and clear the current time-slice.  */
        /* _tx_thread_current_ptr -> tx_thread_time_slice =  _tx_timer_time_slice;
        _tx_timer_time_slice =  0;  */

    sw      t3, 24(t1)                      # Save current time-slice for thread
    sw      x0, (t4)                        # Clear time-slice variable

    /* }  */
_tx_thread_dont_save_ts:

    /* Clear the current thread pointer.  */
    /* _tx_thread_current_ptr =  TX_NULL;  */

    sw      x0, (t0)                        # Clear current thread pointer
    jr      t2                              # Return to thread scheduler

/* }  */
