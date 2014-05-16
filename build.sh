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

BUILDDIR="click/unity-scope-soundcloud"

export GOPATH="${GOPATH}:${HOME}"

rm -rf "${BUILDDIR}"
mkdir "${BUILDDIR}"

go build -o "${BUILDDIR}/unity-scope-soundcloud"
cp "soundcloud.ini" "${BUILDDIR}/unity-scope-soundcloud.ini"

click build click

rm -rf "${BUILDDIR}"
