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
