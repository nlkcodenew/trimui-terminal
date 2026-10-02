#!/bin/sh
# OTA cho trimui-terminal: kiem manifest GitHub, tai file lech hash, staging + apply.
# Dung: sh ota-update.sh | sh ota-update.sh --apply | sh ota-update.sh --check
# --apply: tu dong (launch.sh goi moi lan mo app, khong hoi). That bai/offline -> bo qua.
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
# File VERSION trong app co the bi thieu/rot neu cai dat lai ZIP cung version
# (OTA chay o background nen app dang mo). Version hien thi cua binary doc
# chinh file nay, nen phai chuan hoa lai cho binary moi doc dung.
if [ ! -f "$APP/VERSION" ] || [ "$(cat "$APP/VERSION" 2>/dev/null | tr -d ' \r\n')" != "$CUR" ]; then
  printf "%s" "$CUR" > "$APP/VERSION" 2>/dev/null || true
fi
REPO="${TERMINAL_REPO:-nlkcodenew/trimui-terminal}"
CHANNEL="${TERMINAL_CHANNEL:-latest}"
CA="$APP/certs/cacert.pem"
LOG="$APP/Terminal-ota.log"
TMPD="$APP/.update_staging"
say() { echo "[ota] $*"; echo "$(date '+%Y-%m-%d %H:%M:%S' 2>/dev/null) $*" >> "$LOG" 2>/dev/null; }
# File trang thai de app hien thong bao (checking|downloading <ver>|done <ver>|failed).
# Ghi atomic (ghi .tmp roi doi ten): app doc file 1 lan/giay, tranh doc ban va
# lam badge "Dang kiem tra..." dinh o goc phai khi app vua mo.
OTA_STATUS="$APP/.ota-status"
ota_status() {
  printf "%s" "$*" > "$OTA_STATUS.tmp" 2>/dev/null || true
  mv "$OTA_STATUS.tmp" "$OTA_STATUS" 2>/dev/null || rm -f "$OTA_STATUS.tmp" 2>/dev/null || true
}
ota_clear() { rm -f "$OTA_STATUS" "$OTA_STATUS.tmp" 2>/dev/null || true; }
# So sanh version dang x.y.z (khong dung sort -V vi BusyBox co the thieu).
ver_newer() {
  a="$1"; b="$2"
  [ "$a" = "$b" ] && return 1
  i=1
  while :; do
    pa="$(echo "$a" | cut -d. -f$i 2>/dev/null)"; pb="$(echo "$b" | cut -d. -f$i 2>/dev/null)"
    [ -z "$pa$pb" ] && return 1
    pa="${pa%%[!0-9]*}"; pb="${pb%%[!0-9]*}"
    [ -z "$pa" ] && pa=0; [ -z "$pb" ] && pb=0
    # ep kieu so (loai so 0 o dau de tranh loi octal)
    pa="$(echo "$pa" | sed 's/^0*//')"; pb="$(echo "$pb" | sed 's/^0*//')"
    [ -z "$pa" ] && pa=0; [ -z "$pb" ] && pb=0
    if [ "$pa" -gt "$pb" ] 2>/dev/null; then return 0; fi
    if [ "$pa" -lt "$pb" ] 2>/dev/null; then return 1; fi
    i=$((i + 1))
    [ "$i" -gt 8 ] && return 1
  done
}
fetch() {
  url="$1"; out="$2"
  if command -v curl >/dev/null 2>&1; then
    if [ -f "$CA" ]; then curl -fsSL --connect-timeout 5 --max-time 10 --retry 0 --cacert "$CA" -o "$out" "$url" 2>/dev/null && return 0; fi
    curl -fsSL --connect-timeout 5 --max-time 10 --retry 0 -o "$out" "$url" 2>/dev/null && return 0
  fi
  if command -v wget >/dev/null 2>&1; then
    if [ -f "$CA" ]; then wget -q --timeout=10 --tries=1 --ca-certificate="$CA" -O "$out" "$url" 2>/dev/null && return 0; fi
    wget -q --timeout=10 --tries=1 -O "$out" "$url" 2>/dev/null && return 0
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
    data = urllib.request.urlopen(req, timeout=10, context=ctx).read()
    open(out, "wb").write(data)
except Exception as e:
    sys.stderr.write(str(e))
    sys.exit(1)
PYEOF
  fi
  return 1
}
# Tai + verify sha256 bang shell thuan (khi may khong co python3).
# Doc manifest.json bang awk (manifest do make_release.py sinh, format on dinh).
shell_download_files() {
  LIST="$TMPD.files.list"
  awk '
    /"path"/ { p=$0; sub(/.*"path"[[:space:]]*:[[:space:]]*"/, "", p); sub(/".*/, "", p) }
    /"sha256"/ { s=$0; sub(/.*"sha256"[[:space:]]*:[[:space:]]*"/, "", s); sub(/".*/, "", s); if (p != "" && s != "") print p "|" s; p=""; s="" }
  ' "$MANIFEST_JSON" > "$LIST" 2>/dev/null
  [ -s "$LIST" ] || { say "Không đọc được danh sách file"; return 1; }
  command -v sha256sum >/dev/null 2>&1 || { say "Thiếu sha256sum để kiểm tra"; return 1; }
  while IFS= read -r e; do
    rel="${e%%|*}"; want="${e##*|}"
    [ -n "$rel" ] && [ -n "$want" ] || return 1
    got=""
    for b in $BASES; do
      if fetch "$b/$rel" "$TMPD.dl.tmp"; then got="$TMPD.dl.tmp"; break; fi
    done
    [ -n "$got" ] || { say "Không tải được: $rel"; return 1; }
    have="$(sha256sum "$got" 2>/dev/null | cut -d' ' -f1)"
    if [ "$have" != "$want" ]; then say "Sai mã kiểm tra: $rel"; rm -f "$got"; return 1; fi
    dst="$TMPD/$rel"
    mkdir -p "$(dirname "$dst")" 2>/dev/null
    mv "$got" "$dst" || return 1
    say "ok $rel"
  done < "$LIST"
  n="$(wc -l < "$LIST" 2>/dev/null | tr -d ' ')"
  say "Đã tải xong $n file"
  return 0
}
MANIFEST_JSON="$TMPD.manifest.json"
rm -rf "$TMPD" "$MANIFEST_JSON"
mkdir -p "$TMPD" 2>/dev/null || { say "Không tạo được thư mục tạm"; exit 1; }
if [ "$CHANNEL" = "latest" ]; then
  MURL="https://raw.githubusercontent.com/$REPO/main/manifest.json"
