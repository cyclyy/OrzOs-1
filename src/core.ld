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
    code = .;
    *(.text)
    . = ALIGN(4096);
  }

  .data :
  {
      data = .;
     *(.data)
     *(.rodata)
     . = ALIGN(4096);
  }

  .bss :
  {
    bss = .; 
    *(.bss)
    . = ALIGN(4096);
  }

  end = .;
} 
