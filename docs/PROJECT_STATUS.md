# trimui-terminal - trang thai du an (v0.2.6, 2026-10-02)

Repo doc lap: `Project APPS/Trimui-Terminal` (khong nam trong `chiaki-ng`).
GitHub: `nlkcodenew/trimui-terminal`, branch `main`. Release moi nhat: `v0.2.6`.

## 1. Muc tieu goc

- Co terminal chay duoc tren Brick Pro Stock OS (`sun50iw10`) truoc, Smart Pro S sau.
- Chay `files/bt-survey.sh` de thu thap thong tin Bluetooth, phuc vu danh gia y tuong tay cam BLE HID.
- OTA tu dong: mo app la tu len ban moi, khong copy tay.

## 2. Goc ky thuat (patch so voi upstream haoict/SimpleTerminal 2.1.0)

Snapshot goc giu tai `native/upstream-2.1.0/`, khong sua. Mọi patch nam truc tiep trong `native/src/`:

- `keyboard.h`: profile `TRIMUI_BRICK` (A=1 B=0 X=3 Y=2 L1=4 R1=5 L2=6 R2=7 SELECT=8 START=9 MENU=10 L3=11 R3=12 UP=13 DOWN=14 LEFT=15 RIGHT=16); prototype quit helpers.
- `keyboard.c`: B 2 lan thoat / A huy (4s) + combo SELECT+START + phim Exit tren OSK; DPAD nav bang ty le cot; huong dan + log tieng Viet (khong dau, font bitmap chi co ASCII).
- `main.c`: HAT cho TRIMUI_BRICK; tach font terminal (TTF 16) va OSK (pick co to nhat vua be ngang that); quit helpers + panel thoat to giua man hinh; ve lai terminal sau moi burst output; shutdown cho thread toi da 2s.
- `font.h/.c`: them font TTF thu 2 (`init_osk_ttf_font`, `draw_string_osk_ttf`) chi cho ban phim ao.
- `config.h`: shell `/bin/sh`, scale 1.0, fontsize terminal 16.
- `vt100.c`: HOME fallback + shell fallback.
- `files/`: them `ota-update.sh`, `VERSION`, `certs/cacert.pem`; `launch.sh` tu auto-apply OTA + truyen `-scale 1 -fontsize 16`.

## 3. Lich su release

- v0.1.0: fork + mapping nut + bt-survey + dong goi kieu chiaki-ng.
- v0.2.0: MENU/SELECT+START thoat, DPAD+HAT, TTF auto size 28, OTA (hoi y/N).
- v0.2.1: UI gon (term 16 + OSK 26), thoat B 2 lan / A huy. (Ban nay tung loi nhan doi main.c -> da dung lai source sach.)
- v0.2.2: sua DPAD nhay loan (bo double-handle KEYDOWN + held-repeat, nav ty le cot, OSK scale co dinh).
- v0.2.3: OTA tu dong hoan toan (mo app la cai, khong hoi), timeout fail-fast curl/wget.
- v0.2.4: OSK pick co to nhat vua man hinh; B lan 2 kill shell + select timeout (van sot deadlock).
- v0.2.5: sua OTA that (loi quote parse version + so sanh POSIX + apply subshell), sua treo B that (sigchld/tty_read/shutdown 2s), OSK that (load font sau khi biet 1280 + fallback + uu tien OSK TTF); sua build-tg5050.sh thieu dau `-`.
- v0.2.6: Viet hoa app (help/log/popup, khong dau) + README/docs.

## 4. Quy trinh build + release (da chot, dung lai moi lan)

1. Sua code trong `native/src/` (khong sua `upstream-2.1.0/`).
2. Bump `VERSION` + `files/VERSION` + `CHANGELOG.md` + docs lien quan.
3. Sync sang `C:/tt-build/Trimui-Terminal` (bo .git/dist/binary cu) roi build trong WSL bang SDK TG5050 qua symlink `/tmp/tt-sdk` (duong dan Windows co dau cach lam make vo truc tiep).
4. Strip + copy binary ve `files/bin/trimui-terminal`.
5. `python tools/make_release.py` (ZIP `Apps/TrimuiTerminal/` + manifest + sha256) roi `python tools/verify_release.py` (phai ALL OK).
6. Commit + tag `vX.Y.Z` + push main + push tag.
7. `python tools/make_github_release.py` (tao Release + upload ZIP/sha256/manifest). Token lay tu Windows credential manager, khong hardcode.

Lenh build chuan (WSL): toolchain `aarch64-none-linux-gnu-`, sysroot SDK TG5050, `PLATFORM=trimui-brick` (= BR2 + TRIMUI_BRICK), link SDL2/SDL2_ttf/pthread/util. Binary dung la ELF AArch64 (machine 0xB7).

## 5. Bai hoc xuong mau

- Khong patch bang replace chuoi nhieu lan khong kiem tra: de nhan doi block code (da tung nhan doi main.c 1265 -> 2636 dong, build vo). Sau moi dot patch phai dem so lan xuat hien dinh nghia chinh.
- Duong dan Windows co dau cach ("Trimiu Brick Pro") lam make + WSL vo: luon build qua `/tmp/tt-sdk` hoac copy sang `C:/tt-build`.
- DPAD Brick co the ve dang button hoac HAT tuy firmware: phai ho tro ca hai (JOYBUTTONDOWN convert + JOYHATMOTION).
- Nut MENU vat ly tren Brick bi OS nuot: thoat chinh phai la B 2 lan, MENU/SELECT+START chi la du phong.
- TTF size 28 cho ca terminal gay tran man hinh: tach font terminal (16) va OSK (26).
- visual_offset + fit-width dong gay nhay DPAD: nav ty le cot + OSK scale co dinh.

## 6. Viec tiep theo (cho session moi)

1. Cho user test OTA tu v0.2.5 len v0.2.6 tren may that (mo app la tu len, xem `Terminal-ota.log`).
2. Lay log `Bt-survey-*.log` de danh gia BLE HID (muc dich goc cua app).
3. Neu on: chuyen sang prototype tay cam (BLE HID GATT 0x1812) hoac USB HID gadget.
