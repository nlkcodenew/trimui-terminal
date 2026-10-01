#!/bin/sh
# BT survey: thu thap Bluetooth de danh gia tay cam BT.
# Chay trong Trimui Terminal: sh bt-survey.sh
case "$0" in
  */*)
    cd "$(dirname "$0")" || exit 1
    ;;
esac
APP="$(pwd)"
STAMP="$(date +%Y%m%d-%H%M%S 2>/dev/null)"
[ -n "$STAMP" ] || STAMP=unknown-time
OUT="$APP/Bt-survey-$STAMP.log"
{
echo "===== trimui-terminal bt-survey ====="
date 2>&1
uname -a 2>&1
echo "--- model ---"
cat /sys/firmware/devicetree/base/model 2>/dev/null
echo
cat /proc/device-tree/model 2>/dev/null
echo
echo "--- tools ---"
command -v bluetoothctl hciconfig btmgmt rfkill lsmod dmesg python3 2>&1
echo "--- versions ---"
bluetoothd --version 2>&1
echo "--- bluetoothctl show ---"
{ echo show; echo quit; } | bluetoothctl 2>&1 | head -40
echo "--- hciconfig -a ---"
hciconfig -a 2>&1 | head -60
echo "--- btmgmt info ---"
btmgmt info 2>&1 | head -40
echo "--- rfkill list ---"
rfkill list 2>&1 | head -20
echo "--- lsmod ---"
lsmod 2>&1 | grep -i -e blue -e btbcm -e hci -e rfkill -e uhid -e uinput -e wlan
echo "--- devices ---"
ls -l /dev/uhid /dev/uinput 2>&1
ls /dev/hidraw* 2>&1
echo "--- kernel config ---"
zcat /proc/config.gz 2>/dev/null | grep -i -e CONFIG_BT -e CONFIG_UHID -e CONFIG_UINPUT | head -40
echo "--- main.conf ---"
cat /etc/bluetooth/main.conf 2>/dev/null | head -60
echo "--- dmesg ---"
dmesg 2>&1 | grep -i -e bluetooth -e hci -e rfkill | tail -40
echo "--- processes ---"
ps 2>&1 | grep -i blue | head
echo "--- python dbus ---"
python3 -c "import dbus; print('dbus ok')" 2>&1
echo "--- storage ---"
echo "APP=$APP"
df -h "$APP" 2>&1 | head -5
} > "$OUT" 2>&1
echo "saved: $OUT"
