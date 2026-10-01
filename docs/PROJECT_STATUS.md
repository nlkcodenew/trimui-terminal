# trimui-terminal - trang thai du an v0.1.0

> Cap nhat: 2026-10-01. Repo doc lap trong `Project APPS/Trimui-Terminal`, khong nam trong `chiaki-ng`.

## 1. Muc tieu

- Co terminal chay duoc tren Brick Pro Stock OS (`sun50iw10`) truoc, Smart Pro S sau.
- Chay `files/bt-survey.sh` de thu thap: `bluetoothctl show`, `hciconfig`, `btmgmt`, `lsmod`, `/dev/uhid|uinput`, kernel BT config, `bluetoothd` version, python dbus.
- Ket qua survey quyet dinh y tuong tay cam BT (BLE HID `0x1812`) co lam duoc khong.

## 2. Goc ky thuat

- Upstream: haoict/SimpleTerminal tag `2.1.0` (SDL2, MIT). Snapshot giu tai `native/upstream-2.1.0/`.
- Patch Brick (`native/src/`): profile nut `TRIMUI_BRICK` (A=1 B=0 X=3 Y=2 ...), shell mac dinh `/bin/sh` (Stock OS khong co bash), HOME fallback, giu nguyen VT100/PTY.
- Dong goi hoc theo trimui-chiaki-ng: `launch.sh` giu `/tmp/stay_alive`, ZIP `Apps/TrimuiTerminal/`, manifest + sha256, verify gate.

## 3. Trang thai

- v0.1.0: scaffold hoan chinh, cho build binary AArch64 dau tien.
- Can lam: build tren WSL -> chay that tren Brick -> gui log survey.
