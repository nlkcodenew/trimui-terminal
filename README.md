# trimui-terminal

Terminal SDL2 cho TrimUI Brick Pro (Stock OS, `sun50iw10`) va Smart Pro S, fork tu [haoict/SimpleTerminal](https://github.com/haoict/SimpleTerminal) tag `2.1.0` (MIT).

Muc dich chinh: co terminal tren Brick Pro de chay `bt-survey`, phuc vu danh gia y tuong bien may thanh tay cam Bluetooth (BLE HID) cho iPad/iPhone/Android.

## Cai dat (Brick Pro)

1. Vao [Releases](../../releases/latest), tai `trimui-terminal-vX.Y.Z.zip`.
2. Giai nen vao goc the nho de co `Apps/TrimuiTerminal/launch.sh`.
3. Tren may mo **Trimui Terminal**.
4. Dieu khien: D-pad chon phim, A go phim, B backspace, L1 shift, X an/hien ban phim, START enter, SELECT tab, MENU thoat.

## BT survey (muc dich cua app)

Trong terminal chay:

```sh
sh bt-survey.sh
```

File `Bt-survey-YYYYMMDD-HHMMSS.log` duoc luu canh app. Gui file nay de duoc danh gia: may co ho tro BLE peripheral/advertise HID khong, hay phai fallback USB HID / WiFi virtual pad.

## Build

Can Linux (WSL Ubuntu) + toolchain AArch64 + SDL2/SDL2_ttf ARM64.

```sh
sh build.sh
python3 tools/verify_release.py
python3 tools/make_release.py
```

Uu tien SDK TG5050 (`../sdk-tg5050/sdk_tg5050_linux_v1.0.0`) neu co; fallback sang `aarch64-linux-gnu-gcc` cua he thong. Chi tiet: `docs/BUILD.md`.

## Cau truc

- `files/` - app tren the nho: `launch.sh`, `bt-survey.sh`, `config.json`, `icon.png`, `bin/trimui-terminal`.
- `native/src/` - source C (upstream 2.1.0 + patch Brick: mapping nut, `/bin/sh`, HOME fallback).
- `native/upstream-2.1.0/` - snapshot goc de doi chieu.
- `tools/` - `make_release.py`, `verify_release.py` (hoc theo trimui-chiaki-ng).
- `docs/` - trang thai du an, huong dan build, mau BT survey.

## Giay phep

MIT theo upstream. Xem `LICENSE`.