else
  MURL="https://raw.githubusercontent.com/$REPO/$CHANNEL/manifest.json"
fi
say "local=$CUR repo=$REPO channel=$CHANNEL"
ota_status "checking"
got_manifest=0
for try in 1 2; do
  if fetch "$MURL" "$MANIFEST_JSON" || fetch "https://cdn.jsdelivr.net/gh/$REPO@main/manifest.json" "$MANIFEST_JSON"; then
    got_manifest=1; break
  fi
  [ "$try" = "1" ] && sleep 3
done
[ "$got_manifest" = "1" ] || { say "Không tải được danh mục bản mới"; ota_status "failed"; exit 1; }
# Parse version (dung sys.argv, khong loi quote). Fallback grep neu thieu python3.
REM=""
if command -v python3 >/dev/null 2>&1; then
  REM="$(python3 -c 'import json,sys;print(json.load(open(sys.argv[1], encoding="utf-8")).get("version",""))' "$MANIFEST_JSON" 2>/dev/null)"
fi
[ -n "$REM" ] || REM="$(grep -o '"version"[[:space:]]*:[[:space:]]*"[^"]*"' "$MANIFEST_JSON" 2>/dev/null | head -n 1 | sed 's/.*"\([^"]*\)"$/\1/')"
[ -n "$REM" ] || { say "Danh mục thiếu số phiên bản"; ota_status "failed"; exit 1; }
say "remote=$REM"
if ! ver_newer "$REM" "$CUR"; then
  say "Đã là bản mới nhất ($CUR)"
  # Ghi lai VERSION de binary doc dung (binary cu doc file nay, fallback -DVERSION).
  printf "%s" "$CUR" > "$APP/VERSION" 2>/dev/null || true
  rm -rf "$TMPD" "$MANIFEST_JSON"
  ota_clear
  exit 2
