                      Express Logic's ThreadX for Microsemi RISC-V

                            Using Microsemi SoftConsole

1.  Installation

ThreadX for the Microsemi RISC-V is delivered as a set of SoftConsole projects. 

Unpack the archive and add the ThreadX project and demo_threadx project to 
SoftConsole by using Import (under File) and select "Existing Project into Workspace" 
under "General".

In the import window, select the "tx" directory of the extracted project as root
directory. The "tx" project will show up under "Projects". Ensure that this 
project is selected and click Finish. Repeat for the "demo_threadx" project


2.  Building the ThreadX run-time Library

In Project Explorer, right-click on the "tx" project and click "Build Project"

3.  Demonstration System

Select the "demo_threadx" project from the Project Explorer and click "Build Project"
from the project menu. Next, click the debug icon in the toolbar and open the
"GDB OpenOCD Debugging" by clicking the arrow in front of it. Then, select 
"demo_threadx Debug" and click "Debug" in the right bottom corner
to upload the project to the IGLOO development board. The demo will be started
and halted at the main() function. Click Resume (F8) to start the ThreadX demo.

To show the thread counters, open demo_threadx.c while in the Debug perspective
and select the thread_0_counter variable name. Right-click on the selected text 
and click "Add Watch Expression". Note: The contents of the watch expressions
are only updated when the target is suspended.

4.  System Initialization

The system entry point is at the label __start in the entry.s file. In addition, 
this is where all static and global pre-set C variable initialization processing 
is called from.

After the startup function returns, ThreadX initialization is called. The main
initialization function is _tx_initialize_low_level and is located in the file 
tx_initialize_low_level.S. This function is responsible for setting up various
system data structures, and interrupt vectors.

_tx_initialize_low_level contains the IO addresses of the mandatory MTIME and 
MTIMECMP registers of the RISC-V processor, as well as the PLIC addresses. These
should be updated according to the hardware platform.

In addition, _tx_initialize_low_level also determines the first available 
address for use by the application. By default, the first available address 
is assumed to start at the beginning of the ThreadX section _free_memory. If 
changes are made to the demo_threadx.ld file, the _free_memory section should 
remain the last allocated section in the main RAM area. The starting address of 
this section is passed to the application definition function, 
tx_application_define.

5.  Assembler / Compiler Switches

The default following are compiler switches used in building the demonstration 
system:

Compiler Switch                 Meaning

    -g                  Specifies debug information
    -c                  Specifies object code generation
    -march=rv32im       Specifies RV32IM code generation

6.  Register Usage and Stack Frames

The GNU RISC-V compiler assumes that registers t0-t6 (x5-x7, x28-x31) 
are scratch registers for each function. All other registers used by a C 
function must be preserved by the function. ThreadX takes advantage of this 
in situations where a context switch happens as a result of making a ThreadX 
service call (which is itself a C function). In such cases, the saved context 
of a thread is only the non-scratch registers.

The following defines the saved context stack frames for context switches
that occur as a result of interrupt handling or from thread-level API calls.
All suspended threads have one of these two types of stack frames. The top
of the suspended thread's stack is pointed to by tx_thread_stack_ptr in the 
associated thread control block TX_THREAD.



    Offset        Interrupted Stack Frame        Non-Interrupt Stack Frame

     0x00                   1                           0
     0x04                   s11 (x27)                   s11 (x27)
     0x08                   s10 (x26)                   s10 (x26)
     0x0C                   s9  (x25)                   s9  (x25)
     0x10                   s8  (x24)                   s8  (x24)
     0x14                   s7  (x23)                   s7  (x23)
     0x18                   s6  (x22)                   s6  (x22)
     0x1C                   s5  (x21)                   s5  (x21)
     0x20                   s4  (x20)                   s4  (x20)
     0x24                   s3  (x19)                   s3  (x19)
     0x28                   s2  (x18)                   s2  (x18)
     0x2C                   s1  (x9)                    s1  (x9) 
     0x30                   s0  (x8)                    s0  (x8)    
     0x34                   t6  (x31)                   ra  (x1)
     0x38                   t5  (x30)                   mstatus
     0x3C                   t4  (x29)                   
     0x40                   t3  (x28)                   
     0x44                   t2  (x7)                   
     0x48                   t1  (x6)                   
     0x4C                   t0  (x5)                   
     0x50                   a7  (x17)                    
     0x54                   a6  (x16)                    
     0x58                   a5  (x15)                    
     0x5C                   a4  (x14)                    
     0x60                   a3  (x13)                    
     0x64                   a2  (x12)                    
     0x68                   a1  (x11)                    
     0x6C                   a0  (x10)
     0x70                   ra  (x1)
     0x74                   reserved
     0x78                   mepc




7.  Improving Performance

The distribution version of ThreadX is built without any compiler 
optimizations. This makes it easy to debug because you can trace or set 
breakpoints inside of ThreadX itself. Of course, this costs some 
performance. To make ThreadX run faster, you can change the build configuration
of the SoftConsole project to disable debug information and enable the desired 
optimizations. 

In addition, you can eliminate the ThreadX basic API error checking by 
compiling your application code with the symbol TX_DISABLE_ERROR_CHECKING 
defined before tx_api.h is included. 


8.  Interrupt Handling

ThreadX provides complete and high-performance interrupt handling for RV32I
targets. The ThreadX general exception handler is defined in the file 
tx_initialize_low_level.S at the label _tx_exception_handler. The mtvec CSR
is set to this label during initialization.

8.1  Application ISRs

If application ISRs are used, the application code shall configure the PLIC. 
If an external interrupt appears, the procedure _tx_external_interrupt is called with
the IRQ number as only argument. This procedure shall ensure that the external
interrupt is cleared before returning. ThreadX has control over the CLAIM / COMPLETE 
register so the application only has to ensure that the external interrupt is cleared.

9.  Revision History

For generic code revision information, please refer to the readme_threadx_generic.txt
file, which is included in your distribution. The following details the revision
information associated with this specific port of ThreadX:

12/12/2017  Initial ThreadX version for RISC-V using Microsemi SoftConsole.


Copyright(c) 1996-2017 Express Logic, Inc.


Express Logic, Inc.
11423 West Bernardo Court
San Diego, CA  92127

www.expresslogic.com

