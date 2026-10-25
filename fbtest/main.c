/* Silicum fbtest: paints test patterns into LK's framebuffer.
 *
 * Reading the result on the phone screen:
 *   - Top band is meant to be RED. If it shows BLUE, the pixel order is
 *     RGBA in memory: rebuild with PIXEL_RGBA=1.
 *   - White border. If the right/bottom edges look sheared or drift
 *     diagonally, the stride is wrong: rebuild with STRIDE_PX=1088.
 *   - Yellow squares at the bottom = current exception level (1 square = EL1).
 */
typedef unsigned int  u32;
typedef unsigned long u64;

#ifndef STRIDE_PX
#define STRIDE_PX 1080
#endif

#define FB_BASE 0x7BA10000UL
#define W 1080
#define H 2400

#ifdef PIXEL_RGBA
#define C(r,g,b) (0xFF000000u | ((u32)(b) << 16) | ((u32)(g) << 8) | (u32)(r))
#else
#define C(r,g,b) (0xFF000000u | ((u32)(r) << 16) | ((u32)(g) << 8) | (u32)(b))
#endif

static void fill(int x0, int y0, int x1, int y1, u32 c)
{
    volatile u32 *fb = (volatile u32 *)FB_BASE;
    for (int y = y0; y < y1; y++) {
        volatile u32 *row = fb + (u64)y * STRIDE_PX;
        for (int x = x0; x < x1; x++)
            row[x] = c;
    }
}

void main(u64 el)
{
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

    for (;;)
        __asm__ volatile("wfe");
}
