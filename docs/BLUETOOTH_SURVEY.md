# BT survey - huong dan + cach doc ket qua

Muc dich: xac dinh Brick Pro / Smart Pro S co lam duoc tay cam Bluetooth (BLE HID) khong.

## Chay survey

Trong Trimui Terminal tren may that:

```sh
sh bt-survey.sh
```

File `Bt-survey-YYYYMMDD-HHMMSS.log` nam ngay canh `launch.sh`. Copy ra may tinh va gui.

## Chay tay tung lenh (khi script thieu tool)

```sh
bluetoothctl show; hciconfig -a; btmgmt info
bluetoothd --version; ps | grep -i blue
lsmod | grep -i -e blue -e btbcm -e hci -e rfkill -e uhid -e uinput -e wlan
ls -l /dev/uhid /dev/uinput
ls /dev/hidraw*
zcat /proc/config.gz | grep -i -e CONFIG_BT -e CONFIG_UHID -e CONFIG_UINPUT
cat /etc/bluetooth/main.conf
dmesg | grep -i -e bluetooth -e hci | tail -n 50
python3 -c "import dbus; print('dbus ok')"
```

## Doc ket qua

### DAT (lam tiep BLE HID)

- Co `LEAdvertisingManager` / `GATTManager`, BlueZ tu 5.50 tro len.
- Co `uinput` hoac `uhid` (device node hoac module kernel).
- Python `import dbus` chay duoc.
- Huong tiep theo: prototype BLE HID GATT `0x1812`, advertise Appearance gamepad `0x03C4`, test voi Android (nRF Connect) truoc, iPad/iPhone sau.

### KHONG DAT (doi huong)

- Thieu advertise/peripheral, kernel khong co UHID/UINPUT.
- Stock OS cat BlueZ/dbus, khong co `bluetoothd` hoac input plugin.
- Huong fallback: USB HID gadget qua USB-C (de hon BT nhieu, cam iPad la nhan), hoac WiFi virtual pad.

### Hien tuong "Brick thay phone, phone khong thay Brick"

Day la hanh vi du kien, khong phai hong BT:

- Brick dang o che do scan (central): thay duoc loa/phone dang advertise.
- Muon lam tay cam, Brick phai advertise HID (peripheral): bat `discoverable/pairable` + advertise service HID.
- Menu BT mac dinh cua Stock OS thuong chi scan, khong advertise HID nen dien thoai quet khong ra.
