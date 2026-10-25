# Silicum

UEFI (edk2) bring-up for the Redmi Note 12 Pro+ 5G (ruby / rubypro / rubyx,
MediaTek MT6877 / Dimensity 1080).

- `docs/recon/NOTES.md`   hardware facts from the live device
- `SilicumPkg/Include/`   platform constants
- `fbtest/`               bare-metal framebuffer test stub (builds in CI)
- `tools/recon.sh`        collects hardware data over adb (needs root)

CI builds four `fbtest` variants (stride 1080/1088 x XRGB/RGBA) on every push.
Download them from the run's Artifacts, then pack the `Image` into a boot image.
