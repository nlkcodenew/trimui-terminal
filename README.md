# trimui-terminal

Terminal SDL2 cho TrimUI Brick Pro (Stock OS, `sun50iw10`) va Smart Pro S, fork tu [haoict/SimpleTerminal](https://github.com/haoict/SimpleTerminal) tag `2.1.0` (MIT).

Muc dich chinh: co terminal chay duoc tren Brick Pro de chay `bt-survey`, phuc vu danh gia y tuong bien may thanh tay cam Bluetooth (BLE HID) cho iPad/iPhone/Android.

## Tai nhanh

1. Vao [Releases](../../releases/latest), tai `trimui-terminal-v0.1.0.zip` (khong tai Source code).
2. Giai nen vao goc the nho de co `Apps/TrimuiTerminal/launch.sh`.
3. Lap the vao may, mo **Trimui Terminal**.
4. Chua co GitHub Release? Dung truc tiep file build san: `dist/trimui-terminal-v0.1.0.zip` trong repo nay, giai nen vao goc the nho theo cung cau truc.

Chi tiet tung buoc: [CAI_DAT.md](CAI_DAT.md).

## Dieu khien

| Nut | Tac dung |
| --- | --- |
| D-pad | Chon phim tren ban phim ao |
| A | Go phim dang chon |
| B | Backspace (o man hinh help: gui Ctrl+C) |
| L1 | Shift |
| R1 | Giu/nha phim (toggle) |
| X | An/hien ban phim ao |
| Y | Doi vi tri ban phim (tren/duoi) |
| START | Enter |
| SELECT | Tab |
| L2 / R2 | Cuon lich su len/xuong (khi tat ban phim) |
| MENU | Thoat app |
| START + nut Exit tren ban phim + Enter | Thoat (du phong) |

Tham so khi chay: `-scale 2.0`, `-font 1..5 | /path/font.ttf`, `-fontsize N`, `-rotate 0|90|180|270`, `-r "lenh..."`, `-q`.

## BT survey (muc dich cua app)

Trong terminal chay:

```sh
sh bt-survey.sh
```

File `Bt-survey-YYYYMMDD-HHMMSS.log` duoc luu ngay canh `launch.sh`. Copy file nay ra may tinh va gui de duoc danh gia:

- May co ho tro BLE peripheral/advertise HID khong.
- Hay phai fallback USB HID gadget / WiFi virtual pad.

Huong dan doc ket qua: [docs/BLUETOOTH_SURVEY.md](docs/BLUETOOTH_SURVEY.md).

## Tuong thich

| May | Trang thai |
| --- | --- |
| Brick Pro Stock OS (`sun50iw10`) | Dich chinh, binary build bang SDK TG5050 |
| Smart Pro S | Cung binary, chua test het |
| SpruceOS / CrossMix | Du kien chay duoc, can test thuc te |

Binary can: `libSDL2-2.0.so.0`, `libSDL2_ttf-2.0.so.0` (co san tren Stock OS), `libpthread`, `libutil`, `libc`.

## Build

Can Linux (WSL Ubuntu) + toolchain AArch64 + SDL2/SDL2_ttf ARM64.

```sh
sh build.sh
python3 tools/verify_release.py
python3 tools/make_release.py
```

Uu tien SDK TG5050 (`../sdk-tg5050/sdk_tg5050_linux_v1.0.0`) neu co; fallback sang `aarch64-linux-gnu-gcc` cua he thong. Chi tiet: [docs/BUILD.md](docs/BUILD.md).

## Cau truc

- `files/` - app tren the nho: `launch.sh`, `bt-survey.sh`, `config.json`, `icon.png`, `bin/trimui-terminal`, `assets/fallback.ttf`.
- `native/src/` - source C (upstream 2.1.0 + patch Brick: mapping nut, `/bin/sh`, HOME fallback).
- `native/upstream-2.1.0/` - snapshot goc de doi chieu.
- `native/patches/` - patch rieng neu co (hien de trong, lich su patch nam truc tiep trong `native/src/`).
- `tools/` - `make_release.py`, `verify_release.py` (hoc theo trimui-chiaki-ng).
- `docs/` - trang thai du an, huong dan build, huong dan BT survey.
- `dist/` - ZIP cai dat + sha256 (build cuc bo, khong commit len git neu da co Release).
- `.github/workflows/build.yml` - CI build AArch64 + verify + dong goi.

## Trang thai du an

Xem [docs/PROJECT_STATUS.md](docs/PROJECT_STATUS.md). Ban hien tai: `v0.1.0` (xem [CHANGELOG.md](CHANGELOG.md)).

## Giay phep

MIT theo upstream. Xem [LICENSE](LICENSE). Upstream (c) haoict va cac tac gia st/suckless.
