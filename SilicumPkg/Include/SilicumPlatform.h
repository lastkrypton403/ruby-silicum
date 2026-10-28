/*
 * Silicum platform constants - MT6877 (Dimensity 1080), Redmi Note 12 Pro+ 5G
 * (sku rubypro; ruby/rubyx share the SoC). Every value below comes from the
 * live DT / iomem dump taken on a rubypro running crDroid 12.12
 * (kernel 4.19.325). Items marked VERIFY are not confirmed yet.
 */
#ifndef SILICUM_PLATFORM_H_
#define SILICUM_PLATFORM_H_

/* ---- DRAM: memory/reg = <0 0x40000000 2 0x0> -> 8 GiB, contiguous ---- */
#define SILICUM_DRAM_BASE            0x40000000ULL
#define SILICUM_DRAM_SIZE            0x200000000ULL   /* 0x40000000..0x23fffffff */

/* LK loads the kernel image at 0x40080000 (iomem: "Kernel code" starts here).
 * Free low RAM before the first reserved block (aee_debug_kinfo) is
 * 0x40000000..0x48080000. */
#define SILICUM_FD_BASE              0x40080000ULL    /* boot.img v2 header kernel_addr (confirmed) */
#define SILICUM_DTB_ADDR             0x47C80000ULL    /* boot.img tags_addr; LK-provided DT (expect in x0) */
#define SILICUM_FD_SIZE              0x00400000ULL    /* 4 MiB, adjust to FD size */
#define SILICUM_LOW_FREE_END         0x48080000ULL

/* ---- GICv3 (interrupt-controller, compatible arm,gic-v3) ---- */
#define SILICUM_GICD_BASE            0x0C000000ULL
#define SILICUM_GICD_SIZE            0x00040000ULL
#define SILICUM_GICR_BASE            0x0C040000ULL
#define SILICUM_GICR_SIZE            0x00200000ULL

/* ---- ARM generic timer: clock-frequency = 0xC65D40 ---- */
#define SILICUM_TIMER_FREQ_HZ        13000000U

/* ---- PSCI 1.0 over SMC (psci node), CPUs use enable-method psci ---- */
#define SILICUM_PSCI_CONDUIT_SMC     1

/* ---- UARTs: mediatek,mt6577-uart (16550 compatible). The stock kernel
 * disables UART output (mtk_printk_ctrl.disable_uart=1) and the phone
 * exposes no known pads, so do not rely on it for debugging. ---- */
#define SILICUM_UART0_BASE           0x11002000ULL    /* console ttyS0 @ 921600 */
#define SILICUM_UART1_BASE           0x11003000ULL

/* ---- Display: LK framebuffer (reserved-memory mblock-20-framebuffer) ----
 * Panel: panel-m16-42-0d-0a-dsc-vdo, 1080x2400, DSC, video mode.
 * Android fbdev reports 1088x7200 virtual, stride 4352 B, 32 bpp (3 buffers
 * of 2400 rows, width padded to 1088). LK's own stride/pixel order are
 * VERIFY: draw colour bars and check. */
#define SILICUM_FB_BASE              0x7BA10000ULL
#define SILICUM_FB_SIZE              0x01DF0000ULL
#define SILICUM_FB_WIDTH             1080
#define SILICUM_FB_HEIGHT            2400
#define SILICUM_FB_BPP               32
#define SILICUM_FB_STRIDE_GUESS      4320             /* 1080*4; alt 4352 (1088*4) */

/* ---- Storage / USB (later stages) ---- */
#define SILICUM_UFSHCI_BASE          0x11270000ULL    /* mediatek,mt8183-ufshci, SPI 137 */
#define SILICUM_MTU3_MAC_BASE        0x11201000ULL    /* usb0, dr_mode otg */
#define SILICUM_MTU3_IPPC_BASE       0x11203E00ULL
#define SILICUM_XHCI_BASE            0x11200000ULL    /* xhci0 child node */

/* ---- Debug: pstore/ramoops region, readable from Android after reboot ---- */
#define SILICUM_RAMOOPS_BASE         0x48090000ULL
#define SILICUM_RAMOOPS_SIZE         0x000E0000ULL

