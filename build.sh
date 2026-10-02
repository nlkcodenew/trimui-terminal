#!/bin/sh
# Wrapper: chay trong WSL/Linux. Uu tien SDK TG5050, fallback sang toolchain he thong.
# WSL: copy SDK ra ~ hoac gan "ln -s" vi duong dan "/mnt/e/..." co khoang trang
# lam `make` (CC khong trich dan) that bai. Mac dinh SDK_ROOT lay ban copy trong home.
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
if [ -z "${SDK_ROOT:-}" ]; then
  if [ -x "$HOME/tb/sdk/host/bin/aarch64-none-linux-gnu-gcc" ]; then
    SDK_ROOT="$HOME/tb/sdk"
  else
    SDK_ROOT="$ROOT/../sdk-tg5050/sdk_tg5050_linux_v1.0.0"
  fi
fi
export SDK_ROOT
if [ -x "$SDK_ROOT/host/bin/aarch64-none-linux-gnu-gcc" ]; then
  exec sh "$ROOT/native/build-tg5050.sh" "$@"
fi
VERSION="$(cat "$ROOT/VERSION" 2>/dev/null | tr -d " \r\n")"
[ -n "$VERSION" ] || VERSION=0.1.0
if command -v aarch64-linux-gnu-gcc >/dev/null 2>&1; then
  cd "$ROOT/native"
  make build PLATFORM=trimui-brick VERSION="$VERSION" CROSS_COMPILE=aarch64-linux-gnu- -j"${JOBS:-4}"
  cp trimui-terminal "$ROOT/files/bin/trimui-terminal"
  chmod +x "$ROOT/files/bin/trimui-terminal"
  exit 0
fi
echo "Thieu toolchain aarch64. Cai: sudo apt install gcc-aarch64-linux-gnu libc6-dev-arm64-cross libsdl2-dev:arm64 libsdl2-ttf-dev:arm64" >&2
exit 1