# Prompt cho session moi - trimui-terminal

Copy toan bo khoi duoi day sang session moi:

---

Ban tiep tuc du an trimui-terminal (Terminal SDL2 cho TrimUI Brick Pro / Smart Pro S).

## Boi canh

- Repo local: `E:\Trimiu Brick Pro\Project APPS\Trimui-Terminal` (doc lap, khong nam trong chiaki-ng).
- GitHub: `https://github.com/nlkcodenew/trimui-terminal`, branch `main`, ban moi nhat `v0.2.3`.
- Doc bat buoc truoc khi lam: `README.md`, `CAI_DAT.md`, `CHANGELOG.md`, `docs/PROJECT_STATUS.md` (muc 2, 4, 5), `docs/BLUETOOTH_SURVEY.md`.
- Muc dich goc cua app: co terminal tren Brick Pro de chay `sh bt-survey.sh`, lay log Bluetooth danh gia y tuong tay cam BLE HID.

## Quy tac lam viec (bat buoc)

1. Chi sua `native/src/`; khong sua `native/upstream-2.1.0/` (snapshot goc haoict/SimpleTerminal 2.1.0).
2. Patch C bang replace chuoi phai dem so lan xuat hien dinh nghia chinh sau moi lan patch (da tung nhan doi main.c 1265 -> 2636 dong gay vo build).
3. Build trong WSL qua symlink `/tmp/tt-sdk` toi SDK TG5050 (duong dan Windows co dau cach lam make vo truc tiep). Staging build o `C:/tt-build/Trimui-Terminal`. Chi tiet: `docs/BUILD.md`.
4. Moi release: bump `VERSION` + `files/VERSION` + CHANGELOG + docs -> build SDK TG5050 (`PLATFORM=trimui-brick`) -> strip -> `make_release.py` -> `verify_release.py` (ALL OK) -> commit + tag + push -> `make_github_release.py`.
5. Binary phai la ELF AArch64 (machine 0xB7), phu thuoc chi SDL2/SDL2_ttf he thong.
6. Khong hardcode GitHub token (lay tu Windows credential manager nhu `tools/make_github_release.py`).

## Trang thai hien tai (v0.2.3)

- UI gon: terminal TTF 16, OSK font to rieng 26 scale co dinh, scale 1.0.
- Thoat: B lan 1 hien "B lan nua de thoat | A de huy", B lan 2 thoat; du phong MENU, SELECT+START, o Exit tren OSK.
- DPAD: 1 lan bam = 1 o, nav ty le cot, ho tro ca button + HAT.
- OTA tu dong: mo app la check + tai + cai, offline bo qua (tat bang `TERMINAL_NO_OTA=1`).
- Cho user test that v0.2.3: UI, B-thoat, DPAD, OTA auto. Lay file `Bt-survey-*.log` de danh gia BLE HID.

## Viec can lam

(Tu user, hoac neu user bao on: chuyen sang prototype tay cam BLE HID GATT 0x1812 / USB HID gadget.)

---
