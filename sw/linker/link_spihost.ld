ENTRY(_start)

MEMORY
{
    FLASH_BOOT (rx) : ORIGIN = 0x40000000, LENGTH = 0x0000010 /* 16 bytes reserved for booting */
    FLASH_PROG (rx) : ORIGIN = 0x40000010, LENGTH = 0x0FFFFF0 /* entire remaining flash */
    RAM0 (rxw)      : ORIGIN = 0x00000000, LENGTH = 0x10000 /* entire ram, 64 KB */
}

SECTIONS {

  /* we want a fixed entry point */
  PROVIDE(__boot_address = 0x180);

  /* stack and heap related settings */
  __stack_size = DEFINED(__stack_size) ? __stack_size : 0x1000;
  PROVIDE(__stack_size = __stack_size);
  __heap_size = DEFINED(__heap_size) ? __heap_size : 0x1000;

  .spi_boot (ORIGIN(FLASH_BOOT)) :
  {
    LONG(ORIGIN(FLASH_PROG)); /*  flash source pointer */
    LONG(_etext); /* spi booting copy size in bytes */
    LONG(0x00000000); /* ram destination pointer */
    LONG(0x00000000); /* reserved */
  } >FLASH_BOOT

  /* interrupt vectors */
  .vectors (ORIGIN(RAM0)) :
  {
    PROVIDE(_vector_start = .);
    KEEP(*(.vectors));
  } >RAM0 AT >FLASH_PROG

  /* Fill section to avoid error while booting with SPI host (copy from flash to ram) */
  .fill :
  {
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
    LONG(0xDEADBEEF);
  } >RAM0 AT >FLASH_PROG

  /* crt0 init code */
  .init (__boot_address) :
  {
    KEEP (*(SORT_NONE(.init)));
    KEEP (*(.text.start));
  } >RAM0 AT >FLASH_PROG
  
  /* The program code and other data goes into FLASH_PROG */
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
  } >RAM0 AT >FLASH_PROG

   

  /* This is the initialized data section
  The program executes knowing that the data is in the RAM
  but the loader puts the initial values in the FLASH_PROG (inidata).
  It is one task of the startup to copy the initial values from FLASH_PROG to RAM. */
  .data : 
  {
    . = ALIGN(4);
    _sidata = LOADADDR(.data);
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
  } >RAM0 AT >FLASH_PROG

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
  } >RAM0 AT >FLASH_PROG

  /* The compiler uses this to access data in the .sdata, .data, .sbss and .bss
   sections with fewer instructions (relaxation). This reduces code size. */
  __global_pointer$ = MIN(__SDATA_BEGIN__ + 0x800, MAX(__DATA_BEGIN__ + 0x800, __BSS_END__ - 0x800));

  /* this is to define the start of the heap, and make sure we have a minimum size */
  .heap :
  {
    PROVIDE(__heap_start = .);
    . = __heap_size;
    PROVIDE(__heap_end = .);
  } >RAM0 AT >FLASH_PROG

  /* stack: we should consider putting this further to the top of the address
  space */
  .stack : ALIGN(16) /* this is a requirement of the ABI(?) */
  {
    PROVIDE(__stack_start = .);
    . = __stack_size;
    PROVIDE(_sp = .);
    PROVIDE(__stack_end = .);
  } >RAM0 AT >FLASH_PROG
}
