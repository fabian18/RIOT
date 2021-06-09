/*
 * Copyright (C) 2021 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup         cpu_rp2040
 * @{
 *
 * @file
 * @brief           Memory definitions for the RP2040 MCU
 *
 * @author          Fabian Hüßler <fabian.huessler@ovgu.de>
 *
 * @}
 */

OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)

ENTRY(reset_handler_default)

MEMORY
{
    rom (rx)    : ORIGIN = 0x00000000, LENGTH = 16K
    flash(rx)   : ORIGIN = 0x10000000, LENGTH = 2048k
    ram(rwx)    : ORIGIN = 0x20000000, LENGTH = 256k
    proc1(rwx)  : ORIGIN = 0x20040000, LENGTH = 4k  /* exclusive for cpu1 */
    proc0(rwx)  : ORIGIN = 0x20041000, LENGTH = 4k  /* exclusive for cpu0 */
}

SECTIONS
{
    /* at the beginning of flash resides the bootloader */
    .boot2 : {
        __boot2_start__ = .;
        KEEP(*(.boot2))
        __boot2_end__ = .;
    } > flash

    ASSERT(__boot2_end__ - __boot2_start__ == 256,
        "ERROR: Pico second stage bootloader must be 256 bytes in size")

    .text :
    {
        . = ALIGN(4);
        _sfixed = .;
        _isr_vectors = DEFINED(_isr_vectors) ? _isr_vectors : . ;
        KEEP (*(SORT(.vectors*)))
        *(.text .text.* .gnu.linkonce.t.*)
        *(.glue_7t) *(.glue_7)
        *(.rodata .rodata* .gnu.linkonce.r.*)
        KEEP (*(SORT(.roxfa.*)))
        *(.ARM.extab* .gnu.linkonce.armextab.*)

        /* Support C constructors, and C destructors in both user code
           and the C library. This also provides support for C++ code. */
        . = ALIGN(4);
        KEEP (*(.init))
        . = ALIGN(4);
        __preinit_array_start = .;
        KEEP (*(.preinit_array))
        __preinit_array_end = .;

        . = ALIGN(4);
        __init_array_start = .;
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array))
        __init_array_end = .;

        . = ALIGN(0x4);
        KEEP (*crtbegin.o(.ctors))
        KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
        KEEP (*(SORT(.ctors.*)))
        KEEP (*crtend.o(.ctors))

        . = ALIGN(4);
        KEEP (*(.fini))

        . = ALIGN(4);
        __fini_array_start = .;
        KEEP (*(.fini_array))
        KEEP (*(SORT(.fini_array.*)))
        __fini_array_end = .;

        KEEP (*crtbegin.o(.dtors))
        KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
        KEEP (*(SORT(.dtors.*)))
        KEEP (*crtend.o(.dtors))

        . = ALIGN(4);
        _efixed = .;            /* End of text section */
    } > flash

    /*
     * TLS relocations are offsets relative to the address
     * of the first TLS symbol. That means we just need to
     * allocate them all together so that the TLS region
     * is compact when allocated for each thread.
     */

    /*
     * TLS initialization data is loaded into ROM so that
     * each thread can get its values initialized from there
     * at startup
     */
    .tdata :
    {
         __tdata_start = .;
        *(.tdata .tdata.* .gnu.linkonce.td.*)
        __tdata_end = .;
    } > flash
    __tdata_source = LOADADDR(.tdata);
    __tdata_size = SIZEOF(.tdata);

    /*
     * TLS zeroed data is relocated as if it immediately followed
     * the tdata values. However, the linker 'magically' erases the
     * memory allocation so that no ROM is consumed by this
     * section
     */
    .tbss :
    {
        *(.tbss .tbss.* .gnu.linkonce.tb.*)
        *(.tcommon)
        __tbss_end = .;
    } > flash
    __tls_size = __tbss_end - __tdata_start;
    __tbss_size = __tls_size - __tdata_size;

    /* .ARM.exidx is sorted, so has to go in its own output section.  */
    PROVIDE_HIDDEN (__exidx_start = .);
    .ARM.exidx :
    {
      *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > flash
    PROVIDE_HIDDEN (__exidx_end = .);

    /* exception handling */
    . = ALIGN(4);
    .eh_frame :
    {
        KEEP (*(.eh_frame))
    } > flash

    . = ALIGN(4);
    _etext = .;

    .relocate :
    {
        . = ALIGN(4);
        _srelocate = .;
        *(.ramfunc .ramfunc.*);
        *(.data .data.*);
        KEEP (*(SORT(.xfa.*)))
        KEEP (*(.openocd .openocd.*))
        . = ALIGN(4);
        _erelocate = .;
    } > ram AT> flash

    /* .bss section which is used for uninitialized data */
    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = . ;
        _szero = .;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = . ;
        _ezero = .;
    } > ram

    /*
     * collect all uninitialized sections that go into RAM
     */
    .noinit (NOLOAD) :
    {
        __noinit_start = .;
        *(.noinit)
        . = ALIGN(4);
        __noinit_end = .;
    } > ram

    /* heap section */
    .heap (COPY):
    {
        . = ALIGN(4);
        _sheap = . ;
        _eheap = ADDR(.pico-debugger);
    } > ram

    .pico-debugger 0x2003C000:
    {
        . = . + 16k;
    } > ram

    /* stack section */
    .stack (NOLOAD):
    {
        KEEP (*(.puf))
        . = ALIGN(8);
        _sstack = .;
        KEEP (*(.isr_stack))
        . = ALIGN(8);
        _estack = .;
    } > proc0

    /* Populate information about ram size */
    _sram = ORIGIN(ram);
    _eram = ORIGIN(ram) + LENGTH(ram) + LENGTH(proc1) + LENGTH(proc0);
}
