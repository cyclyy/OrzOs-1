/* Link.ld -- Linker script for the programs - ensure everything goes in the */
/*            Correct place.  */

PAGE_SIZE = 0x1000;
STARTUP(/home/ora/cross/x86_64-pc-orzos/lib/crt0.o)
ENTRY(_start)
OUTPUT_FORMAT(elf64-x86-64)
SEARCH_DIR(/home/ora/cross/x86_64-pc-orzos/lib)
SECTIONS
{
  . = 0x0000040000000000;
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
  . = ALIGN(0x1000);
  }

  .bss :
  {
    __bss_start = .; bss = .; _bss = .; __bss = .;
    *(.bss)
  . = ALIGN(0x1000);
  }

  end = .; _end = .; __end = .;
} 
