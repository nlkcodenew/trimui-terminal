#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Tao GitHub Release vX.Y.Z cho trimui-terminal + upload ZIP/sha256/manifest."""
import json
import os
import subprocess
import sys
import urllib.request
import urllib.error
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
REPO = "nlkcodenew/trimui-terminal"
API = "https://api.github.com"
UPLOAD = "https://uploads.github.com"

def version():
    with open(os.path.join(ROOT, "VERSION"), encoding="utf-8") as f:
        return f.read().strip().strip("vV")

def token():
    p = subprocess.run(["git", "credential", "fill"], input="url=https://github.com\n\n",
                       capture_output=True, text=True, cwd=ROOT)
    for line in (p.stdout or "").splitlines():
        if line.startswith("password="):
            return line[len("password="):].strip()
    return ""

def api(method, path, tok, data=None, ctype="application/json"):
    url = API + path if path.startswith("/") else path
    body = json.dumps(data).encode() if isinstance(data, dict) else data
    req = urllib.request.Request(url, data=body, method=method,
                                 headers={"Authorization": "Bearer " + tok,
                                          "Accept": "application/vnd.github+json",
                                          "Content-Type": ctype,
                                          "User-Agent": "trimui-terminal-release"})
    try:
        with urllib.request.urlopen(req, timeout=60) as r:
            raw = r.read()
            return r.status, json.loads(raw.decode() or "{}")
    except urllib.error.HTTPError as e:
        raw = e.read().decode(errors="replace")
        try:
            return e.code, json.loads(raw or "{}")
        except Exception:
            return e.code, {"raw": raw[:500]}

def upload_asset(tok, upload_url, fpath, ctype):
    name = os.path.basename(fpath)
    url = upload_url.split("{")[0] + "?name=" + urllib.request.quote(name)
    with open(fpath, "rb") as h:
        data = h.read()
    req = urllib.request.Request(url, data=data, method="POST",
                                 headers={"Authorization": "Bearer " + tok,
                                          "Content-Type": ctype,
                                          "Content-Length": str(len(data)),
                                          "User-Agent": "trimui-terminal-release"})
    try:
        with urllib.request.urlopen(req, timeout=120) as r:
            return r.status, json.loads(r.read().decode() or "{}")
    except urllib.error.HTTPError as e:
        return e.code, {"raw": e.read().decode(errors="replace")[:500]}

def main():
    ver = version()
    tag = "v" + ver
    tok = token()
    if not tok:
        print("FAIL: khong lay duoc github token tu credential manager")
        return 1
    dist = os.path.join(ROOT, "dist")
    zname = "trimui-terminal-%s.zip" % tag
    zpath = os.path.join(dist, zname)
    spath = zpath + ".sha256"
    mpath = os.path.join(ROOT, "manifest.json")
    for p in (zpath, spath, mpath):
        if not os.path.isfile(p):
            print("FAIL: thieu file " + p)
            return 1
    notes = "Terminal SDL2 cho Brick Pro + BT survey. Giai nen vao goc the nho de co Apps/TrimuiTerminal/launch.sh. Chi tiet: CAI_DAT.md."
    try:
        with open(os.path.join(ROOT, "CHANGELOG.md"), encoding="utf-8") as f:
            notes = f.read().strip()[:3000]
    except OSError:
        pass
    st, rel = api("GET", "/repos/%s/releases/tags/%s" % (REPO, tag), tok)
    if st == 200:
        print("release da ton tai id=%s -> xoa asset cu + upload lai" % rel.get("id"))
        rid = rel["id"]
        for a in rel.get("assets", []):
            api("DELETE", "/repos/%s/releases/assets/%s" % (REPO, a["id"]), tok)
    else:
        st, rel = api("POST", "/repos/%s/releases" % REPO, tok,
                      {"tag_name": tag, "name": "trimui-terminal " + tag,
                       "body": notes, "draft": False, "prerelease": False})
        if st not in (200, 201):
            print("FAIL tao release: %s %s" % (st, rel))
            return 1
        rid = rel["id"]
        print("created release id=%s" % rid)
    up = rel["upload_url"]
    # refetch upload_url neu release cu
    if "upload_url" not in rel or "{" not in str(up):
        st2, rel2 = api("GET", "/repos/%s/releases/tags/%s" % (REPO, tag), tok)
        up = rel2.get("upload_url", up)
    ok = True
    for fpath, ctype in ((zpath, "application/zip"), (spath, "text/plain"), (mpath, "application/json")):
        st, r = upload_asset(tok, up, fpath, ctype)
        print(("PASS " if st in (200, 201) else "FAIL ") + os.path.basename(fpath) + " -> " + str(st))
        ok = ok and st in (200, 201)
    print("RELEASE OK: https://github.com/%s/releases/tag/%s" % (REPO, tag) if ok else "RELEASE UPLOAD LOI")
    return 0 if ok else 1

if __name__ == "__main__":
    sys.exit(main())
