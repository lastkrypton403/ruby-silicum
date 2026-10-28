#!/usr/bin/env python3
"""Replace the kernel inside an Android boot image (header v0-v4), keep the rest.

  swap_kernel.py --info boot.img
  swap_kernel.py boot.img Image -o test-boot.img

Handles MediaTek's 512-byte kernel wrapper (magic 0x58881688) by copying the
old wrapper and patching its size field. Does not touch ramdisk, dtb, cmdline,
or any AVB footer (an AVB footer after the old end of file is carried over but
will no longer sit at the end; fine when verification is bypassed).
"""
import argparse, gzip, struct, sys

MTK_MAGIC = b"\x88\x16\x88\x58"

def pad(n, a): return (n + a - 1) // a * a

def parse(img):
    if img[:8] != b"ANDROID!":
        sys.exit("not an Android boot image (missing ANDROID! magic)")
    ver = struct.unpack_from("<I", img, 40)[0]
    ksize = struct.unpack_from("<I", img, 8)[0]
    page = 4096 if ver >= 3 else struct.unpack_from("<I", img, 36)[0]
    return ver, ksize, page

def kind(k):
    if k[:4] == MTK_MAGIC: return "MediaTek-wrapped"
    if k[:2] == b"\x1f\x8b": return "gzip (Image.gz)"
    if k[0x38:0x3c] == b"ARM\x64": return "raw arm64 Image"
    return "unknown"

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("boot"); ap.add_argument("kernel", nargs="?")
    ap.add_argument("-o", "--out", default="test-boot.img")
    ap.add_argument("--info", action="store_true")
    ap.add_argument("--gzip", action="store_true", help="gzip the kernel like a stock Image.gz")
    a = ap.parse_args()
    img = open(a.boot, "rb").read()
    ver, ksize, page = parse(img)
    kstart = page
    old = img[kstart:kstart + ksize]
    print(f"header_version={ver} page_size={page} kernel_size={ksize} kernel={kind(old)}")
    if ver <= 2:
        ka, rs, ra, ss, sa, ta = struct.unpack_from("<IIIIII", img, 12)
        print(f"kernel_addr=0x{ka:x} ramdisk_addr=0x{ra:x} tags_addr=0x{ta:x}")
    if a.info: return
    if not a.kernel: sys.exit("need a kernel file (or use --info)")
    new = open(a.kernel, "rb").read()
    if new[0x38:0x3c] != b"ARM\x64":
        print("warning: kernel file has no arm64 Image magic", file=sys.stderr)
    if a.gzip:
        new = gzip.compress(new, 9, mtime=0)
        print(f"gzipped kernel: {len(new)} bytes")
    payload = new
    if old[:4] == MTK_MAGIC:
        hdr = bytearray(old[:512])
        struct.pack_into("<I", hdr, 4, len(new))
        payload = bytes(hdr) + new
        print("re-wrapped with MediaTek 512-byte header")
    old_pad = pad(ksize, page)
    new_pad = pad(len(payload), page)
    delta = new_pad - old_pad
    out = bytearray(img[:kstart]) + payload + b"\0" * (new_pad - len(payload)) + img[kstart + old_pad:]
    struct.pack_into("<I", out, 8, len(payload))
    if ver in (1, 2):
        dsz = struct.unpack_from("<I", out, 1632)[0]
        if dsz:
            off = struct.unpack_from("<Q", out, 1636)[0]
            struct.pack_into("<Q", out, 1636, off + delta)
    open(a.out, "wb").write(out)
    print(f"wrote {a.out}: {len(out)} bytes (kernel {len(payload)} bytes, shifted rest by {delta})")

if __name__ == "__main__":
    main()
