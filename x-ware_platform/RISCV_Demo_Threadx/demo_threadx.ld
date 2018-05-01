OUTPUT_ARCH(riscv)

/**** Start point ****/

ENTRY(__start)          /* Entry point of application       */

SECTIONS
{
  /**** Code and read-only data ****/

  .text 0x80000000 :
  {
    *entry.o(.text.entry);
    _ftext = ABSOLUTE(.) ;  /* Start of code and read-only data */
    *(.text*)
    _ecode = ABSOLUTE(.) ;  /* End of code              */

    *(.rodata*)

    . = ALIGN(8);
    _etext = ABSOLUTE(.) ;  /* End of code and read-only data   */
  } = 0

  /**** Initialised data ****/

  .data :
  {
    _fdata = ABSOLUTE(.);   /* Start of initialised data        */
    *(.data*)
   
    . = ALIGN(8);

    _gp = ABSOLUTE(. + 0x7ff0); /* Base of small data           */
    LC8 = ABSOLUTE(. + 0x7ff0); /* Base of small data           */

    *(.lit8) 
    *(.lit4) 
    *(.sdata*) 

    . = ALIGN(8);

    _edata  = ABSOLUTE(.) ; /* End of initialised data      */
  }

  /**** Uninitialised data ****/

  .sbss : 
  { 
    *(.sbss*) 
    *(.scommon)
  }
  .bss :
  {
    *(.bss*)
    *(COMMON)
    _ebss = ABSOLUTE(.) ;
  }

  .stack _ebss :
  {
    /* Allocate room for stack */
    .   =  ALIGN(8) ;
    .   += 4096 ;
    _sp =  . - 16 ;
    _stack_top = ABSOLUTE(.) ;
  }

  _free_memory =  _stack_top + 4 ;

  .gptab.sdata : { *(.gptab.data) *(.gptab.sdata) }
  .gptab.sbss : { *(.gptab.bss) *(.gptab.sbss) }

  /DISCARD/ :
  {
    *(.reginfo)
  }

  PROVIDE(etext = _etext);
  PROVIDE (end = _stack_top);
}