#!/bin/sh
# Build trimui-terminal bang SDK TG5050 (da co san trong workspace).
set -eu
REPO_ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
SDK_ROOT="${SDK_ROOT:-$(CDPATH= cd -- "$REPO_ROOT/../sdk-tg5050/sdk_tg5050_linux_v1.0.0" && pwd)}"
VERSION="$(cat "$REPO_ROOT/VERSION" 2>/dev/null | tr -d " \r\n")"
[ -n "$VERSION" ] || VERSION=0.1.0
GCC="$SDK_ROOT/host/bin/aarch64-none-linux-gnu-gcc"
SYSROOT="$SDK_ROOT/host/aarch64-buildroot-linux-gnu/sysroot"
STRIP="$SDK_ROOT/host/bin/aarch64-none-linux-gnu-strip"
test -x "$GCC" || { echo "Khong tim thay SDK gcc: $GCC" >&2; exit 1; }
test -d "$SYSROOT/usr/include/SDL2" || { echo "Thieu SDL2 trong sysroot: $SYSROOT" >&2; exit 1; }
cd "$REPO_ROOT/native"
make build PLATFORM=trimui-brick VERSION="$VERSION" CROSS_COMPILE="${GCC%gcc}" SYSROOT="$SYSROOT" -j"${JOBS:-4}"
"$STRIP" --strip-unneeded trimui-terminal
cp trimui-terminal "$REPO_ROOT/files/bin/trimui-terminal"
chmod +x "$REPO_ROOT/files/bin/trimui-terminal"
ls -l "$REPO_ROOT/files/bin/trimui-terminal"
file "$REPO_ROOT/files/bin/trimui-terminal" 2>/dev/null || true
