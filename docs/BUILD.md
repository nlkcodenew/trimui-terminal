# Build trimui-terminal

## Cach nhanh (WSL Ubuntu)

```sh
cd Trimui-Terminal
sh build.sh
```

`build.sh` tu chon: SDK TG5050 (`../sdk-tg5050/sdk_tg5050_linux_v1.0.0`) neu co, neu khong dung `aarch64-linux-gnu-gcc`.

## Cai toolchain he thong (khi khong dung SDK)

```sh
sudo dpkg --add-architecture arm64
sudo apt-get update
sudo apt-get install -y --no-install-recommends make file gcc-aarch64-linux-gnu libc6-dev-arm64-cross libsdl2-dev:arm64 libsdl2-ttf-dev:arm64
```

## Build tay

```sh
cd native
make build PLATFORM=trimui-brick VERSION=$(cat ../VERSION | tr -d " \r\n") CROSS_COMPILE=aarch64-linux-gnu-
cp trimui-terminal ../files/bin/trimui-terminal
```

## Verify + dong goi

```sh
python3 tools/verify_release.py
python3 tools/make_release.py
```

ZIP nam tai `dist/trimui-terminal-vX.Y.Z.zip` voi cau truc `Apps/TrimuiTerminal/`.
