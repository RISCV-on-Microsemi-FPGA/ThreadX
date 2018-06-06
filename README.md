# ThreadX
ThreadX port for Mi-V CPUs

This folder contains the simple SoftConsole project demonstrating ThreadX on Mi-V processor.

### Test Platform and FPGA design:
This project is tested on following hardware platforms:

RISCV-Creative-Board
- [RISC-V Creative board Mi-V Sample Design](https://github.com/RISCV-on-Microsemi-FPGA/RISC-V-Creative-Board/tree/master/Programming_The_Target_Device/PROC_SUBSYSTEM_MIV_RV32IMA_BaseDesign)

PolarFire-Eval-Kit
- [PolarFire-Eval-Kit RISC-V Sample Design](https://github.com/RISCV-on-Microsemi-FPGA/PolarFire-Eval-Kit/tree/master/Programming_The_Target_Device/MIV_RV32IMA_L1_AHB_BaseDesign)

SmartFusion2-Advanced-Dev-Kit
- [SmartFusion2 Advanced Development Kit RISC-V Sample Design](https://github.com/RISCV-on-Microsemi-FPGA/SmartFusion2-Advanced-Dev-Kit/tree/master/Programming_The_Target_Device/PROC_SUBSYSTEM_BaseDesign)    

### How to use this project:
The `RISCV_Demo_Threadx` is the demonstration project which uses the ThreadX library 'tx' generated using another SoftConsole project ./tx. This library is imported by RISCV_Demo_Threadx.

The demo_threadx.c creates several threads thread_0_entry and thread_1_entry etc and demonstrates the usage of IPC such as queue, mutex etc.

### ThreadX project configurations:
The ./hw_platform.h file contains the FPGA design related information that is required 
for this project. If you update the FPGA design, the hw_platform.h must be updated 
accordingly.
    
E.g. You must use the processor clock and Memory size as per the Libero design that you are using. 

The RISC-V creative board design uses 66Mhz processor clock. The PolarFire Eval Kit design uses 50Mhz processor clock. The SmartFusion2 Adv. Development kit design uses 83Mhz processor clock.

### Microsemi SoftConsole Toolchain:
To know more please refer: [SoftConsole](https://github.com/RISCV-on-Microsemi-FPGA/SoftConsole)

### Documentation for Microsemi RISC-V processor, SoftConsole toochain, Debug Tools, FPGA design etc.
To know more please refer: [Documentation](https://github.com/RISCV-on-Microsemi-FPGA/Documentation)
    

