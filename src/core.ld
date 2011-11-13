/* Link.ld -- Linker script for the kernel - ensure everything goes in the */
/*            Correct place.  */
/*            Original file taken from Bran's Kernel Development */
/*            tutorials: http://www.osdever.net/bkerndev/index.php. */

ENTRY(kmain)
OUTPUT_FORMAT(elf64-x86-64)
SECTIONS
{
  . = 0xffffffffC0100000;
  .text :
  {
    code = .; _code = .; __code = .;
    *(.text)
     . = ALIGN(0x1000);
  }

  .data :
  {
     data = .; _data = .; __data = .;
     *(.data)
     *(.rodata)
     . = ALIGN(8);
     exceptTable = .;
     *(.exTable)
     exceptTableEnd = .;
     . = ALIGN(0x1000);
  }

  .bss :
  {
    bss = .; _bss = .; __bss = .;
    *(.bss)
     . = ALIGN(0x1000);
  }

  end = .; _end = .; __end = .;
} 
