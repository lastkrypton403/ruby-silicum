/* Silicum fbtest: tiny AArch64 stub packaged as an arm64 "Image".
 * Position independent (no absolute addresses except the framebuffer), so it
 * works wherever LK loads the kernel. Enters with MMU/caches off at EL1. */
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
    msr     daifset, #0xf           /* mask all interrupts */
    mrs     x0, CurrentEL
    lsr     x0, x0, #2              /* x0 = exception level number */
    adr     x1, _stack_top
    mov     sp, x1
    bl      main
2:  wfe
    b       2b
