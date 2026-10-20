#!/bin/sh
# run-in-terminal.sh — helper của Trimui-Terminal.
#
# Đảm bảo một lệnh chạy BÊN TRONG cửa sổ Trimui-Terminal:
#  - Nếu tiến trình cha đã là trimui-terminal: chạy lệnh trực tiếp (tránh đệ quy).
#  - Ngược lại: mở một cửa sổ terminal mới chạy lệnh, rồi thoát.
#
# Cách dùng (trong script của app khác):
#   /mnt/SDCARD/Apps/TrimuiTerminal/run-in-terminal.sh [-s 24] -- "/path/script.sh"
#
# Pattern học từ app Terminal tham khảo (launch_TermSP.sh -s 24 -e "$0"):
# script tự mở lại chính nó trong terminal khi chưa chạy trong terminal.
SIZE=16
while [ $# -gt 0 ]; do
  case "$1" in
    -s)
      SIZE="$2"
      shift 2
      ;;
    --)
      shift
      break
      ;;
    *) break ;;
  esac
done
if [ $# -eq 0 ]; then
  echo "Usage: run-in-terminal.sh [-s SIZE] -- \"COMMAND\"" >&2
  exit 1
fi
CMD="$*"
HERE="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
# Đang ở trong terminal rồi? (tiến trình cha là trimui-terminal)
PPROC="$(cat "/proc/$PPID/comm" 2>/dev/null)"
case "$PPROC" in
  *trimui-terminal*)
    exec sh -c "$CMD"
    ;;
esac
exec "$HERE/bin/trimui-terminal" -fontsize "$SIZE" -r "$CMD"
