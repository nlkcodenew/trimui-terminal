# Cai dat Trimui Terminal v0.1.0

Terminal + BT survey cho TrimUI Brick Pro (Stock OS) va Smart Pro S.

## 1. Chuan bi

- The nho cua may (FAT32/exFAT).
- File `trimui-terminal-v0.1.0.zip` lay tu GitHub Releases (khong tai Source code).
- Neu chua co Release: dung file build san trong repo tai `dist/trimui-terminal-v0.1.0.zip`.
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

## 5. Dieu khien

- D-pad: chon phim tren ban phim ao.
- A: go phim. B: backspace.
- L1: shift. R1: giu phim. X: an/hien ban phim. Y: doi vi tri ban phim.
- START: enter. SELECT: tab. MENU: thoat.
- L2/R2: cuon lich su khi tat ban phim.

## 6. Loi thuong gap

| Hien tuong | Cach xu ly |
| --- | --- |
| Bat app thoat ngay | Mo `Apps/TrimuiTerminal/Terminal-loi.txt` xem loi SDL/thieu thu vien |
| Khong thay `launch.sh` | Giai nen sai cap thu muc; phai co `Apps/TrimuiTerminal/launch.sh`, khong phai `Apps/Apps/...` |
| `sh: bt-survey.sh: not found` | Dang dung thu muc khac; chay `pwd; ls` roi `cd` ve `Apps/TrimuiTerminal` |
| Chu tieng Viet hien o vuong | Thu `-font /duong/dan/font.ttf -fontsize 14` hoac doi font nhi phan 1..5 |
| Man hinh lech/xoay | Thu `-rotate 0`, `-scale 1` hoac `-scale 2` |
