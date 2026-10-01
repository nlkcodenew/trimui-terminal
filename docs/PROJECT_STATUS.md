# trimui-terminal - trang thai du an v0.2.0

> Cap nhat: 2026-10-01. Repo doc lap trong `Project APPS/Trimui-Terminal`, khong nam trong `chiaki-ng`.

## 1. Muc tieu

- Co terminal chay duoc tren Brick Pro Stock OS (`sun50iw10`) truoc, Smart Pro S sau.
- Chay `files/bt-survey.sh` de thu thap: `bluetoothctl show`, `hciconfig`, `btmgmt`, `lsmod`, `/dev/uhid|uinput`, kernel BT config, `bluetoothd` version, python dbus.
- Ket qua survey quyet dinh y tuong tay cam BT (BLE HID `0x1812`) co lam duoc khong.

## 2. Goc ky thuat

- Upstream: haoict/SimpleTerminal tag `2.1.0` (SDL2, MIT). Snapshot giu tai `native/upstream-2.1.0/`.
- Patch Brick (`native/src/` so voi upstream):
  - `keyboard.h`: them profile `TRIMUI_BRICK` (A=1 B=0 X=3 Y=2, L1=4 R1=5, L2=6 R2=7, SELECT=8 START=9, MENU=10, L3=11 R3=12, UP=13 DOWN=14 LEFT=15 RIGHT=16).
  - `config.h`: shell mac dinh `/bin/sh` vi Stock OS khong co bash.
  - `vt100.c`: HOME fallback + shell fallback, giu nguyen co che openpty/fork/exec.
  - `main.c`, `keyboard.c`: doi nhan "Simple Terminal" thanh "Trimui Terminal".
- Build: SDK TG5050 (`aarch64-none-linux-gnu-gcc`, sysroot co SDL2/SDL2_ttf), `PLATFORM=trimui-brick` bat ca `BR2` + `TRIMUI_BRICK`.
- Binary `files/bin/trimui-terminal`: ELF AArch64, strip, phu thuoc `libSDL2-2.0.so.0`, `libSDL2_ttf-2.0.so.0`, `libpthread`, `libutil`, `libc`.
- Dong goi hoc theo trimui-chiaki-ng: `launch.sh` giu `/tmp/stay_alive`, ZIP `Apps/TrimuiTerminal/`, manifest + sha256, verify gate, CI build AArch64.

## 3. Release v0.1.0

- Binary: `593408` bytes, NEEDED chi con SDL2/SDL2_ttf he thong.
- ZIP: `trimui-terminal-v0.1.0.zip` (`417902` bytes), 6 file OTA, cau truc `Apps/TrimuiTerminal/`.
- Verify: 12/12 PASS (chua tinh SKIP ZIP thi 9/9 PASS truoc khi co ZIP).
- Cach cai: xem [CAI_DAT.md](../CAI_DAT.md), cach build lai: [BUILD.md](BUILD.md).

## 4. Viec tiep theo

1. Cai that tren Brick Pro, chay `sh bt-survey.sh`, gui log.
2. Doc log theo [BLUETOOTH_SURVEY.md](BLUETOOTH_SURVEY.md) de chot BLE HID / USB HID / WiFi pad.
3. Neu terminal loi font/scale/rotate tren may that: ghi lai model + chup man hinh + gui `Terminal-loi.txt`.
