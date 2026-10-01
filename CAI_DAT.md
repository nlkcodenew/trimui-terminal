# Cai dat Trimui Terminal

Terminal + BT survey cho TrimUI Brick Pro (Stock OS) va Smart Pro S.

## 1. Chuan bi

- The nho cua may (FAT32/exFAT).
- File `trimui-terminal-vX.Y.Z.zip` lay tu GitHub Releases (khong tai Source code).
- Neu chua co Release: dung file build san trong repo tai `dist/trimui-terminal-vX.Y.Z.zip`.
- Doi chieu SHA-256 bang file `.zip.sha256` di kem neu can.

## 2. Cai moi

1. Giai nen ZIP vao goc the nho.
2. Kiem tra co duong dan: `Apps/TrimuiTerminal/launch.sh`.
3. Cau truc dung sau khi giai nen:

```text
Apps/TrimuiTerminal/launch.sh
Apps/TrimuiTerminal/config.json
Apps/TrimuiTerminal/icon.png
Apps/TrimuiTerminal/bt-survey.sh
Apps/TrimuiTerminal/bin/trimui-terminal
Apps/TrimuiTerminal/assets/fallback.ttf
```

4. Thao the an toan, lap vao may, mo **Trimui Terminal**.

## 3. Cap nhat

Giai nen / copy de file ZIP moi len `Apps/TrimuiTerminal/`. Khong can xoa thu muc cu truoc.

Cac file log do app tao ra (vi du `Terminal-loi.txt`, `Bt-survey-*.log`) khong co trong ZIP nen khong bi ghi de.

## 4. Chay BT survey

1. Trong terminal go:

```sh
sh bt-survey.sh
```

2. Neu bao khong thay file, di chuyen ve thu muc app truoc:

```sh
pwd
ls
sh bt-survey.sh
```

3. File `Bt-survey-YYYYMMDD-HHMMSS.log` nam ngay canh `launch.sh`.
4. Tat app, cam the vao may tinh, copy file log ra de gui danh gia Bluetooth.

## 5. Thoat app

- Bam B 1 lan: hien "B lan nua de thoat | A de huy".
- Bam B lan 2 trong 4 giay: thoat. Bam A: huy, tiep tuc dung.
- Du phong: giu MENU, hoac giu SELECT+START cung luc, hoac chon o Exit tren ban phim ao roi bam A.

## 6. Dieu khien

- D-pad: chon phim tren ban phim ao.
- A: go phim. B: backspace.
- L1: shift. R1: giu phim. X: an/hien ban phim. Y: doi vi tri ban phim.
- START: enter. SELECT: tab. MENU: thoat.
- L2/R2: cuon lich su khi tat ban phim.

## 7. Cap nhat tu xa (OTA)

1. Mo app, OTA tu kiem tra ban moi moi lan chay (ghi vao `Terminal-ota.log`).
2. De cap nhat thu cong trong terminal: `sh ota-update.sh` (hoi truoc khi cai) hoac `sh ota-update.sh --apply`.
3. OTA tai manifest tu `https://raw.githubusercontent.com/nlkcodenew/trimui-terminal/main/manifest.json`, verify sha256 tung file, cai atomically, luu VERSION moi. Xong thoat app mo lai.
4. Tat auto-check: mo `launch.sh` voi `TERMINAL_NO_OTA=1`.

## 8. Loi thuong gap

| Hien tuong | Cach xu ly |
| --- | --- |
| Bat app thoat ngay | Mo `Apps/TrimuiTerminal/Terminal-loi.txt` xem loi SDL/thieu thu vien |
| Khong thay `launch.sh` | Giai nen sai cap thu muc; phai co `Apps/TrimuiTerminal/launch.sh`, khong phai `Apps/Apps/...` |
| `sh: bt-survey.sh: not found` | Dang dung thu muc khac; chay `pwd; ls` roi `cd` ve `Apps/TrimuiTerminal` |
| Chu tieng Viet hien o vuong | Thu `-font /duong/dan/font.ttf -fontsize 14` hoac doi font nhi phan 1..5 |
| Man hinh lech/xoay | Thu `-rotate 0`, `-scale 1` hoac `-scale 2` |