fi
if [ "${1:-}" = "--check" ]; then
  say "Có bản mới: $REM (đang dùng $CUR). Chạy sh ota-update.sh --apply để cập nhật."
  rm -rf "$TMPD" "$MANIFEST_JSON"
  ota_clear
  exit 10
fi
BASES="https://raw.githubusercontent.com/$REPO/v$REM/files https://raw.githubusercontent.com/$REPO/main/files https://cdn.jsdelivr.net/gh/$REPO@v$REM/files"
if [ "${1:-}" != "--apply" ]; then
  printf "Có bản mới %s (hiện tại %s). Cập nhật? [y/N] " "$REM" "$CUR"
  read -r ans
  case "$ans" in y|Y|yes|YES) ;; *) say "Đã hủy"; rm -rf "$TMPD" "$MANIFEST_JSON"; ota_clear; exit 3;; esac
fi
say "Đang tải $REM ..."
ota_status "downloading $REM"
DL_OK=0
if command -v python3 >/dev/null 2>&1; then
python3 - "$MANIFEST_JSON" "$TMPD" "$BASES" "$CA" <<PYEOF && DL_OK=1
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
    with urllib.request.urlopen(req, timeout=15, context=ctx) as r:
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
fi
if [ "$DL_OK" != "1" ]; then
  say "Thử tải bằng shell (máy không có python3)..."
  if shell_download_files; then DL_OK=1; fi
fi
if [ "$DL_OK" != "1" ]; then say "Tải file thất bại"; rm -rf "$TMPD" "$MANIFEST_JSON"; ota_status "failed"; exit 1; fi
# Apply: khong dung pipe-while (exit trong subshell khong lan ra ngoai).
LIST="$TMPD.apply.list"
(cd "$TMPD" && find . -type f -print > "$LIST") || { say "Cài đặt thất bại"; rm -rf "$TMPD" "$MANIFEST_JSON"; ota_status "failed"; exit 1; }
APPLY_FAIL=0
while IFS= read -r f; do
  [ -n "$f" ] || continue
  rel="${f#./}"
  case "$rel" in "$LIST"*|*.apply.list) continue;; esac
  [ -f "$TMPD/$rel" ] || continue
  dst="$APP/$rel"
  mkdir -p "$(dirname "$dst")" 2>/dev/null
  tmp="$dst.ota-new"
  if ! cp "$TMPD/$rel" "$tmp" 2>/dev/null; then say "Chép thất bại: $rel"; APPLY_FAIL=1; break; fi
  case "$rel" in *.sh|bin/*) chmod +x "$tmp" 2>/dev/null;; esac
  if ! mv "$tmp" "$dst" 2>/dev/null; then say "Cài đặt thất bại: $rel"; APPLY_FAIL=1; break; fi
done < "$LIST"
if [ "$APPLY_FAIL" != "0" ]; then rm -rf "$TMPD" "$MANIFEST_JSON"; ota_status "failed"; exit 1; fi
cd "$APP" || exit 1
printf "%s" "$REM" | tr -d " \r\n" > "$APP/VERSION" 2>/dev/null
say "Cập nhật xong $CUR -> $REM. Thoát app và mở lại."
ota_status "done $REM"
rm -rf "$TMPD" "$MANIFEST_JSON"
exit 0
