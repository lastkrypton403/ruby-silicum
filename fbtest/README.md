# fbtest

Smallest possible payload to prove the boot path before any edk2 work:
paints color bars into LK's framebuffer at 0x7ba10000 and shows the EL.

Build (untested, written without a cross compiler on hand):

    make                      # default: XRGB order, stride 1080 px
    make clean && make STRIDE_PX=1088
    make clean && make PIXEL_RGBA=1

Pack: take your working boot image flow (same one used for mod.img), keep the
ramdisk/dtb/header settings from the crDroid boot image and replace only the
kernel with `Image`. `unpack_bootimg --boot_img boot-12-12.img --format=mkbootimg`
prints the exact arguments to reuse. The stub is position independent, so the
kernel load address does not matter.

Test with `fastboot boot` first. If that is not accepted by this LK, flash
boot_a only (you have a full dump) and keep boot_b stock.

Report back: do you see any colors, which order, is the border straight, how
many yellow squares. Also the unpack_bootimg output.
