/* Silicum fbtest: paints test patterns into LK's framebuffer, then proves it
 * is alive by rebooting itself after 6 seconds through PSCI SYSTEM_RESET.
 *
 * Proof of life (does not need the display):
 *   - the phone reboots after ~6 s  -> this code ran
 *   - the phone reboots after ~14 s -> the stock watchdog fired instead
 *     (code did not run, or hung before the delay finished)
 *
 * Log channel (works without a display): messages are appended to the
 * Android ramoops console zone at 0x4811f000. After the phone is back in
 * Android:  cat /sys/fs/pstore/console-ramoops-0   (or dmesg | grep persistent_ram)
 *
 * Display, if it works:
 *   - whole reserved framebuffer region goes MAGENTA first (shows the display
 *     scans out somewhere in that region), then the test pattern is drawn at
 *     its base:
 *   - Top band is meant to be RED. If it shows BLUE, rebuild with PIXEL_RGBA=1.
 *   - White border. If the right/bottom edges look sheared, rebuild with
 *     STRIDE_PX=1088.
 *   - Yellow squares at the bottom = current exception level (1 square = EL1).
 */
typedef unsigned int  u32;
typedef unsigned long u64;

#ifndef STRIDE_PX
#define STRIDE_PX 1080
#endif

#define FB_BASE 0x7BA10000UL
#define FB_SIZE 0x01DF0000UL
#define W 1080
#define H 2400

#ifdef PIXEL_RGBA
#define C(r,g,b) (0xFF000000u | ((u32)(b) << 16) | ((u32)(g) << 8) | (u32)(r))
#else
#define C(r,g,b) (0xFF000000u | ((u32)(r) << 16) | ((u32)(g) << 8) | (u32)(b))
#endif


/* ---- pstore (ramoops console zone) log ---- */
#define PS_BASE     0x4811F000UL
#define PS_DATA_MAX (0x40000UL - 12)

static void plog(const char *s)
{
    volatile u32 *hdr = (volatile u32 *)PS_BASE;
    volatile unsigned char *d = (volatile unsigned char *)(PS_BASE + 12);
    u32 n = hdr[2];
    while (*s && n < PS_DATA_MAX)
        d[n++] = (unsigned char)*s++;
    hdr[2] = n;
    __asm__ volatile("dsb sy" ::: "memory");
}

static void plog_hex(const char *label, u64 v)
{
    char b[24];
    int i = 0;
    b[i++] = '0';
    b[i++] = 'x';
    for (int sh = 60; sh >= 0; sh -= 4) {
        u32 dgt = (u32)((v >> sh) & 15);
        b[i++] = (char)(dgt < 10 ? '0' + dgt : 'a' + dgt - 10);
    }
    b[i++] = '\n';
    b[i] = 0;
    plog(label);
    plog("=");
    plog(b);
}

static void fill(int x0, int y0, int x1, int y1, u32 c)
{
    volatile u32 *fb = (volatile u32 *)FB_BASE;
    for (int y = y0; y < y1; y++) {
        volatile u32 *row = fb + (u64)y * STRIDE_PX;
        for (int x = x0; x < x1; x++)
            row[x] = c;
    }
}

static void fill_region_magenta(void)
{
    volatile u64 *p = (volatile u64 *)FB_BASE;
    for (u64 i = 0; i < FB_SIZE / 8; i++)
        p[i] = 0xFFFF00FFFFFF00FFUL;   /* magenta in both pixel orders */
}

static inline u64 cnt(void)
{
    u64 v;
    __asm__ volatile("isb\n\tmrs %0, cntvct_el0" : "=r"(v) :: "memory");
    return v;
}

static void wait_seconds(u32 s)
{
    u64 f;
    __asm__ volatile("mrs %0, cntfrq_el0" : "=r"(f));
    if (!f)
        f = 13000000UL;
    u64 end = cnt() + f * s;
    while (cnt() < end)
        ;
}

static void psci_system_reset(void)
{
    register u64 x0 __asm__("x0") = 0x84000009UL;   /* PSCI SYSTEM_RESET */
    __asm__ volatile("smc #0"
                     : "+r"(x0)
                     :
                     : "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9",
                       "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17",
                       "memory");
}

void exception(u64 idx, u64 esr, u64 elr, u64 far)
{
    plog("EXC\n");
    plog_hex("idx", idx);
    plog_hex("esr", esr);
    plog_hex("elr", elr);
    plog_hex("far", far);
    wait_seconds(2);
    psci_system_reset();
    for (;;)
        __asm__ volatile("wfe");
}

void main(u64 el, u64 dtb)
{
    u64 midr, mpidr, sctlr, frq;
    __asm__ volatile("mrs %0, midr_el1" : "=r"(midr));
    __asm__ volatile("mrs %0, mpidr_el1" : "=r"(mpidr));
    __asm__ volatile("mrs %0, sctlr_el1" : "=r"(sctlr));
    __asm__ volatile("mrs %0, cntfrq_el0" : "=r"(frq));
    plog("main\n");
    plog_hex("el", el);
    plog_hex("dtb", dtb);
    plog_hex("midr", midr);
    plog_hex("mpidr", mpidr);
    plog_hex("sctlr", sctlr);
    plog_hex("cntfrq", frq);

    fill_region_magenta();
    plog("magenta done\n");

    fill(0,    0, W,  400, C(255, 0, 0));       /* red   */
    fill(0,  400, W,  800, C(0, 255, 0));       /* green */
    fill(0,  800, W, 1200, C(0, 0, 255));       /* blue  */
    fill(0, 1200, W, 1600, C(255, 255, 255));   /* white */
    fill(0, 1600, W, 2000, C(128, 128, 128));   /* gray  */
    fill(0, 2000, W, H,    C(0, 0, 0));         /* black */

    /* 4 px white border for the stride check */
    fill(0, 0, W, 4, C(255, 255, 255));
    fill(0, H - 4, W, H, C(255, 255, 255));
    fill(0, 0, 4, H, C(255, 255, 255));
    fill(W - 4, 0, W, H, C(255, 255, 255));

    /* one yellow square per EL number */
    for (u64 i = 0; i < el; i++)
        fill(40 + (int)i * 140, 2150, 140 + (int)i * 140, 2250, C(255, 255, 0));

    plog("pattern done, waiting 6s then PSCI reset\n");
    wait_seconds(6);
    plog("calling reset\n");
    psci_system_reset();
    plog("reset returned?!\n");

    for (;;)
        __asm__ volatile("wfe");
}
