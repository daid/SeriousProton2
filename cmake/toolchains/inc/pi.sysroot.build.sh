#!/bin/bash

set -eu
# Always build where this script is, no matter where it is ran from.
cd "$(dirname "$(readlink -f "$0")")"

TARGET=pi.sysroot

#rm -rf "${TARGET}"
#fakeroot /usr/sbin/multistrap -f pi.sysroot.multistrap.conf -d "${TARGET}"

# Strip a bunch of files we don't need anyhow. Saves disk space
rm -rf "${TARGET}/usr/share/locale"
rm -rf "${TARGET}/usr/share/doc"
rm -rf "${TARGET}/usr/share/man"
rm -rf "${TARGET}/usr/share/perl"
rm -rf "${TARGET}/usr/share/zoneinfo"
rm -rf "${TARGET}/var/cache/apt"

# Fix up the symlinks in the sysroot, find all links that start with absolute paths
#  and replace them with relative paths inside the sysroot.
for f in `find "${TARGET}" -type l`; do
    link=`readlink "${f}" || echo ''`
    if [ ! -z "${link}" ]; then
        if [ "${link:0:1}" == "/" ]; then
            if [ -e "${TARGET}${link}" ]; then
                rm "${f}"
                ln --relative -sf "${TARGET}${link}" "${f}"
            fi
        fi
    fi
done
