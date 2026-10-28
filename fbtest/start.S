/* Silicum fbtest: tiny AArch64 stub packaged as an arm64 "Image".
 * Position independent (no absolute addresses except fixed hardware/RAM
 * addresses used by main.c). Enters with MMU/caches off at EL1.
 * x0..x3 are passed through to main() untouched (x0 = DTB pointer per the
 * arm64 boot protocol). */
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
    adr     x4, _stack_top          /* x4: do not clobber x0..x3 */
    mov     sp, x4
    bl      main
2:  wfe
    b       2b
