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
"$BIN" -scale 1 -fontsize 16 "$@" 2>> "$ERRLOG"
CODE=$?
rm -f /tmp/stay_alive 2>/dev/null
exit $CODE
