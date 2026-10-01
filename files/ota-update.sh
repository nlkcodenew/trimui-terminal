#!/bin/sh
# OTA cho trimui-terminal: kiem manifest GitHub, tai file lech hash, staging + apply.
# Dung: sh ota-update.sh | sh ota-update.sh --apply | sh ota-update.sh --check
case "$0" in
  */*)
    cd "$(dirname "$0")" || exit 1
    ;;
esac
APP="$(pwd)"
VERSION_FILE="$APP/VERSION"
[ -f "$VERSION_FILE" ] || VERSION_FILE="$APP/../VERSION"
CUR="$(cat "$VERSION_FILE" 2>/dev/null | tr -d ' \r\n')"
[ -n "$CUR" ] || CUR="0.0.0"
REPO="${TERMINAL_REPO:-nlkcodenew/trimui-terminal}"
CHANNEL="${TERMINAL_CHANNEL:-latest}"
CA="$APP/certs/cacert.pem"
LOG="$APP/Terminal-ota.log"
TMPD="$APP/.update_staging"
say() { echo "[ota] $*"; echo "$(date '+%Y-%m-%d %H:%M:%S' 2>/dev/null) $*" >> "$LOG" 2>/dev/null; }
ver_newer() {
  a="$1"; b="$2"
  [ "$a" = "$b" ] && return 1
  first="$(printf '%s\n%s\n' "$a" "$b" | sort -V | head -n 1)"
  [ "$first" = "$b" ]
}
fetch() {
  url="$1"; out="$2"
  if command -v curl >/dev/null 2>&1; then
    if [ -f "$CA" ]; then curl -fsSL --cacert "$CA" -o "$out" "$url" 2>/dev/null && return 0; fi
    curl -fsSL -o "$out" "$url" 2>/dev/null && return 0
  fi
  if command -v wget >/dev/null 2>&1; then
    if [ -f "$CA" ]; then wget -q --ca-certificate="$CA" -O "$out" "$url" 2>/dev/null && return 0; fi
    wget -q -O "$out" "$url" 2>/dev/null && return 0
  fi
  if command -v python3 >/dev/null 2>&1; then
    python3 - "$url" "$out" "$CA" <<PYEOF 2>/dev/null && return 0
import ssl, sys, urllib.request
url, out, ca = sys.argv[1], sys.argv[2], sys.argv[3]
try:
    ctx = ssl.create_default_context()
    try:
        ctx.load_verify_locations(cafile=ca)
    except Exception:
        pass
    req = urllib.request.Request(url, headers={"User-Agent": "trimui-terminal-ota"})
    data = urllib.request.urlopen(req, timeout=20, context=ctx).read()
    open(out, "wb").write(data)
except Exception as e:
    sys.stderr.write(str(e))
    sys.exit(1)
PYEOF
  fi
  return 1
}
MANIFEST_JSON="$TMPD.manifest.json"
rm -rf "$TMPD" "$MANIFEST_JSON"
mkdir -p "$TMPD" 2>/dev/null || { say "khong tao duoc staging"; exit 1; }
if [ "$CHANNEL" = "latest" ]; then
  MURL="https://raw.githubusercontent.com/$REPO/main/manifest.json"
else
  MURL="https://raw.githubusercontent.com/$REPO/$CHANNEL/manifest.json"
fi
say "local=$CUR repo=$REPO channel=$CHANNEL"
fetch "$MURL" "$MANIFEST_JSON" || fetch "https://cdn.jsdelivr.net/gh/$REPO@main/manifest.json" "$MANIFEST_JSON" || { say "khong tai duoc manifest"; exit 1; }
REMF="$(python3 -c "import json;print(json.load(open("$MANIFEST_JSON")).get('version',''))" 2>/dev/null)"
REM="$REMF"
[ -n "$REM" ] || { say "manifest khong co version"; exit 1; }
say "remote=$REM"
if ! ver_newer "$REM" "$CUR"; then
  say "da la ban moi nhat ($CUR)"
  rm -rf "$TMPD" "$MANIFEST_JSON"
  exit 2
fi
if [ "$1" = "--check" ]; then
  say "co ban moi: $REM (local $CUR). Chay sh ota-update.sh --apply de cap nhat."
  rm -rf "$TMPD" "$MANIFEST_JSON"
  exit 10
fi
BASES="https://raw.githubusercontent.com/$REPO/v$REM/files https://raw.githubusercontent.com/$REPO/main/files https://cdn.jsdelivr.net/gh/$REPO@v$REM/files"
if [ "$1" != "--apply" ]; then
  printf "Co ban moi %s (hien tai %s). Cap nhat? [y/N] " "$REM" "$CUR"
  read -r ans
  case "$ans" in y|Y|yes|YES) ;; *) say "huy bo"; rm -rf "$TMPD" "$MANIFEST_JSON"; exit 3;; esac
fi
say "tai $REM ..."
python3 - "$MANIFEST_JSON" "$TMPD" "$BASES" "$CA" <<PYEOF || { say "tai file that bai"; rm -rf "$TMPD" "$MANIFEST_JSON"; exit 1; }
import hashlib, os, ssl, sys, json, urllib.request
mp, tmpd, bases, ca = sys.argv[1], sys.argv[2], sys.argv[3].split(), sys.argv[4]
man = json.load(open(mp, encoding="utf-8"))
files = man.get("files", [])
def sha(b): return hashlib.sha256(b).hexdigest()
try:
    ctx = ssl.create_default_context()
    try: ctx.load_verify_locations(cafile=ca)
    except Exception: pass
except Exception:
    ctx = None
def get(url):
    req = urllib.request.Request(url, headers={"User-Agent": "trimui-terminal-ota"})
    with urllib.request.urlopen(req, timeout=30, context=ctx) as r:
        return r.read()
fails = []
for e in files:
    rel = e["path"].replace("/", os.sep)
    data = None
    for b in bases:
        try:
            data = get(b.rstrip("/") + "/" + e["path"])
            break
        except Exception:
            continue
    if data is None:
        print("FAIL fetch " + e["path"]); fails.append(rel); continue
    if sha(data) != e["sha256"]:
        print("FAIL hash " + e["path"]); fails.append(rel); continue
    dst = os.path.join(tmpd, rel)
    dd = os.path.dirname(dst)
    if dd: os.makedirs(dd, exist_ok=True)
    open(dst, "wb").write(data)
    print("ok " + e["path"])
if fails:
    print("FAILED %d file(s)" % len(fails)); sys.exit(1)
print("STAGED %d file(s)" % len(files))
PYEOF
cd "$TMPD" || exit 1
find . -type f | while IFS= read -r rel; do
  rel="${rel#./}"
  dst="$APP/$rel"
  mkdir -p "$(dirname "$dst")" 2>/dev/null
  tmp="$dst.ota-new"
  cp "$TMPD/$rel" "$tmp" 2>/dev/null || { say "copy that bai: $rel"; exit 1; }
  case "$rel" in *.sh|bin/*) chmod +x "$tmp" 2>/dev/null;; esac
  mv "$tmp" "$dst" || { say "apply that bai: $rel"; exit 1; }
done || { say "apply that bai"; rm -rf "$TMPD" "$MANIFEST_JSON"; exit 1; }
cd "$APP" || exit 1
printf "%s" "$REM" | tr -d " \r\n" > "$APP/VERSION" 2>/dev/null
say "cap nhat xong $CUR -> $REM. Thoat app va mo lai."
rm -rf "$TMPD" "$MANIFEST_JSON"
exit 0
