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
/**   Initialize                                                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/* #define TX_SOURCE_CODE  */


/* Include necessary system files.  */
/*  #include "tx_api.h"
    #include "tx_initialize.h"
    #include "tx_thread.h"
    #include "tx_timer.h"  */
    #include "encoding.h"


    /* PCRI: Power, Reset, Clock, Interrupt */
    /* Defined in riscv_CoreplexE31.h, but that file contains C-code */
    PRCI_BASE            =  0x44000000
    PRCI_MSIP_OFFSET     =  0x00000000
    PRCI_MTIMECMP_OFFSET =  0x00004000
    PRCI_MTIME_OFFSET    =  0x0000BFF8

    PLIC_BASE            =  0x40000000
    PLIC_PRI_OFFSET      =  0x00000000
    PLIC_IE_OFFSET       =  0x00002000
    PLIC_THRES_OFFSET    =  0x00200000
    PLIC_CLAIM_OFFSET    =  0x00200004

    .text
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_initialize_low_level                         RISC-V/SoftConsole */
/*                                                           5.0          */
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*    Tom van Leeuwen, Technolution B.V.                                  */
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is responsible for any low-level processor            */ 
/*    initialization, including setting up interrupt vectors, setting     */ 
/*    up a periodic timer interrupt source, saving the system stack       */ 
/*    pointer for use in ISR processing later, and finding the first      */ 
/*    available RAM memory address for tx_application_define.             */ 
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
/*    _tx_initialize_kernel_enter           ThreadX entry function        */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-12-2017     Tom van Leeuwen          Initial Version 5.0           */
/*                                                                        */ 
/**************************************************************************/ 
/* VOID   _tx_initialize_low_level(VOID)
{  */
    .globl  _tx_initialize_low_level
_tx_initialize_low_level:
    sw      sp, _tx_thread_system_stack_ptr, t0     # Save system stack pointer

    la      t0, _free_memory                        # Pickup first free address
    sw      t0, _tx_initialize_unused_memory, t1    # Save unused memory address


    /* ThreadX Timer interrupt setup. */
    nop
    li      t0, PRCI_BASE + PRCI_MTIME_OFFSET       # Load MTIME base
_tx_init_read_mtime:
    lw      t4, 4(t0)                               # Load MTIME[63:32]
    lw      t2, 0(t0)                               # Load MTIME[31:0]
    lw      t3, 4(t0)                               # Load MTIME[63:32]
    bne     t3, t4, _tx_init_read_mtime
    lw      t4, _systick_increment                  # Load _systick_increment

    add     t5, t2, t4                              # Add _systick_increment to MTIMECMP[31:0]
    bltu    t2, t5, _time_no_overflow               # If no overflow, just write the new MTIMECMP[31:0]

    /* new MTIMECMP[31:0] is lower, increment MTIMECMP[63:32] */
    addi    t3, t3, 1                               # Overflow, add 1 to upper word
_time_no_overflow:
    /* Before storing MTIMECMP[63:32], set MTIMECMP[31:0] to max to prevent spurious interrupts */
    li      t0, PRCI_BASE + PRCI_MTIMECMP_OFFSET    # Load MTIMECMP base to store new time value.
    li      t2, -1                                  # Load max value in t0
    sw      t2, (t0)                                # Store max value in lower word to prevent spurious interrupt
    sw      t3, 4(t0)                               # Store new value of upper word
    sw      t5, (t0)                                # Set lower word to actual new value
    /* Enable the timer and external interrupt */
    li      t0, (MIP_MTIP | MIP_MEIP)
    csrw    mie, t0                                 # Enable timer interrupt

    ret                              #


    /* Define the actual interrupt/exception handler.  Since this routine must handle
       multiple exceptions, the context save/restore functions are called automatically.
       Application specific ISR processing added to this routine must be inserted into
       the proper place and use registers in accordance with the RISC-V compiler.  
       C functions called from this area will automatically save/restore these registers 
       if they are used.  */

    .globl  _tx_exception_handler
