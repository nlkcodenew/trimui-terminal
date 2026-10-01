# BT survey - huong dan + mau ket qua

Chay trong Trimui Terminal tren may that:

```sh
sh bt-survey.sh
```

## Cac lenh thu cong (neu script thieu tool)

```sh
bluetoothctl show; hciconfig -a; btmgmt info
bluetoothd --version; ps | grep -i blue
lsmod | grep -i -e blue -e uhid -e uinput; ls -l /dev/uhid /dev/uinput
cat /etc/bluetooth/main.conf; dmesg | grep -i -e bluetooth -e hci | tail -n 50
python3 -c "import dbus; print(123)"
```

## Nguong danh gia

- DAT: co `LEAdvertisingManager`/`GATTManager`, `/dev/uhid` hoac `uinput`, BlueZ >= 5.50, python dbus ok -> lam tiep prototype BLE HID.
- KHONG DAT: thieu advertise/peripheral, khong co uhid/uinput, Stock OS cat BlueZ -> chuyen huong USB HID gadget hoac WiFi virtual pad.
- Luu y: Brick thay duoc phone nhung phone khong thay Brick = dang o che do scan (central), chua advertise HID (peripheral). Day la hanh vi du kien, khong phai hong BT.
