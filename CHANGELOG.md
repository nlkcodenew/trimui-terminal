## v0.3.3 - 2026-10-02

- Logo khoi dong NLK **ve bang chinh app** (SDL_ttf) thay vi dua cho `fim`/`fbv`/`fbi`: firmware nay khong co trinh xem framebuffer nao nen cach cu khong bao gio hien du co `assets/intro.png`. Gio nền gần đen + chữ đỏ #E50914 có quầng, canh giữa, co vừa màn hình nhỏ, ~1.2s, bấm phím bất kỳ bỏ qua ngay.
- Bỏ khối `fim/fbv/fbi` trong `launch.sh` (chết, gây hiểu nhầm là đã có intro).
- Tắt intro: `TERMINAL_NO_INTRO=1`, file `intro.off` / `.no-intro`, `config.json` `"intro": false`, hoặc thêm `-nointro` khi gọi binary.
- `verify_release.py` chặn release nếu binary không chứa intro hoặc được biên dịch sai `-DVERSION`.
- OTA: xoá luôn `.update_staging.apply.list` cạnh thư mục staging (trước đó sót lại mỗi lần cập nhật).

## v0.3.2 - 2026-10-02

- Sửa OTA "cập nhật xong" nhưng app vẫn hiện version cũ: nhãn version + banner + credit giờ đọc file `VERSION` (do `ota-update.sh` ghi lại sau mỗi lần cập nhật) thay vì hằng `-DVERSION` lúc biên dịch. Bản v0.3.1 đã đóng gói y hệt binary của v0.3.0 (`-DVERSION="0.3.0"`) nên không bao giờ đổi nhãn.
- Sửa badge "Đang kiểm tra..." dính vĩnh viễn ở góc phải: trước đây khi OTA xong và xoá `.ota-status`, binary không xoá badge. Badge giờ bị xoá khi file trạng thái biến mất.
- Sửa `.ota-status` ghi không atomic (app có thể đọc file rỗng lúc mới mở): ghi file tạm rồi `mv`.
- `build.sh`/`native/build-tg5050.sh` dùng `SDK_ROOT` (mặc định `~/tb/sdk`) vì đường dẫn `/mnt/e/Trimiu Brick Pro/...` có khoảng trắng làm `make` fail.
- `verify_release.py` chặn `files/VERSION` lệch với `VERSION` + kiểm tra `intro.png` tồn tại.

## v0.3.1 - 2026-10-02

- Intro splash NLK tĩnh ~1s khi mở app: ảnh `assets/intro.png` 1024x768 render sẵn bằng Pillow (`python tools/render_intro.py`), nền (8,8,12) + chữ đỏ #E50914, chiếu bằng fim/fbv/fbi (thử lần lượt, có `-a` auto-scale, không có thì bỏ qua êm).
- Tắt intro: `TERMINAL_NO_INTRO=1`, hoặc file `intro.off` / `.no-intro` cạnh `launch.sh`, hoặc `"intro": false` trong `config.json` (mặc định bật). Không bao giờ chặn boot: mọi lỗi câm, luôn chạy tiếp `trimui-terminal`.
## v0.3.0 - 2026-10-02

- Sửa vỡ font tiếng Việt: `TTF_RenderText_*` trên SDL_ttf của máy hiểu chuỗi theo Latin-1 (chữ UTF-8 thành `Báº¥m...`); chuyển toàn bộ sang `TTF_RenderUTF8_*`.
- Nhãn version thường trực `v0.3.0` góc trên-phải vùng terminal khi gõ lệnh.
- Sửa dòng credit đè lên nội dung trợ giúp: chuyển xuống đáy màn hình (TTF) / cuối màn hình (bitmap).
## v0.2.9 - 2026-10-02

- Thông báo cập nhật: OTA chạy nền (app mở ngay), hiện badge "Đang tải..." góc phải trong lúc tải, popup lớn "ĐÃ CẬP NHẬT LÊN vX - MỞ LẠI APP ĐỂ DÙNG" khi xong (đọc file .ota-status do ota-update.sh ghi).
- Sửa banner/version không bao giờ hiện: bỏ dòng tự tắt help khi có TTF; banner hiện mỗi lần mở app đến khi bấm phím.
- Chữ Việt ổn định: popup/banner/badge render 1 lần rồi cache (dùng kích thước thật của chữ, API UTF8 tường minh), không vẽ lại mỗi frame.
## v0.2.8 - 2026-10-02

- Việt hóa có dấu toàn bộ app (xác minh font DejaVu kèm theo đủ 136/136 ký tự Việt) + README có dấu.
- Banner màn hình giúp đỡ hiện tên + số phiên bản ("Trimui Terminal v0.2.8") và dòng gợi ý.
## v0.2.7 - 2026-10-02

- Sua terminal den thui (chi thay ban phim): ban va 0.2.4 doi select() sang timeout 200ms nhung continue luon, bo qua SDL_PushEvent ve lai -> terminal khong bao gio repaint. Khoi phuc ve lai sau moi burst output + co tty_data_pending + ve khung ngay khi mo app.
- Sua OTA khong can python3: them duong tai shell thuan (curl/wget + sha256sum, parse manifest bang awk) khi may khong co python3 (Stock OS chi co header python, khong co trinh thong dich). Them thu lai manifest 2 lan cho WiFi len cham.
- Panel xac nhan thoat to giua man hinh (vien vang nen den, font OSK to, 2 dong), thay popup bitmap be ti.
- Phat hien tu log may that: man hinh Brick la 1024x768 (khong phai 1280), OSK pick size 24 la dung.
## v0.2.6 - 2026-10-02

