#!/bin/sh
# Trimui-Terminal launcher cho TrimUI Brick Pro / Smart Pro S.
case "$0" in
  */*)
    cd "$(dirname "$0")" || exit 1
    ;;
esac
APP="$(pwd)"
SDCARD_PATH="${SDCARD_PATH:-/mnt/SDCARD}"
export SDCARD_PATH
export LD_LIBRARY_PATH="$APP/libs:/usr/trimui/lib:/usr/lib64:/usr/lib:/lib:$LD_LIBRARY_PATH"
BIN="$APP/bin/trimui-terminal"
ERRLOG="$APP/Terminal-loi.txt"
if [ ! -x "$BIN" ]; then
  echo "Không thấy trimui-terminal hoặc thiếu quyền chạy" > "$ERRLOG" 2>/dev/null
  exit 1
fi
touch /tmp/stay_alive 2>/dev/null
rm -f "$APP/.ota-status" 2>/dev/null
if [ -x "$APP/ota-update.sh" ] && [ "$TERMINAL_NO_OTA" != "1" ]; then
  # Chay nen: app mo ngay lap tuc, app tu hien "Dang cap nhat..." + bao khi xong.
  if command -v timeout >/dev/null 2>&1; then
    timeout 90 sh "$APP/ota-update.sh" --apply >> "$APP/Terminal-ota.log" 2>&1 &
  else
    sh "$APP/ota-update.sh" --apply >> "$APP/Terminal-ota.log" 2>&1 &
  fi
fi
# NLK intro splash: anh tinh ~1s, trang tri, KHONG bao gio duoc lam hong boot.
# Tat bang: TERMINAL_NO_INTRO=1, hoac file $APP/intro.off, hoac config.json "intro": false.
if [ "$TERMINAL_NO_INTRO" != "1" ]; then
  _INTRO_PNG="$APP/assets/intro.png"
  _INTRO_OFF=0
  if [ -f "$APP/intro.off" ] || [ -f "$APP/.no-intro" ]; then
    _INTRO_OFF=1
  elif command -v grep >/dev/null 2>&1; then
    if grep -q '"intro"[[:space:]]*:[[:space:]]*false' "$APP/config.json" 2>/dev/null; then
      _INTRO_OFF=1
    fi
  fi
  if [ "$_INTRO_OFF" != "1" ] && [ -f "$_INTRO_PNG" ]; then
    if command -v fim >/dev/null 2>&1; then
      fim -q -a "$_INTRO_PNG" >/dev/null 2>&1 &
      _INTRO_PID=$!
      sleep 1 2>/dev/null || true
      kill "$_INTRO_PID" 2>/dev/null || true
    elif command -v fbv >/dev/null 2>&1; then
      # fbv tuy build co/khong co co -a: co thi dung (auto-scale), khong thi chup nguyen.
      _FBV_A=""
      if fbv --help 2>/dev/null | grep -q -- "-a"; then
        _FBV_A="-a"
      fi
      # shellcheck disable=SC2086
      fbv $_FBV_A "$_INTRO_PNG" >/dev/null 2>&1 &
      _INTRO_PID=$!
      sleep 1 2>/dev/null || true
      kill "$_INTRO_PID" 2>/dev/null || true
      unset _FBV_A
    elif command -v fbi >/dev/null 2>&1; then
      fbi -a -T 1 --noverbose "$_INTRO_PNG" >/dev/null 2>&1 &
      _INTRO_PID=$!
      sleep 1 2>/dev/null || true
      kill "$_INTRO_PID" 2>/dev/null || true
    fi
    unset _INTRO_PID
  fi
  unset _INTRO_PNG _INTRO_OFF
fi
"$BIN" -scale 1 -fontsize 16 "$@" 2>> "$ERRLOG"
CODE=$?
rm -f /tmp/stay_alive 2>/dev/null
exit $CODE