/* ---- Ramdisk placement chosen by LK (chosen/linux,initrd-*) ---- */
#define SILICUM_INITRD_BASE          0x51100000ULL    /* boot.img ramdisk_addr (confirmed) */

/* ---- Fixed reserved regions from DT /reserved-memory (reg present).
 * UEFI must NOT hand these to the OS as conventional memory.
 * NoMap=1 -> EfiReservedMemoryType, NoMap=0 -> still firmware-owned or
 * shared, keep reserved for the first bring-up. ---- */
typedef struct {
  UINT64  Base;
  UINT64  Size;
  UINT8   NoMap;
  CONST CHAR8 *Name;
} SILICUM_RESERVED_REGION;

#define SILICUM_RESERVED_REGIONS                                              \
  { 0x048080000ULL, 0x00010000ULL, 1, "aee_debug_kinfo" },                    \
  { 0x048090000ULL, 0x000E0000ULL, 1, "pstore" },                             \
  { 0x048170000ULL, 0x00010000ULL, 1, "minirdump" },                          \
  { 0x048C00000ULL, 0x00200000ULL, 1, "atf-reserved" },                       \
  { 0x06C000000ULL, 0x03560000ULL, 1, "ap_md_c_smem" },                       \
  { 0x070000000ULL, 0x06000000ULL, 1, "tee-reserved" },                       \
  { 0x076000000ULL, 0x00A20000ULL, 1, "adsp" },                               \
  { 0x077FF0000ULL, 0x00010000ULL, 1, "SPM-reserved" },                       \
  { 0x078000000ULL, 0x001D0000ULL, 1, "ap_md_nc_smem" },                      \
  { 0x07BA10000ULL, 0x01DF0000ULL, 0, "framebuffer" },                        \
  { 0x07D800000ULL, 0x00C00000ULL, 1, "tee-ree-reserved" },                   \
  { 0x07E400000ULL, 0x00400000ULL, 1, "tee-secmem" },                         \
  { 0x07E800000ULL, 0x00041000ULL, 0, "gz-log" },                             \
  { 0x07E900000ULL, 0x00100000ULL, 1, "MCUPM-reserved" },                     \
  { 0x07EA00000ULL, 0x01400000ULL, 1, "gz" },                                 \
  { 0x07FEB0000ULL, 0x00100000ULL, 1, "SSPM-reserved" },                      \
  { 0x07FFBF000ULL, 0x00040000ULL, 0, "log_store" },                          \
  { 0x07FFFF000ULL, 0x00001000ULL, 1, "emi_mbist_buf" },                      \
  { 0x0B8400000ULL, 0x00400000ULL, 1, "SCP-reserved" },                       \
  { 0x0B8880000ULL, 0x00F80000ULL, 1, "vpu_binary" },                         \
  { 0x0BFC00000ULL, 0x00020000ULL, 1, "tee-log-reserved" },                  \
  { 0x0BFD00000ULL, 0x00080000ULL, 1, "PICACHU" },                            \
  { 0x0BFE00000ULL, 0x00040000ULL, 1, "atf-log-reserved" },                  \
  { 0x0BFF70000ULL, 0x00010000ULL, 1, "ccci_tag_mem" },                       \
  { 0x0BFF80000ULL, 0x00080000ULL, 1, "atf-ramdump-reserved" },              \
  { 0x0E0000000ULL, 0x02060000ULL, 1, "md_mem_usage_25" },                    \
  { 0x0E2060000ULL, 0x00B70000ULL, 0, "drdi_memory" },                        \
  { 0x0E2BD0000ULL, 0x00010000ULL, 1, "md_mem_usage_26" },                    \
  { 0x0E3000000ULL, 0x00150000ULL, 1, "md_mem_usage_27" },                    \
  { 0x0E4000000ULL, 0x07710000ULL, 1, "md_mem_usage_28" },                    \
  { 0x0FB180000ULL, 0x02180000ULL, 1, "md_mem_usage_29" },                    \
  { 0x0FFE60000ULL, 0x00190000ULL, 1, "DPM-reserved" },                       \
  { 0x13FFFF000ULL, 0x00001000ULL, 1, "dramc-rk0" },                          \
  { 0x23FFFF000ULL, 0x00001000ULL, 1, "dramc-rk1" }

#endif /* SILICUM_PLATFORM_H_ */
