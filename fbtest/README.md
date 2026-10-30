# fbtest v2

Smallest possible payload to prove the boot path before any edk2 work. It works
even when the screen stays black, because it logs into the pstore/ramoops
console zone (0x4811f000) which the stock Android kernel exposes after the next
boot.

Build: `make` (CI builds XRGB/RGBA variants; the pixel order only matters for
the colour bands). No cross compiler is needed to read the logic, but one is
needed to build: `sudo dnf install gcc-aarch64-linux-gnu binutils-aarch64-linux-gnu`.

## How to run a test and read the result

1. Pack with `tools/swap_kernel.py` (use `--gzip` if the raw Image is refused).
2. **Use `fastboot boot test.img`**, not a flash, so that after the watchdog
   reset (about 14 s) the phone starts the normal stock boot and the log in
   DRAM is still there. If you must flash, flash `boot_b` and keep `boot_a`
   stock, and get back to Android without a long power-off.
3. When Android is up, as root:

       ls /sys/fs/pstore
       su -c 'cat /sys/fs/pstore/console-ramoops-0' > silicum-log.txt

   Send me `silicum-log.txt`. The first line is `SILICUM fbtest v2` if the
   stub ran at all.

If `/sys/fs/pstore` is empty or has no `console-ramoops-0`, say so. Either DRAM
was not preserved across the reset, the kernel has no ramoops, or the stub never
ran.

## What the log tells us

- `CurrentEL`: 4 = EL1, 8 = EL2.
- `entry_x0` and `dtb_totalsize`: whether the arm64 boot protocol DTB pointer
  arrives (expected 0x47c80000).
- `PSCI_VERSION`: 0x10001 = PSCI 1.1, confirming SMC works from here.
- `fb survey`: where in LK's framebuffer memory there is non-zero content
  (the boot logo), which tells us buffer layout and stride without needing sight.
- `t=N`: last heartbeat before the reset gives the exact watchdog timeout.

On screen (if it shows): 256 KiB bands, red, green, blue, white repeating down
the panel. If red appears blue, the pixel order is RGBA (use that CI artifact).

## Reading the result (current stub)

The stub now reboots the phone itself 6 seconds after painting (PSCI SYSTEM_RESET).

- Reboots after ~6 s: the stub ran. Whatever the screen showed is real.
- Reboots after ~14 s (the old behaviour): it did not run or hung early.
- Screen: whole screen magenta first, then bars if the pattern base is scanned.
  Black the whole time while it reboots at ~6 s means code runs but LK has the
  display off or scans out elsewhere (needs display bring-up, a bigger task).

## Log channel (pstore)

The stub writes a log into the Android ramoops console zone (0x4811f000).
After the phone is back in Android (if you flashed the stub to boot_a, put the
stock boot image back first and boot normally):

    adb shell su -c 'ls /sys/fs/pstore; cat /sys/fs/pstore/console-ramoops-0'
    adb shell su -c 'dmesg | grep -i persistent_ram'

Expected if it ran: "ST1", "main", el/dtb/midr values, "magenta done", ...
"EXC" with idx/esr/elr/far means it crashed; send those values.
If the log is missing, the stub never ran, or the memory did not survive
the reset (then the dmesg persistent_ram lines still tell us what the
kernel found: "found existing buffer", "no valid data", or "invalid").
