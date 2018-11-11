#!/bin/sh

set -eu
# Always build where this script is, no matter where it is ran from.
cd "$(dirname "$(readlink -f "$0")")"


rm -rf mingw.win32.sysroot
mkdir -p mingw.win32.sysroot/usr

wget https://www.libsdl.org/release/SDL2-devel-2.0.9-mingw.tar.gz -O - | tar -xz -C mingw.win32.sysroot/usr SDL2-2.0.9/i686-w64-mingw32 --strip-components 2

wget http://gnuwin32.sourceforge.net/downlinks/zlib-bin-zip.php -O tmp.zlib-bin.zip
wget http://gnuwin32.sourceforge.net/downlinks/zlib-lib-zip.php -O tmp.zlib-dev.zip
unzip tmp.zlib-bin.zip -d mingw.win32.sysroot/usr
unzip tmp.zlib-dev.zip -d mingw.win32.sysroot/usr
rm tmp.zlib-bin.zip
rm tmp.zlib-dev.zip

# Fix the sdl2-config file, which contains the wrong paths.
sed -i "s|/opt/local/i686-w64-mingw32|$(pwd)/mingw.win32.sysroot/usr|" ./mingw.win32.sysroot/usr/lib/cmake/SDL2/sdl2-config.cmake

echo "Done with sysroot for mingw.win32.toolchain"