- Viet hoa app: man hinh huong dan, popup luu anh, log thoat (tieng Viet khong dau vi font bitmap chi co ASCII). README/docs cap nhat theo.
## v0.2.5 - 2026-10-02

- Sua OTA khong bao gio tu chay: parse version dung sys.argv (ban cu loi quote khien python SyntaxError, REM rong, luon "manifest khong co version"); so sanh version thuan shell (khong phu thuoc sort -V cua BusyBox); timeout fail-fast (curl/wget 5s+10s, python 10s/15s); sua vong apply pipe-while (exit trong subshell khong tac dung); launch.sh gioi han OTA 30s bang timeout va log rieng Terminal-ota.log.
- Sua B lan 2 treo that: sig_chld khong exit() trong signal handler nua (reap WNOHANG + co child_exited, main_loop tu thoat); tty_read tra ve -1 thay vi die() trong thread (die -> WaitThread chinh no = deadlock); kill ca process group; SIGPIPE ignore; sdl_shutdown cho toi da 2s roi bo qua join thay vi doi vo han; go shell typed exit cung ve menu.
- Sua OSK thu nho that: load font SAU khi biet kich thuoc man hinh that (truoc day load khi width=0); pick fallback size 16 neu khong co nao vua; draw uu tien OSK TTF ke ca khi term TTF loi; cleanup dong ca 2 font.
## v0.2.4 - 2026-10-01

- Sua OSK thu nho: chon co TTF lon nhat vua that be ngang full 1280 va 6 hang vua 55% chieu cao (pick_osk_ttf_font thu size 48->16), dung kich thuoc glyph thuc te, khong scale 3/4 cung.
- Sua B lan 2 treo: kill shell con (TERM roi KILL) + select() timeout 200ms de thread tty tu thoat + tty_write/tty_read chiu duoc fd chet + bao thread_should_exit truoc khi kill.
## v0.2.3 - 2026-10-01

- OTA tu dong hoan toan: mo app la tu check + tu tai + tu cai (khong hoi y/N). That bai/offline thi bo qua, khong chan mo app. Them timeout fail-fast cho curl/wget.
# Changelog - trimui-terminal

## v0.2.2 - 2026-10-01

- Sua DPAD nhay loan ban phim ao: bo xu ly trung (KEYDOWN + main_loop held-repeat), viet lai handle_narrow_keys_held bang ty le cot (khong con visual_offset) de len/xuong giua cac hang khac do dai khong nhay lung tung. OSK dung scale co dinh 3/4, khong fit-width dong theo man hinh, nen vi tri ve va chi so i luon khop nhau.
## v0.2.1 - 2026-10-01

- Sua UI tran man hinh: terminal dung font TTF size 16 (gon), ban phim ao dung font to rieng size 26 co fit-width theo man hinh; scale mac dinh 1.0.
- Thoat bang B 2 lan: B lan 1 hien "B lan nua de thoat | A de huy", B lan 2 thoat, A huy. Giu MENU va combo SELECT+START.
## v0.2.0 - 2026-10-01

- Thoat app: MENU (KEY_QUIT) + combo du phong SELECT+START an cung luc + phim Exit tren ban phim ao (di chuyen toi Exit roi bam A).
- DPAD dieu huong ban phim ao: DPAD button (13-16) di chuyen ngay tu lan bam dau, khong cho giu 150ms; mo rong xu ly SDL_JOYHATMOTION cho TRIMUI_BRICK.
- Ban phim ao to, de doc: tu dong nap TTF he thong (DejaVuSansMono/DejaVuSans/fallback.ttf) voi fontsize mac dinh 28; launch.sh truyen -fontsize 28.
- OTA tu xa: them files/ota-update.sh (manifest GitHub + staging + verify sha256 + apply), files/certs/cacert.pem, files/VERSION; launch.sh tu check ban moi moi lan mo (tat bang TERMINAL_NO_OTA=1).
## v0.1.0 - 2026-10-01

- Fork haoict/SimpleTerminal tag `2.1.0` (SDL2).
- Them profile nut `TRIMUI_BRICK` cho Brick Pro / Smart Pro S: A=1 B=0 X=3 Y=2 L1=4 R1=5 L2=6 R2=7 SELECT=8 START=9 L3=11 R3=12 UP=13 DOWN=14 LEFT=15 RIGHT=16.
- Shell mac dinh `/bin/sh` (Stock OS khong co bash), HOME fallback, giu nguyen VT100/PTY.
- Them `files/bt-survey.sh` thu thap Bluetooth mot cham cho danh gia tay cam BLE HID.
- Dong goi kieu chiaki-ng: `launch.sh` giu `stay_alive`, ZIP `Apps/TrimuiTerminal/`, manifest + sha256, verify gate, CI AArch64.
