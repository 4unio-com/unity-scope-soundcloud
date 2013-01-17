#! /usr/bin/python
# -*- coding: utf-8 -*-

# Copyright (C) 2012 name <email>
# This program is free software: you can redistribute it and/or modify it 
# under the terms of the GNU General Public License version 3, as published 
# by the Free Software Foundation.
# 
# This program is distributed in the hope that it will be useful, but 
# WITHOUT ANY WARRANTY; without even the implied warranties of 
# MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR 
# PURPOSE.  See the GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License along 
# with this program.  If not, see <http://www.gnu.org/licenses/>.

from gi.repository import GLib, GObject, Gio
from gi.repository import Unity, UnityExtras
import urllib
import urllib2
import simplejson
import gettext

APP_NAME = 'unity-scope-soundcloud'
LOCAL_PATH = '/usr/share/locale/'
gettext.bindtextdomain(APP_NAME, LOCAL_PATH)
gettext.textdomain(APP_NAME)
_ = gettext.gettext

BUS_NAME = 'com.canonical.Unity.Scope.Music.Soundcloud'
BUS_PATH = '/com/canonical/unity/scope/music/soundcloud'

SVG_DIR = '/usr/share/icons/unity-icon-theme/places/svg/'
CAT_0_ICON = Gio.ThemedIcon.new(SVG_DIR + 'group-music.svg')
CAT_0_TITLE = _('Categorytitle')

NO_RESULTS_HINT = _('Sorry, there are no Soundcloud results that match your search.')
SEARCH_HINT = _('Search Soundcloud')
SEARCH_URI = 'https://api.soundcloud.com/'
#FIXME register Canonical API key
API_KEY = '916e2a744323e1f28e8f1fe50728f86d'


class Daemon:
    def __init__ (self):
        self.scope = Unity.Scope.new (BUS_PATH, 'soundcloud')
        self.scope.props.search_hint = SEARCH_HINT
        self.scope.search_in_global = True
        cats = []
        cats.append(Unity.Category.new('cat_0',
                                       CAT_0_TITLE,
                                       CAT_0_ICON,
                                       Unity.CategoryRenderer.VERTICAL_TILE))
        self.scope.props.categories = cats
        self.preferences = Unity.PreferencesManager.get_default()
        self.preferences.connect('notify::remote-content-search', self._on_preference_changed)
        self.scope.connect('search-changed', self.on_search_changed)
        self.scope.props.metadata_schema = {'album': 's',
                                            'artist': 's'}
        self.scope.props.optional_metadata_schema = {'genre': 's',
                                                     'label': 's',
                                                     'license': 's',
                                                     'stream': 's'}
        self.scope.export()

    def _on_preference_changed(self, *_):
        self.scope.queue_search_changed(Unity.SearchType.DEFAULT)

    def on_search_changed(self, scope, search, search_type, *_):
        model = search.props.results_model
        model.clear()
        if self.preferences.props.remote_content_search != Unity.PreferencesManagerRemoteContent.ALL:
            search.finished()
            return
        search_string = search.props.search_string.strip()
        print ('Search changed to \'%s\'' % search_string)
        self.update_results_model(search_string, model)
        search.set_reply_hint('no-results-hint',
                              GLib.Variant.new_string(NO_RESULTS_HINT))
        search.finished()

    def update_results_model(self, search, results):
        checks = ['permalink_url', 'artwork_url',
                  'title', 'description', 'stream_url',
                  'genre', 'label_name', 'license',
                  'user']
        for r in self.soundcloud_search(search):
            for c in checks:
                if not r.has_key(c) or not r[c]:
                    if c == 'artwork_url':
                        #FIXME Icon needed
                        r[c] = ''
                    elif c == 'user':
                        if not r[c]['username']:
                            r[c]['username'] = ''
                    else:
                        r[c] = ''
            if r.has_key('stream_url') and r['stream_url'] != '':
                r['stream_url'] = r['stream_url'] + '?consumer_key=%s' % API_KEY
            results.append(uri=r['permalink_url'],
                           icon_hint=r['artwork_url'],
                           category=0,
                           mimetype='text/html',
                           title=r['title'],
                           comment=r['description'],
                           dnd_uri=r['permalink_url'],
                           album='',
                           artist=r['user']['username'],
                           genre=r['genre'],
                           label=r['label_name'],
                           license=r['license'],
                           stream=r['stream_url'],
                           result_type=Unity.ResultType.DEFAULT)

    def soundcloud_search(self, search_string):
        query = urllib.quote(search_string)
        data = []
        if query:
            uri = ("%stracks.json?consumer_key=%s&q=%s" % (SEARCH_URI, API_KEY, query))
            print uri
            try:
                response = urllib2.urlopen(uri).read()
                data = simplejson.loads(response)
            except Exception as error:
                print "SoundCloud error : ", error
        return data

if __name__ == '__main__':
    daemon = UnityExtras.dbus_own_name (BUS_NAME, Daemon, None)
    if daemon:
        GLib.unix_signal_add(0, 2, lambda x: daemon.quit(), None)
        daemon.run([])
