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
touch /var/trimui_inputd/sticks_disabled 2>/dev/null
rm -f "$APP/.ota-status" 2>/dev/null
OTA_PID=""
if [ -x "$APP/ota-update.sh" ] && [ "$TERMINAL_NO_OTA" != "1" ]; then
  # Chay nen: app mo ngay lap tuc, app tu hien "Dang cap nhat..." + bao khi xong.
  # </dev/null + redirect log: tach FD khoi pipe cua launcher (Knulli/
  # emulatorlauncher cho pipe EOF; job nen giu pipe se lam ES ket khi app thoat).
  if command -v timeout >/dev/null 2>&1; then
    timeout 90 sh "$APP/ota-update.sh" --apply </dev/null >> "$APP/Terminal-ota.log" 2>&1 &
  else
    sh "$APP/ota-update.sh" --apply </dev/null >> "$APP/Terminal-ota.log" 2>&1 &
  fi
  OTA_PID=$!
fi
# NLK intro (logo khoi dong) do CHINH BINARY ve bang SDL_ttf ngay khi app mo,
# khong con dua vao fim/fbv/fbi: firmware nay khong co trinh xem framebuffer nao
# nen cach do khong bao gio hien du co anh intro.png.
# Tat intro: TERMINAL_NO_INTRO=1, file $APP/intro.off, $APP/.no-intro,
# hoac config.json "intro": false (binary doc trong intro_disabled()).
if [ "$TERMINAL_NO_INTRO" = "1" ]; then
  export TERMINAL_NO_INTRO
fi
"$BIN" -scale 1 -fontsize 16 "$@" 2>> "$ERRLOG"
CODE=$?
# Don job OTA nen + cac con (curl...): Knulli khong co pkill -> quet /proc theo PPID.
if [ -n "$OTA_PID" ]; then
  kill "$OTA_PID" 2>/dev/null
  for p in /proc/[0-9]*; do
    [ -r "$p/stat" ] || continue
    pp=$(awk '{print $4}' "$p/stat" 2>/dev/null)
    [ "$pp" = "$OTA_PID" ] && kill "${p#/proc/}" 2>/dev/null
  done
fi
rm -f /tmp/stay_alive /var/trimui_inputd/sticks_disabled 2>/dev/null
exit $CODE
