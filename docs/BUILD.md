# Build trimui-terminal

Yeu cau: Linux (WSL Ubuntu duoc), toolchain AArch64, SDL2 + SDL2_ttf ban ARM64.

## Cach nhanh (WSL Ubuntu)

```sh
cd Trimui-Terminal
sh build.sh
python3 tools/verify_release.py
python3 tools/make_release.py
```

`build.sh` tu chon theo thu tu:

1. SDK TG5050 tai `../sdk-tg5050/sdk_tg5050_linux_v1.0.0` (duong dan tuong doi tu repo).
2. `aarch64-linux-gnu-gcc` cua he thong neu khong co SDK.

Luu y WSL: duong dan Windows co dau cach (vi du `Trimiu Brick Pro`) lam make vo. Repo nay build qua symlink `/tmp/tt-sdk` trong script mau `C:/tt-build/run.sh`, hoac copy repo sang duong dan khong dau cach roi build.

## Cai toolchain he thong (khi khong dung SDK)

```sh
sudo dpkg --add-architecture arm64
sudo apt-get update
sudo apt-get install -y --no-install-recommends make file gcc-aarch64-linux-gnu libc6-dev-arm64-cross libsdl2-dev:arm64 libsdl2-ttf-dev:arm64
```

## Build truc tiep bang SDK TG5050

```sh
SDK_ROOT=/path/to/sdk_tg5050_linux_v1.0.0 sh native/build-tg5050.sh
```

Script tu lay VERSION tu file `VERSION`, build `PLATFORM=trimui-brick`, strip, copy vao `files/bin/trimui-terminal`.

## Build tay khong can script

```sh
cd native
make build PLATFORM=trimui-brick VERSION=$(cat ../VERSION | tr -d " \r\n") CROSS_COMPILE=aarch64-linux-gnu-
file trimui-terminal
readelf -d trimui-terminal | grep NEEDED
cp trimui-terminal ../files/bin/trimui-terminal
```

Binary dung phai la ELF AArch64 (`Machine: 0xB7`), NEEDED chi co SDL2/SDL2_ttf/pthread/util/libc.

## Verify + dong goi

```sh
python3 tools/verify_release.py
python3 tools/make_release.py
```

- Verify kiem tra: du file app, binary ELF AArch64, config tro dung launch, cau truc ZIP, khong dinh secret.
- Make release tao: `dist/trimui-terminal-vX.Y.Z.zip` (cau truc `Apps/TrimuiTerminal/`), `.zip.sha256`, `manifest.json`.

## CI

`.github/workflows/build.yml` chay tren Ubuntu 24.04: cai toolchain ARM64, build, verify, dong goi, upload artifact (binary + ZIP + sha256 + manifest).
