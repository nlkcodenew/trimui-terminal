#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Gate truoc khi release: binary AArch64, cau truc ZIP, khong dinh secret."""
import json
import os
import sys
import zipfile
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BIN = os.path.join(ROOT, "files", "bin", "trimui-terminal")
DIST = os.path.join(ROOT, "dist")
FAIL = []

def check(cond, msg):
    print(("PASS " if cond else "FAIL ") + msg)
    if not cond:
        FAIL.append(msg)

def main():
    check(os.path.isfile(os.path.join(ROOT, "files", "launch.sh")), "launch.sh ton tai")
    check(os.path.isfile(os.path.join(ROOT, "files", "config.json")), "config.json ton tai")
    check(os.path.isfile(os.path.join(ROOT, "files", "icon.png")), "icon.png ton tai")
    check(os.path.isfile(BIN), "files/bin/trimui-terminal ton tai")
    if os.path.isfile(BIN):
        with open(BIN, "rb") as h:
            magic = h.read(20)
        check(magic[:4] == b"\x7fELF", "binary la ELF")
        check(int.from_bytes(magic[18:20], "little") == 0xB7, "binary la AArch64 (EM=183)")
        check(os.path.getsize(BIN) > 20000, "binary co kich thuoc hop ly")
        s1 = open(os.path.join(ROOT, "files", "launch.sh"), encoding="utf-8", errors="replace").read()
        check("stay_alive" in s1, "launch.sh giu may tranh deep-suspend")
        cfg = json.load(open(os.path.join(ROOT, "files", "config.json"), encoding="utf-8"))
        check(cfg.get("launch") == "launch.sh", "config.json tro dung launch.sh")
    zips = [f for f in os.listdir(DIST) if f.endswith(".zip")] if os.path.isdir(DIST) else []
    if zips:
        zp = os.path.join(DIST, sorted(zips)[-1])
        names = zipfile.ZipFile(zp).namelist()
        check("Apps/TrimuiTerminal/launch.sh" in names, "ZIP co Apps/TrimuiTerminal/launch.sh")
        check("Apps/TrimuiTerminal/bin/trimui-terminal" in names, "ZIP co binary")
        check(not any("secrets" in n for n in names), "ZIP khong chua secret")
    else:
        print("SKIP kiem tra ZIP (chua chay make_release.py)")
    if FAIL:
        print("FAILED: %d" % len(FAIL))
        return 1
    print("ALL OK")
    return 0

if __name__ == "__main__":
    sys.exit(main())
