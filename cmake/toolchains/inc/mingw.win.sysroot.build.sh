#!/bin/sh

set -eu
# Always build where this script is, no matter where it is ran from.
cd "$(dirname "$(readlink -f "$0")")"
ARCH="${1:-x86_64}"

if [ -x "$(command -v wget)" ]; then
    DOWNLOAD="wget -O -"
elif [ -x "$(command -v curl)" ]; then
    DOWNLOAD="curl"
else
    echo "Missing curl or wget to download files..."
    exit 1
fi

rm -rf mingw.win.${ARCH}.sysroot
mkdir -p mingw.win.${ARCH}.sysroot/usr

${DOWNLOAD} https://github.com/libsdl-org/SDL/releases/download/release-3.2.4/SDL3-devel-3.2.4-mingw.tar.gz | tar -xz -C mingw.win.${ARCH}.sysroot/usr SDL3-3.2.4/${ARCH}-w64-mingw32 --strip-components 2

# Fix the sdl2-config file, which contains the wrong paths.
#sed -i "s|/opt/local/${ARCH}-w64-mingw32|$(pwd)/mingw.win.${ARCH}.sysroot/usr|" ./mingw.win.${ARCH}.sysroot/usr/lib/cmake/SDL2/sdl2-config.cmake

echo "Done with sysroot for mingw.win.${ARCH}.toolchain"
