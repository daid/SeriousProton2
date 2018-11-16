#!/bin/sh

set -eu
# Always build where this script is, no matter where it is ran from.
cd "$(dirname "$(readlink -f "$0")")"
ARCH="${1:-x86_64}"

rm -rf mingw.win.${ARCH}.sysroot
mkdir -p mingw.win.${ARCH}.sysroot/usr mingw.win.${ARCH}.sysroot/src/zlib

wget https://www.libsdl.org/release/SDL2-devel-2.0.9-mingw.tar.gz -O - | tar -xz -C mingw.win.${ARCH}.sysroot/usr SDL2-2.0.9/${ARCH}-w64-mingw32 --strip-components 2

wget https://zlib.net/zlib-1.2.11.tar.gz -O - | tar -xz -C mingw.win.${ARCH}.sysroot/src/zlib --strip-components 1
make -j 3 -C mingw.win.${ARCH}.sysroot/src/zlib/ -f win32/Makefile.gcc SHARED_MODE=1 PREFIX=${ARCH}-w64-mingw32- DESTDIR="$(pwd)/mingw.win.${ARCH}.sysroot/usr/" BINARY_PATH=bin INCLUDE_PATH=include LIBRARY_PATH=lib install

# Fix the sdl2-config file, which contains the wrong paths.
sed -i "s|/opt/local/${ARCH}-w64-mingw32|$(pwd)/mingw.win.${ARCH}.sysroot/usr|" ./mingw.win.${ARCH}.sysroot/usr/lib/cmake/SDL2/sdl2-config.cmake

echo "Done with sysroot for mingw.win.${ARCH}.toolchain"
