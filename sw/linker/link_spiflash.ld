ENTRY(_start)

MEMORY
{
    FLASH (rx)      : ORIGIN = 0x40000000, LENGTH = 0x1000000 /* entire flash, 16 MB */
    RAM (xrw)       : ORIGIN = 0x00000004, LENGTH = 0x000FFFC /* entire ram, 64 KB */
}

SECTIONS {

    /* we want a fixed entry point */
    PROVIDE(__boot_address = 0x40000180);

    /* stack and heap related settings */
    __stack_size = DEFINED(__stack_size) ? __stack_size : 0x1000;
    PROVIDE(__stack_size = __stack_size);
    __heap_size = DEFINED(__heap_size) ? __heap_size : 0x1000;

 /* interrupt vectors */
    .vectors (ORIGIN(FLASH)):
    {
      PROVIDE(_vector_start = .);
      KEEP(*(.vectors));
    } >FLASH


    /* crt0 init code */
    .init (__boot_address):
    {
        KEEP (*(SORT_NONE(.init)))
	KEEP (*(.text.start))
    } >FLASH

    
    /* The program code and other data goes into FLASH */
    .text :
    {
        . = ALIGN(4);
        *(.text)           /* .text sections (code) */
        *(.text*)          /* .text* sections (code) */
        *(.rodata)         /* .rodata sections (constants, strings, etc.) */
        *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
        *(.srodata)        /* .rodata sections (constants, strings, etc.) */
        *(.srodata*)       /* .rodata* sections (constants, strings, etc.) */
        . = ALIGN(4);
        _etext = .;        /* define a global symbol at end of code */
        _sidata = _etext;  /* This is used by the startup in order to initialize the .data secion */
    } >FLASH

   

    /* This is the initialized data section
    The program executes knowing that the data is in the RAM
    but the loader puts the initial values in the FLASH (inidata).
    It is one task of the startup to copy the initial values from FLASH to RAM. */
    .data : AT ( _sidata )
    {
        . = ALIGN(4);
        _sdata = .;        /* create a global symbol at data start; used by startup code in order to initialise the .data section in RAM */
        _ram_start = .;    /* create a global symbol at ram start for garbage collector */
        . = ALIGN(4);
        __DATA_BEGIN__ = .;
        *(.data)           /* .data sections */
        *(.data*)          /* .data* sections */
        __SDATA_BEGIN__ = .;
        *(.sdata)           /* .sdata sections */
        *(.sdata*)          /* .sdata* sections */
        . = ALIGN(4);
        _edata = .;        /* define a global symbol at data end; used by startup code in order to initialise the .data section in RAM */
    } >RAM

    /* Uninitialized data section */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;         /* define a global symbol at bss start; used by startup code */
        *(.bss)
        *(.bss*)
        *(.sbss)
        *(.sbss*)
        *(COMMON)

        . = ALIGN(4);
        _ebss = .;         /* define a global symbol at bss end; used by startup code */
        __BSS_END__ = .;
    } >RAM

    /* The compiler uses this to access data in the .sdata, .data, .sbss and .bss
     sections with fewer instructions (relaxation). This reduces code size. */
    __global_pointer$ = MIN(__SDATA_BEGIN__ + 0x800,
			    MAX(__DATA_BEGIN__ + 0x800, __BSS_END__ - 0x800));

    /* this is to define the start of the heap, and make sure we have a minimum size */
    .heap          :
    {
     PROVIDE(__heap_start = .);
    . = __heap_size;
    PROVIDE(__heap_end = .);
    } >RAM

    /* stack: we should consider putting this further to the top of the address
    space */
  .stack         : ALIGN(16) /* this is a requirement of the ABI(?) */
  {
   PROVIDE(__stack_start = .);
   . = __stack_size;
   PROVIDE(_sp = .);
   PROVIDE(__stack_end = .);
  } >RAM
}