_tx_exception_handler:
    /* Before calling _tx_thread_context_save, we have to save the current value of
       x1 (ra). To avoid modifying the thread context here, don't store the upper-
       immediate of _tx_thread_context_save in t0 but use x1 instead. */
    csrw    mscratch, x1                            # Save x1 in scratch so we can use it as RA.

    csrr    x1, mcause                              # Fetch cause - temporarily abuse x1
    bge     x1, x0, _tx_error_exceptions            # If negative (msb set), it's a normal interrupt.


    la      x1, _tx_thread_context_save             # Build the address in x1 so t1 is not affected.
    jalr    x1, x1                                  # Call TX Thread context save

    csrr    t0, mcause                              # Fetch the cause register again.
    li      t2, 0x7FFFFFFF
    and     t1, t0, t2
    li      t2, 7                                   # TODO: make constant.
    bne     t1, t2, _tx_not_timer

    /* TX Timer interrupt */
    li      t0, PRCI_BASE + PRCI_MTIMECMP_OFFSET    # Load MTIME base
_tx_timer_read_mtime:
    lw      t2, 0(t0)                               # Load MTIMECMP[31:0]
    lw      t3, 4(t0)                               # Load MTIMECMP[63:32]
    lw      t4, _systick_increment                  # Load _systick_increment

    add     t5, t2, t4                              # Add _systick_increment to MTIMECMP[31:0]
    bltu    t2, t5, _tx_timecmp_no_overflow         # If no overflow, just write the new MTIMECMP[31:0]

    /* new MTIMECMP[31:0] is lower, increment MTIMECMP[63:32] */
    addi    t3, t3, 1                               # Overflow, add 1 to upper word
    /* Before storing MTIMECMP[63:32], set MTIMECMP[31:0] to max to prevent spurious interrupts */
    li      t2, -1                                  # Load max value in t0
    sw      t2, (t0)                                # Store max value in lower word to prevent spurious interrupt
    sw      t3, 4(t0)                               # Store new value of upper word
_tx_timecmp_no_overflow:
    sw      t5, (t0)                                # Set lower word to actual new value

    /* Call the ThreadX timer routine.  */
    call    _tx_timer_interrupt                     # Call timer interrupt handler
    j       _tx_interrupt_done

_tx_not_timer:
    /* Check for External Interrupt.  */
    li      t2, 11                                  # TODO: make constant.
    bne     t1, t2, _tx_not_external

    li      t0, PLIC_BASE + PLIC_CLAIM_OFFSET       # Build the CLAIM address
    lw      s0, (t0)                                # Fetch the IRQ from the PLIC
    beqz    s0, _tx_interrupt_done                  # IRQ=0 means no external interrupt, but CAUSE == 11
                                                    # so can't be software either. Jump to done.

    mv      a0, s0                                  # Provide IRQ as argument to C-oode
    call    _tx_external_interrupt                  # Call the C-code
    li      t0, PLIC_BASE + PLIC_CLAIM_OFFSET       # Build the COMPELTE address
    sw      s0, (t0)                                # Indicate interrupt done.

    j       _tx_interrupt_done

_tx_not_external:
    /* Check for Software Interrupt.  */
    li      t2, 3                                   # TODO: make constant.
    bne     t1, t2, _tx_interrupt_done

    call    _tx_software_interrupt
    li      t0, PRCI_BASE + PRCI_MSIP_OFFSET        # Build the MSIP address
    sw      x0, (t0)                                # Clear MSIP

_tx_interrupt_done:
    j       _tx_thread_context_restore              # Call context restore function

    /* Error Exception processing goes here!  */
_tx_error_exceptions:
    j   _tx_error_exceptions                    // Default error exception processing

    /* Reference the build options and the version ID to ensure they are part of the image.  */
    la      t0, _tx_build_options
    la      t1, _tx_version_id
