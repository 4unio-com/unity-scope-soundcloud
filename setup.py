#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from distutils.core import setup
from DistUtilsExtra.command import *

setup(name='unity-scope-soundcloud',
      version='0.1',
      author='David Callé',
      author_email='davidc@framli.eu',
      url='http://launchpad.net/ubuntu-scopes',
      license='GNU General Public License (GPL)',
      data_files=[
    ('share/dbus-1/services', ['data/unity-scope-soundcloud.service']),
    ('share/unity-scopes/soundcloud', ['src/unity_soundcloud_daemon.py']),
    ('share/unity-scopes/soundcloud', ['src/__init__.py']),
    ], cmdclass={'build':  build_extra.build_extra,
                 'build_i18n': build_i18n.build_i18n,})
