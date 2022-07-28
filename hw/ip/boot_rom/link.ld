/* See LICENSE.SiFive for license details. */
OUTPUT_ARCH( "riscv" )
ENTRY( entry )
SECTIONS
{
    . = 0x000;
    .text :
    {
        *(.text)
    }
    _end = .;
}
