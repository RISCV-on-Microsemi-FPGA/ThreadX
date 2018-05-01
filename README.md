# ThreadX
ThreadX port for Mi-V CPUs

This folder has the simple SoftConsole project demonstrating ThreadX on Mi-V processor.

### Test Platform and FPGA design:
This project is tested on foloowing hardware platforms:

RISCV-Creative-Board
[IGLOO2 Creative board Ni-V Sample Design](https://github.com/RISCV-on-Microsemi-FPGA/RISC-V-Creative-Board/Programming_The_Target_Device/PROC_SUBSYSTEM_MIV_RV32IMA_BaseDesign)

PolarFire-Eval-Kit
[SmartFusion2 Advanced Development Kit RISC-V Sample Design](https://github.com/RISCV-on-Microsemi-FPGA/PolarFire-Eval-Kit/Programming_The_Target_Device\MIV_RV32IMA_L1_AHB_BaseDesign)

M2S150-Advanced-Dev-Kit
[SmartFusion2 Advanced Development Kit RISC-V Sample Design](https://github.com/RISCV-on-Microsemi-FPGA/M2S150-Advanced-Dev-Kit/Programming_The_Target_Device/PROC_SUBSYSTEM_BaseDesign)    

### How to use this project:
The RISCV_Demo_Threadx is the demonstration project which uses the ThreadX library 'tx' generated using another softconsole project ./tx. This library is imported by RISCV_Demo_Threadx.

The demo_threadx.c creates several threads thread_0_entry and thread_1_entry etc and demonstrates the usage of IPC such as queue, mutex etc.

### ThreadX project configurations:
The ./hw_platform.h file contains the design related information that is required 
for this project. If you update the design, the hw_platform.h must be updated 
accordingly.
    
E.g. You must use the clock value in demo_threadx.c as per the Libero design that 
you are using. 

The RISC-V creative board design is running at 66Mhz clock.
The PolarFire Eval Kit design is running at 50Mhz clock.
The SmartFusion2 Adv. Developement kit design is running at 83Mhz clock.
    
### Microsemi SoftConsole Toolchain:
To know more please refer: [SoftConsole](https://github.com/RISCV-on-Microsemi-FPGA/SoftConsole)

### Documentation for Microsemi RISC-V processor, SoftConsole toochain, Debug Tools, FPGA design etc.
To know more please refer: [Documentation](https://github.com/RISCV-on-Microsemi-FPGA/Documentation)
    

