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
    with open(os.path.join(ROOT, "VERSION"), encoding="utf-8") as h:
        version = h.read().strip().strip("vV")
    check(os.path.isfile(os.path.join(ROOT, "files", "launch.sh")), "launch.sh ton tai")
    check(os.path.isfile(os.path.join(ROOT, "files", "config.json")), "config.json ton tai")
    check(os.path.isfile(os.path.join(ROOT, "files", "icon.png")), "icon.png ton tai")
    shipped = os.path.join(ROOT, "files", "VERSION")
    shipped_version = open(shipped, encoding="utf-8").read().strip() if os.path.isfile(shipped) else ""
    check(shipped_version == version,
          "files/VERSION khop VERSION (%r)" % shipped_version)
    check(os.path.isfile(os.path.join(ROOT, "files", "assets", "fallback.ttf")),
          "assets/fallback.ttf ton tai")
    check(os.path.isfile(BIN), "files/bin/trimui-terminal ton tai")
    if os.path.isfile(BIN):
        with open(BIN, "rb") as h:
            magic = h.read(20)
        check(magic[:4] == b"\x7fELF", "binary la ELF")
        check(int.from_bytes(magic[18:20], "little") == 0xB7, "binary la AArch64 (EM=183)")
        check(os.path.getsize(BIN) > 20000, "binary co kich thuoc hop ly")
        # Intro NLK phai nam trong binary: firmware nay khong co fim/fbv/fbi
        # nen intro ve trong app la cach duy nhat that su hien duoc.
        with open(BIN, "rb") as h:
            blob = h.read()
        for marker in (b"intro: NLK giant=", b"intro: NLK xong", b"intro.off", b".no-intro", b"-nointro"):
            check(marker in blob, "binary chua intro NLK (%s)" % marker.decode("ascii"))
        check(version.encode("ascii") in blob,
              "binary duoc bien dich voi VERSION=%s" % version)
        s1 = open(os.path.join(ROOT, "files", "launch.sh"), encoding="utf-8", errors="replace").read()
        check("stay_alive" in s1, "launch.sh giu may tranh deep-suspend")
        check("fim -" not in s1 and "fbv " not in s1 and "fbi " not in s1,
              "launch.sh khong con dua intro vao fim/fbv/fbi (firmware khong co)")
        cfg = json.load(open(os.path.join(ROOT, "files", "config.json"), encoding="utf-8"))
        check(cfg.get("launch") == "launch.sh", "config.json tro dung launch.sh")
    with open(os.path.join(ROOT, "files", "ota-update.sh"), encoding="utf-8", errors="replace") as h:
        ota = h.read()
    check("$OTA_STATUS.tmp" in ota and 'mv "$OTA_STATUS.tmp" "$OTA_STATUS"' in ota,
          "ota ghi .ota-status atomic (tranh badge dinh)")
    check('> "$APP/VERSION"' in ota, "OTA ghi lai files/VERSION cho binary doc")
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
