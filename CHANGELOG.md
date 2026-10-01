# Changelog - trimui-terminal

## v0.1.0 - 2026-10-01

- Fork haoict/SimpleTerminal tag `2.1.0` (SDL2).
- Them profile nut `TRIMUI_BRICK` cho Brick Pro / Smart Pro S: A=1 B=0 X=3 Y=2 L1=4 R1=5 L2=6 R2=7 SELECT=8 START=9 L3=11 R3=12 UP=13 DOWN=14 LEFT=15 RIGHT=16.
- Shell mac dinh `/bin/sh` (Stock OS khong co bash), HOME fallback, giu nguyen VT100/PTY.
- Them `files/bt-survey.sh` thu thap Bluetooth mot cham cho danh gia tay cam BLE HID.
- Dong goi kieu chiaki-ng: `launch.sh` giu `stay_alive`, ZIP `Apps/TrimuiTerminal/`, manifest + sha256, verify gate, CI AArch64.
