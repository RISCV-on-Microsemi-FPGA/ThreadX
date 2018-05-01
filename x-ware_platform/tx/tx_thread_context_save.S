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
/*    _tx_thread_context_save                          RISC-V/SoftConsole */
/*                                                           5.0          */
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*    Tom van Leeuwen, Technolution B.V.                                  */
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function saves the context of an executing thread in the       */ 
/*    beginning of interrupt processing.  The function also ensures that  */ 
/*    the system stack is used upon return to the calling ISR.            */ 
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
/*    ISRs                                                                */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-12-2017     Tom van Leeuwen          Initial Version 5.0           */
/*                                                                        */ 
/**************************************************************************/ 
/* VOID   _tx_thread_context_save(VOID)
{  */
    .globl _tx_thread_context_save
_tx_thread_context_save:

    /* Upon entry to this routine, it is assumed that interrupts are locked
       out and the stack is exactly were it was when the interrupt occurred.
       The return address is in x1 (ra).
       The original content of x1 is in MSCRATCH  */


    csrrw   x1, mscratch, x1                # Restore RA from thread, save our RA.
    addi    sp, sp, -128                    # Allocate space for all registers

    sw      x5, 0x4C(sp)                    # First store t0 and t1
    sw      x6, 0x48(sp)

    la      x5, _tx_thread_system_state     # Pickup address of system state
    lw      x6, 0(x5)                       # Pickup system state

    /* Check for a nested interrupt condition.  */
    /* if (_tx_thread_system_state++)
    {  */
    beqz    x6, _tx_thread_not_nested_save  # If 0, first interrupt condition
    addi    x6, x6, 1                       # Increment the interrupt counter
    sw      x6, (x5)                        # Store the interrupt counter

    /* Nested interrupt condition.
       Save the reset of the scratch registers on the stack and return to the
       calling ISR.  */

    sw      x1, 0x70(sp)                    # Store RA
    sw      x7, 0x44(sp)                    # Store t2
    sw      x8, 0x30(sp)                    # Store s0
    sw      x10, 0x6C(sp)                   # Store a0
    sw      x11, 0x68(sp)                   # Store a1
    sw      x12, 0x64(sp)                   # Store a2
    sw      x13, 0x60(sp)                   # Store a3
    sw      x14, 0x5C(sp)                   # Store a4
    sw      x15, 0x58(sp)                   # Store a5
    sw      x16, 0x54(sp)                   # Store a6
    sw      x17, 0x50(sp)                   # Store a7
    sw      x28, 0x40(sp)                   # Store t3
    sw      x29, 0x3C(sp)                   # Store t4
    sw      x30, 0x38(sp)                   # Store t5
    sw      x31, 0x34(sp)                   # Store t6
    csrr    t0, mepc                        # Load exception program counter
    sw      t0, 0x78(sp)                    # Save it on the stack

#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
    call    _tx_execution_isr_enter         # Call the ISR execution enter function
#endif

    csrr    x1, mscratch                    # Restore our RA
    ret

_tx_thread_not_nested_save:
    /* }  */

    /* Otherwise, not nested, check to see if a thread was running.  */
    /* else if (_tx_thread_current_ptr)
    {  */
    addi    x6, x6, 1                       # Increment the interrupt counter
    sw      x6, (x5)                        # Store the interrupt counter

    /* Not nested: Find the user thread that was running and load our SP */

    lw      x5, _tx_thread_current_ptr      # Pickup current thread pointer
    beqz    x5, _tx_thread_idle_system_save # If NULL, idle system was interrupted

    /* Save the standard scratch registers.  */
    sw      x1, 0x70(sp)                    # Store RA
    sw      x7, 0x44(sp)                    # Store t2
    sw      x8, 0x30(sp)                    # Store s0
    sw      x10, 0x6C(sp)                   # Store a0
    sw      x11, 0x68(sp)                   # Store a1
    sw      x12, 0x64(sp)                   # Store a2
    sw      x13, 0x60(sp)                   # Store a3
    sw      x14, 0x5C(sp)                   # Store a4
    sw      x15, 0x58(sp)                   # Store a5
    sw      x16, 0x54(sp)                   # Store a6
    sw      x17, 0x50(sp)                   # Store a7
    sw      x28, 0x40(sp)                   # Store t3
    sw      x29, 0x3C(sp)                   # Store t4
    sw      x30, 0x38(sp)                   # Store t5
    sw      x31, 0x34(sp)                   # Store t6

    csrr    t0, mepc                        # Load exception program counter
    sw      t0, 0x78(sp)                    # Save it on the stack

    /* Save the current stack pointer in the thread's control block.  */
    /* _tx_thread_current_ptr -> tx_thread_stack_ptr =  sp;  */

    /* Switch to the system stack.  */
    /* sp =  _tx_thread_system_stack_ptr;  */

    lw      t1, _tx_thread_current_ptr      # Pickup current thread pointer
    sw      sp, 8(t1)                       # Save stack pointer*/

#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
    /* _tx_execution_isr_enter is called with thread stack pointer */
    call    _tx_execution_isr_enter         # Call the ISR execution enter function
#endif


    lw      sp, _tx_thread_system_stack_ptr # Switch to system stack

    csrr    x1, mscratch                    # Restore our RA
    ret

    /* }
    else
    {  */

_tx_thread_idle_system_save:


#ifdef TX_ENABLE_EXECUTION_CHANGE_NOTIFY
    call    _tx_execution_isr_enter         # Call the ISR execution enter function
#endif

    /* Interrupt occurred in the scheduling loop.  */

    /* }
}  */
    addi    sp, sp, 128                     # Recover the reserved stack space
    csrr    x1, mscratch                    # Restore our RA
    ret                                     # Return to interrupt handler
