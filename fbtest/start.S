/* Silicum fbtest: tiny AArch64 stub packaged as an arm64 "Image".
 * Position independent (no absolute addresses except fixed hardware), so it
 * works wherever LK loads the kernel. Enters with MMU/caches off at EL1,
 * x0 = device tree pointer (arm64 boot protocol).
 *
 * The very first thing it does is write "ST1\n" into the Android ramoops
 * console zone (0x4811f000), so a reboot into Android can prove it ran:
 *   cat /sys/fs/pstore/console-ramoops-0
 */
    .section .text.boot, "ax"
    .global _start
_start:
    b       1f                      /* code0: branch over the header */
    .word   0xd503201f              /* code1: nop */
    .quad   0                       /* text_offset (0: placement-independent) */
    .quad   _image_size             /* image_size */
    .quad   0xa                     /* flags: LE, 4K pages, place anywhere */
    .quad   0                       /* res2 */
    .quad   0                       /* res3 */
    .quad   0                       /* res4 */
    .word   0x644d5241              /* magic "ARM\x64" */
    .word   0                       /* res5 */
1:
    mov     x19, x0                 /* keep the dtb pointer */
    msr     daifset, #0xf           /* mask all interrupts */

    /* proof-of-life marker in the ramoops console zone (persistent_ram_buffer:
     * u32 sig 'DBGC', u32 start, u32 size, data[]) */
    movz    x2, #0x4811, lsl #16
    movk    x2, #0xf000
    movz    w3, #0x4244
    movk    w3, #0x4347, lsl #16
    str     w3, [x2]                /* sig = 0x43474244 */
    str     wzr, [x2, #4]           /* start = 0 */
    mov     w3, #4
    str     w3, [x2, #8]            /* size = 4 */
    movz    w3, #0x5453
    movk    w3, #0x0a31, lsl #16
    str     w3, [x2, #12]           /* "ST1\n" */
    dsb     sy

    adr     x2, vectors
    msr     vbar_el1, x2
    isb

    mrs     x0, CurrentEL
    lsr     x0, x0, #2              /* x0 = exception level number */
    mov     x1, x19                 /* x1 = dtb pointer */
    adr     x2, _stack_top
    mov     sp, x2
    bl      main
2:  wfe
    b       2b

/* ---- exception vectors: log the fault, then reset ---- */
    .macro ventry n
    .balign 0x80
    mov     x0, #\n
    b       exc_common
    .endm

    .section .text.vectors, "ax"
    .balign 2048
    .global vectors
vectors:
    ventry 0
    ventry 1
    ventry 2
    ventry 3
    ventry 4
    ventry 5
    ventry 6
    ventry 7
    ventry 8
    ventry 9
    ventry 10
    ventry 11
    ventry 12
    ventry 13
    ventry 14
    ventry 15

exc_common:
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    mrs     x3, far_el1
    adr     x4, _stack_top
    mov     sp, x4
    bl      exception
3:  wfe
    b       3b
