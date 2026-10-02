# trimui-terminal

Terminal SDL2 cho TrimUI Brick Pro (Stock OS, `sun50iw10`) va Smart Pro S, fork tu [haoict/SimpleTerminal](https://github.com/haoict/SimpleTerminal) tag `2.1.0` (MIT).

Muc dich chinh: co terminal chay duoc tren Brick Pro de chay `bt-survey`, phuc vu danh gia y tuong bien may thanh tay cam Bluetooth (BLE HID) cho iPad/iPhone/Android.

## Tai nhanh

1. Vao [Releases](../../releases/latest), tai `trimui-terminal-vX.Y.Z.zip` (khong tai Source code).
2. Giai nen vao goc the nho de co `Apps/TrimuiTerminal/launch.sh`.
3. Lap the vao may, mo **Trimui Terminal**.
4. Tu v0.2.5: mo app la tu check + tu tai + tu cai ban moi, khong can copy tay nua.

Chi tiet tung buoc: [CAI_DAT.md](CAI_DAT.md).

## Dieu khien

| Nut | Tac dung |
| --- | --- |
| DPAD | Di chuyen 1 o tren ban phim ao (khong nhay) |
| A | Go phim dang chon / Huy hop thoat |
| B lan 1 | Hien "B lan nua de thoat \| A de huy" |
| B lan 2 (trong 4s) | Thoat app |
| L1 | Shift |
| R1 | Giu/nha phim (toggle) |
| X | An/hien ban phim ao |
| Y | Doi vi tri ban phim (tren/duoi) |
| START | Enter |
| SELECT | Tab |
| SELECT+START (giu cung luc) | Thoat app (du phong) |
| MENU | Thoat app (neu OS khong nuot) |
| L2 / R2 | Cuon lich su len/xuong (khi tat ban phim) |

Tham so khi chay: `-scale 1.0`, `-font 1..5 | /path/font.ttf`, `-fontsize N` (terminal), `-rotate 0|90|180|270`, `-r "lenh..."`, `-q`. Ban phim ao tu chon co chu to nhat vua full be ngang man hinh (Brick 1024).

## OTA tu dong

- Mo app la tu check + tai + cai (v0.2.5+ da sua loi OTA, chay that), log vao `Terminal-ota.log`. Offline thi bo qua, khong chan mo app. OTA bi gioi han 30s moi lan mo.
- Chay tay trong terminal: `sh ota-update.sh` (ban cu hoi y/N), `sh ota-update.sh --apply`, `sh ota-update.sh --check`.
- Tat tu dong: mo app voi `TERMINAL_NO_OTA=1`.

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

Uu tien SDK TG5050 (`../sdk-tg5050/sdk_tg5050_linux_v1.0.0`) neu co; fallback sang `aarch64-linux-gnu-gcc` cua he thong. Chi tiet: [docs/BUILD.md](docs/BUILD.md). Luu y WSL: duong dan Windows co dau cach phai build qua symlink `/tmp/tt-sdk` hoac copy repo sang duong dan khong dau cach (xem BUILD.md).

## Cau truc

- `files/` - app tren the nho: `launch.sh`, `ota-update.sh`, `bt-survey.sh`, `VERSION`, `config.json`, `icon.png`, `bin/trimui-terminal`, `assets/fallback.ttf`, `certs/cacert.pem`.
- `native/src/` - source C (upstream 2.1.0 + patch Brick).
- `native/upstream-2.1.0/` - snapshot goc de doi chieu (khong sua).
- `tools/` - `make_release.py`, `verify_release.py`, `make_github_release.py`.
- `docs/` - trang thai du an, huong dan build, huong dan BT survey, handoff session.
- `dist/` - ZIP cai dat + sha256 build cuc bo.
- `.github/workflows/build.yml` - CI build AArch64 + verify + dong goi.

## Trang thai du an

Xem [docs/PROJECT_STATUS.md](docs/PROJECT_STATUS.md). Ban hien tai: `v0.2.6` (xem [CHANGELOG.md](CHANGELOG.md)).

## Giay phep

MIT theo upstream. Xem [LICENSE](LICENSE). Upstream (c) haoict va cac tac gia st/suckless.
