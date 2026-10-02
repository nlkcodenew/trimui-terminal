# trimui-terminal

Terminal SDL2 cho TrimUI Brick Pro (Stock OS, `sun50iw10`) và Smart Pro S, fork từ [haoict/SimpleTerminal](https://github.com/haoict/SimpleTerminal) tag `2.1.0` (MIT).

Mục đích chính: có terminal chạy được trên Brick Pro để chạy `bt-survey`, phục vụ đánh giá ý tưởng biến máy thành tay cầm Bluetooth (BLE HID) cho iPad/iPhone/Android.

## Tải nhanh

1. Vào [Releases](../../releases/latest), tải `trimui-terminal-vX.Y.Z.zip` (không tải Source code).
2. Giải nén vào gốc thẻ nhớ để có `Apps/TrimuiTerminal/launch.sh`.
3. Lắp thẻ vào máy, mở **Trimui Terminal**.
4. Từ v0.2.5: mở app là tự kiểm tra + tự tải + tự cài bản mới, không cần copy tay nữa.

Chi tiết từng bước: [CAI_DAT.md](CAI_DAT.md).

## Điều khiển

| Nút | Tác dụng |
| --- | --- |
| DPAD | Di chuyển 1 ô trên bàn phím ảo (không nhảy) |
| A | Gõ phím đang chọn / Hủy hộp thoát |
| B lần 1 | Hiện bảng “BẤM B LẦN NỮA ĐỂ THOÁT” |
| B lần 2 (trong 4s) | Thoát app |
| L1 | Shift |
| R1 | Giữ/nhả phím (toggle) |
| X | Ẩn/hiện bàn phím ảo |
| Y | Đổi vị trí bàn phím (trên/dưới) |
| START | Enter |
| SELECT | Tab |
| SELECT+START (giữ cùng lúc) | Thoát app (dự phòng) |
| MENU | Thoát app (nếu OS không nuốt) |
| L2 / R2 | Cuộn lịch sử lên/xuống (khi tắt bàn phím) |

Tham số khi chạy: `-scale 1.0`, `-font 1..5 | /path/font.ttf`, `-fontsize N` (terminal), `-rotate 0|90|180|270`, `-r "lệnh..."`, `-q`. Bàn phím ảo tự chọn cỡ chữ to nhất vừa full bề ngang màn hình (Brick 1024).

## OTA tự động

- Mở app là tự kiểm tra + tải + cài (v0.2.5+ đã sửa lỗi OTA, chạy thật; không cần python3 trên máy), log vào `Terminal-ota.log`. Offline thì bỏ qua, không chặn mở app. OTA bị giới hạn 30s mỗi lần mở.
- Chạy tay trong terminal: `sh ota-update.sh` (bản cũ hỏi y/N), `sh ota-update.sh --apply`, `sh ota-update.sh --check`.
- Tắt tự động: mở app với `TERMINAL_NO_OTA=1`.

## Intro NLK (2.2s)

Mở app hiện logo chữ **NLK** đỏ (#E50914) trên nền gần đen, **do chính binary vẽ bằng SDL_ttf** ngay khi app mở (không phụ thuộc `fim`/`fbv`/`fbi` — firmware này không có trình xem framebuffer nào nên cách cũ không bao giờ hiện được).

Hiệu ứng **giống hệt Music-Player**: chữ bay lên lần lượt, nảy nhẹ, giãn ra, đổi từ đỏ sẫm sang đỏ tươi kèm quầng đỏ, rồi tia sáng trắng quét qua N → L → K. Tổng 2.2 giây, bấm phím bất kỳ là bỏ qua ngay. Cỡ chữ `giant = 132 × max(0.75, min(w/1024, h/768))` — trên Brick 1024×768 là 132px, y hệt Music-Player.

Tắt intro (mặc định bật): `TERMINAL_NO_INTRO=1`, tạo file `intro.off` hoặc `.no-intro` cạnh `launch.sh`, đặt `"intro": false` trong `config.json`, hoặc thêm `-nointro` khi gọi binary.

## BT survey (mục đích của app)

Trong terminal chạy:

```sh
sh bt-survey.sh
```

File `Bt-survey-YYYYMMDD-HHMMSS.log` được lưu ngay cạnh `launch.sh`. Copy file này ra máy tính và gửi để được đánh giá:

- Máy có hỗ trợ BLE peripheral/advertise HID không.
- Hay phải fallback USB HID gadget / WiFi virtual pad.

Hướng dẫn đọc kết quả: [docs/BLUETOOTH_SURVEY.md](docs/BLUETOOTH_SURVEY.md).

## Tương thích

| Máy | Trạng thái |
| --- | --- |
| Brick Pro Stock OS (`sun50iw10`) | Đích chính, binary build bằng SDK TG5050 |
| Smart Pro S | Cùng binary, chưa test hết |
| SpruceOS / CrossMix | Dự kiến chạy được, cần test thực tế |

Binary cần: `libSDL2-2.0.so.0`, `libSDL2_ttf-2.0.so.0` (có sẵn trên Stock OS), `libpthread`, `libutil`, `libc`.

## Build

Cần Linux (WSL Ubuntu) + toolchain AArch64 + SDL2/SDL2_ttf ARM64.

```sh
sh build.sh
python3 tools/verify_release.py
python3 tools/make_release.py
```

Ưu tiên SDK TG5050 (`../sdk-tg5050/sdk_tg5050_linux_v1.0.0`) nếu có; fallback sang `aarch64-linux-gnu-gcc` của hệ thống. Chi tiết: [docs/BUILD.md](docs/BUILD.md). Lưu ý WSL: đường dẫn Windows có dấu cách phải build qua symlink `/tmp/tt-sdk` hoặc copy repo sang đường dẫn không dấu cách (xem BUILD.md).

## Cấu trúc

- `files/` - app trên thẻ nhớ: `launch.sh`, `ota-update.sh`, `bt-survey.sh`, `VERSION`, `config.json`, `icon.png`, `bin/trimui-terminal`, `assets/fallback.ttf`, `certs/cacert.pem`.
- `native/src/` - source C (upstream 2.1.0 + patch Brick).
- `native/upstream-2.1.0/` - snapshot gốc để đối chiếu (không sửa).
- `tools/` - `make_release.py`, `verify_release.py`, `make_github_release.py`.
- `docs/` - trạng thái dự án, hướng dẫn build, hướng dẫn BT survey, handoff session.
- `dist/` - ZIP cài đặt + sha256 build cục bộ.
- `.github/workflows/build.yml` - CI build AArch64 + verify + đóng gói.

## Trạng thái dự án

Xem [docs/PROJECT_STATUS.md](docs/PROJECT_STATUS.md). Bản hiện tại: `v0.3.0` (xem [CHANGELOG.md](CHANGELOG.md)).

## Giấy phép

MIT theo upstream. Xem [LICENSE](LICENSE). Upstream (c) haoict và các tác giả st/suckless.
