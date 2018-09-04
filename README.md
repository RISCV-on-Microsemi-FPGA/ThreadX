# ThreadX
ThreadX port for Mi-V CPUs

This repository contains a simple SoftConsole project demonstrating ThreadX on a Mi-V processor.

### Test Platform and FPGA design:
This project is tested on following hardware platforms:

RISCV-Creative-Board
- [RISC-V Creative board Mi-V Sample Design](https://github.com/RISCV-on-Microsemi-FPGA/RISC-V-Creative-Board/tree/master/Programming_The_Target_Device/PROC_SUBSYSTEM_MIV_RV32IMA_BaseDesign)

PolarFire-Eval-Kit
- [PolarFire Eval Kit RISC-V Sample Design](https://github.com/RISCV-on-Microsemi-FPGA/PolarFire-Eval-Kit/tree/master/Programming_The_Target_Device/PF_MIV_RV32IMA_L1_AHB_BaseDesign)

SmartFusion2-Advanced-Dev-Kit
- [SmartFusion2 Advanced Development Kit RISC-V Sample Design](https://github.com/RISCV-on-Microsemi-FPGA/SmartFusion2-Advanced-Dev-Kit/tree/master/Programming_The_Target_Device/PROC_SUBSYSTEM_MIV_RV32IMA_BaseDesign)

### How to use this project:
ThreadX for the Microsemi RISC-V is delivered as a set of SoftConsole projects. 
Please read ./x-ware_platform/readme_threadx.txt to know how to use these projects with SoftConsole IDE.

The application creates several threads thread_0_entry and thread_1_entry etc and demonstrates the usage of IPC such as queue, mutex etc.

### ThreadX project configurations:
The Demo project related configurations can be done in ./x-ware_platform/RISCV_Demo_Threadx/demo_threadx.c

The RISC-V creative board design uses 66Mhz processor clock. The PolarFire Eval Kit design uses 50Mhz processor clock. The SmartFusion2 Adv. Development kit design uses 83Mhz processor clock.

### Microsemi SoftConsole Toolchain:
To know more please refer: [SoftConsole](https://github.com/RISCV-on-Microsemi-FPGA/SoftConsole)

### Documentation for Microsemi RISC-V processor, SoftConsole toochain, Debug Tools, FPGA design etc.
To know more please refer: [Documentation](https://github.com/RISCV-on-Microsemi-FPGA/Documentation)
    

