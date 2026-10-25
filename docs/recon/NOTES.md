# Silicum recon notes (rubypro, MT6877)

Source: recon run on-device, crDroid 12.12, kernel 4.19.325-Eclipse-v2.2.

## Confirmed from DT / iomem
- DRAM: 8 GiB contiguous at 0x40000000 (0x40000000..0x23fffffff)
- GICv3: GICD 0x0c000000 (0x40000), GICR 0x0c040000 (0x200000)
- Arch timer: 13 MHz
- PSCI 1.0, method smc; cpus use enable-method psci (6x A55 + 2x A78)
- UART0 0x11002000, UART1 0x11003000 (mt6577-uart); kernel UART output disabled
- Display: DSC video-mode panel, 1080x2400. LK framebuffer reserved block at
  0x7ba10000, size 0x1df0000 (matches atag,videolfb fb base)
- UFS: ufshci 0x11270000 (mt8183-ufshci). GPT lives on sdc (LUN 2)
- USB: mtu3 0x11201000, xhci0 0x11200000, otg, high-speed max
- Boot partitions: lk_a/lk_b, boot_a/boot_b, vendor_boot_a/b, dtbo_a/b, tee_a/b
- Boot args contain bootopt=64S3,32N2,64N2

## Measured from fresh-boot dmesg
- "CPU: All CPU(s) started at EL1": the kernel (so any boot.img payload) is
  entered in AArch64 at EL1, NOT EL2. My earlier EL2 guess from bootopt was
  wrong. EL2 is most likely held by MediaTek GenieZone (gz_a/gz_b partitions,
  gz reserved block at 0x7ea00000, "GZ SMC" messages in dmesg). Not confirmed.
- PSCI v1.1 over SMC. GICv3 redistributors are 0x20000 apart starting at
  0x0c040000. MPIDRs are 0x000,0x100,...,0x700 (one core per Aff1 value).
  CPU0-5 Cortex-A55 (MIDR 0x412fd050), CPU6-7 Cortex-A78 (MIDR 0x411fd410).
- LK is 32-bit, so do not try to replace or patch lk_a. Chain-load from boot.img.
- The green/locked state in the boot args is spoofed (per the device owner);
  the bootloader is unlocked. Ignore it. Leave lk/preloader untouched and only
  test via `fastboot boot` or boot_a.

## Missing: need a fresh-boot dmesg
el.txt and dmesg-mem.txt came back empty because the kernel ring buffer had
rotated (uptime ~13 h). Right after a reboot run:

    adb shell su -c 'dmesg' > dmesg-boot.txt
    grep -E "started at EL|Booted|EL[0-3]|memblock|psci" dmesg-boot.txt

## Still unknown
- LK framebuffer stride and pixel order (test with colour bars)
- Exact kernel/ramdisk load addresses from the boot.img header
  (unpack boot_a, read kernel_offset / ramdisk_offset)
- Whether the display pipeline keeps scanning out 0x7ba10000 after handoff
- dmesg-psci.txt matched fingerprint TA logs only, no PSCI info; the DT psci node
  is enough
