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
    #include "tx_thread.h"  */

    .text
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _tx_thread_stack_build                           RISC-V/SoftConsole */
/*                                                           5.0          */
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    William E. Lamie, Express Logic, Inc.                               */ 
/*    Tom van Leeuwen, Technolution B.V.                                  */
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function builds a stack frame on the supplied thread's stack.  */
/*    The stack frame results in a fake interrupt return to the supplied  */
/*    function pointer.                                                   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    thread_ptr                            Pointer to thread control blk */
/*    function_ptr                          Pointer to return function    */
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
/*    _tx_thread_create                     Create thread service         */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-12-2017     Tom van Leeuwen          Initial Version 5.0           */
/*                                                                        */ 
/**************************************************************************/ 
/* VOID   _tx_thread_stack_build(TX_THREAD *thread_ptr, VOID (*function_ptr)(VOID))
{  */
    .globl  _tx_thread_stack_build
_tx_thread_stack_build:

       
    /* Build a fake interrupt frame.  The form of the fake interrupt stack
       on the RISC-V should look like the following after it is built:
       
       Stack Top:      1       (00)    Interrupt stack frame type
                       x27     (04)    Initial s11
                       x26     (08)    Initial s10
                       x25     (12)    Initial s9
                       x24     (16)    Initial s8
                       x23     (20)    Initial s7
                       x22     (24)    Initial s6
                       x21     (28)    Initial s5
                       x20     (32)    Initial s4
                       x19     (36)    Initial s3
                       x18     (40)    Initial s2
                       x9      (44)    Initial s1
                       x8      (48)    Initial s0
                       x31     (52)    Initial t6
                       x30     (56)    Initial t5
                       x29     (60)    Initial t4
                       x28     (64)    Initial t3
                       x7      (68)    Initial t2
                       x6      (72)    Initial t1
                       x5      (76)    Initial t0
                       x17     (80)    Initial a7
                       x16     (84)    Initial a6
                       x15     (88)    Initial a5
                       x14     (92)    Initial a4
                       x13     (96)    Initial a3
                       x12     (100)   Initial a2
                       x11     (104)   Initial a1
                       x10     (108)   Initial a0
                       x1      (112)   Initial ra
                       mepc    (120)   Initial mepc
                               (124)   Reserved...

    Stack Bottom: (higher memory address)  */

    lw      t0, 16(a0)                      # Pickup end of stack area
    li      t1, ~7                          # Build double alignment mask
    and     t0, t0, t1                      # Make sure double word alignment

    /* Actually build the stack frame.  */

    addi    t0, t0, -128                    # Allocate space for the stack frame
    li      t1, 1                           # Build stack type
    sw      t1, (t0)                        # Place stack type on the top
    sw      x0, 4(t0)                       # Initial s11
    sw      x0, 8(t0)                       # Initial s10
    sw      x0, 12(t0)                      # Initial s9
    sw      x0, 16(t0)                      # Initial s8
    sw      x0, 20(t0)                      # Initial s7
    sw      x0, 24(t0)                      # Initial s6
    sw      x0, 28(t0)                      # Initial s5
    sw      x0, 32(t0)                      # Initial s4
    sw      x0, 36(t0)                      # Initial s3
    sw      x0, 40(t0)                      # Initial s2
    sw      x0, 44(t0)                      # Initial s1
    sw      x0, 48(t0)                      # Initial s0
    sw      x0, 52(t0)                      # Initial t6
    sw      x0, 56(t0)                      # Initial t5
    sw      x0, 60(t0)                      # Initial t4
    sw      x0, 64(t0)                      # Initial t3
    sw      x0, 68(t0)                      # Initial t2
    sw      x0, 72(t0)                      # Initial t1
    sw      x0, 76(t0)                      # Initial t0
    sw      x0, 80(t0)                      # Initial a7
    sw      x0, 84(t0)                      # Initial a6
    sw      x0, 88(t0)                      # Initial a5
    sw      x0, 92(t0)                      # Initial a4
    sw      x0, 96(t0)                      # Initial a3
    sw      x0, 100(t0)                     # Initial a2
    sw      x0, 104(t0)                     # Initial a1
    sw      x0, 108(t0)                     # Initial a0
    sw      x0, 112(t0)                     # Initial ra
    sw      a1, 120(t0)                     # Initial mepc

    /* Setup stack pointer.  */
    /* thread_ptr -> tx_thread_stack_ptr =  t0;  */

    sw      t0, 8(a0)                       # Save stack pointer in thread's
    ret                                     #   control block
/* }  */

