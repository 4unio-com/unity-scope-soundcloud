#!/bin/sh

# Copyright (C) 2014 Canonical Ltd
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Authored by: Pete Woods <pete.woods@canonical.com>

# Simple script to build a click packaged scope.

if [ "$#" -ne 1 ]; then
    echo "Usage: click-build.sh amd64|armhf|..."
    exit 1
fi

CLICK_ARCH="$1"
BUILDDIR="$PWD/builddir"

export GOPATH="${GOPATH}:${HOME}"

rm -rf "${BUILDDIR}"
mkdir -p "${BUILDDIR}/soundcloud"

go build -o "${BUILDDIR}/soundcloud/com.pete-woods.soundcloud_soundcloud"
cp "soundcloud.ini" "${BUILDDIR}/soundcloud/com.pete-woods.soundcloud_soundcloud.ini"
cp "click/scope-security.json" "${BUILDDIR}"
sed -e "s/%CLICK_ARCH%/${CLICK_ARCH}/g" "click/manifest.json" > "${BUILDDIR}/manifest.json"

click build "${BUILDDIR}"

rm -rf "${BUILDDIR}"
