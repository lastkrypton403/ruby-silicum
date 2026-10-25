#!/usr/bin/env bash
# Silicum recon: collect the facts needed to write the edk2 platform package.
# Run on your PC with the phone connected via adb and root (su) available.
# Usage: ./recon.sh [outdir]
set -u
OUT="${1:-recon-$(date +%Y%m%d-%H%M%S)}"
mkdir -p "$OUT"

sh() { adb shell "su -c '$1'" < /dev/null; }

# Preflight: fail loudly if adb or root is not working (empty output files mean this failed).
adb get-state >/dev/null 2>&1 || { echo "ERROR: no adb device (check cable, USB debugging, authorize this PC)"; exit 1; }
WHO="$(adb shell 'su -c id' < /dev/null 2>&1 | tr -d '\r')"
case "$WHO" in
  *uid=0*) echo "[ok] root via su: $WHO" ;;
  *) echo "ERROR: root not available to adb shell. su said: $WHO"
     echo "KernelSU: open the manager app > Superuser tab > grant root to 'Shell' (com.android.shell),"
     echo "then re-run. If 'su -c' is not accepted, try: adb shell su 0 id   or   adb root"
     exit 1 ;;
esac

echo "[*] basic identity"
sh 'cat /proc/cmdline'                         > "$OUT/cmdline.txt"
sh 'getprop ro.product.device; getprop ro.boot.hardware.sku; getprop ro.hardware' > "$OUT/props.txt"
sh 'uname -a'                                  > "$OUT/uname.txt"

echo "[*] exception level the kernel was started at (EL1 vs EL2)"
sh 'dmesg | grep -iE "started at EL|booted at EL|CPUs started"' > "$OUT/el.txt"

echo "[*] physical memory map"
sh 'cat /proc/iomem'                           > "$OUT/iomem.txt"
sh 'dmesg | grep -iE "memblock|reserved|no-map|Zone ranges|early_memtest"' > "$OUT/dmesg-mem.txt"

echo "[*] framebuffer / display handoff from LK"
sh 'dmesg | grep -iE "simplefb|framebuffer|fb0|lk_fb|bootloader|logo|dsi|dsc|mtk_drm"' > "$OUT/dmesg-fb.txt"
sh 'cat /sys/class/graphics/fb0/{virtual_size,stride,bits_per_pixel,name} 2>/dev/null' > "$OUT/fb0.txt"
sh 'wm size; wm density' > "$OUT/wm.txt" 2>/dev/null

echo "[*] interrupt controller, timer, UART"
sh 'dmesg | grep -iE "GICv3|gic|arch_timer|timer.*MHz|8250|ttyS|uart|earlycon"' > "$OUT/dmesg-irq-timer-uart.txt"
sh 'cat /proc/interrupts | head -40'           > "$OUT/interrupts.txt"

echo "[*] PSCI / secure monitor"
sh 'dmesg | grep -iE "psci|smccc|optee|trusty|tee|spmd|atf"' > "$OUT/dmesg-psci.txt"

echo "[*] storage"
sh 'ls -l /dev/block/by-name/'                 > "$OUT/by-name.txt"
sh 'dmesg | grep -iE "ufs|ufshcd"'             > "$OUT/dmesg-ufs.txt"

echo "[*] device tree (live)"
if adb shell 'command -v dtc' >/dev/null 2>&1; then
  sh 'dtc -I fs -O dts /proc/device-tree'      > "$OUT/live.dts"
else
  sh 'tar -C /proc/device-tree -cf /data/local/tmp/dt.tar .'
  adb pull /data/local/tmp/dt.tar "$OUT/dt.tar" >/dev/null 2>&1
  echo "dtc not on phone; decompile on PC: mkdir dt && tar -xf $OUT/dt.tar -C dt (properties are raw binary)" > "$OUT/DT_NOTE.txt"
fi

echo "[*] done -> $OUT/"
echo "Share: el.txt, iomem.txt, dmesg-mem.txt, dmesg-fb.txt, fb0.txt, dmesg-irq-timer-uart.txt, dmesg-psci.txt, and live.dts (or dt.tar)."
