#! /usr/bin/python3
# -*- coding: utf-8 -*-

# Copyright (C) 2013 David Callé <davidc@framli.eu>
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

from gi.repository import Unity, UnityExtras
from gi.repository import Gio, GLib
import urllib.parse
import urllib.request
import json
import gettext

APP_NAME = 'unity-scope-soundcloud'
LOCAL_PATH = '/usr/share/locale/'
gettext.bindtextdomain(APP_NAME, LOCAL_PATH)
gettext.textdomain(APP_NAME)
_ = gettext.gettext

GROUP_NAME = 'com.canonical.Unity.Scope.Music.Soundcloud'
UNIQUE_PATH = '/com/canonical/unity/scope/music/soundcloud'
SEARCH_URI = 'https://api.soundcloud.com/'
SEARCH_HINT = _('Search SoundCloud')
API_KEY = '398e83f17ec3c5cf945f04772de9f400'
NO_RESULTS_HINT = _('Sorry, there are no SoundCloud tracks that match your search.')
PROVIDER_CREDITS = _('Powered by SoundCloud')
SVG_DIR = '/usr/share/icons/unity-icon-theme/places/svg/'
PROVIDER_ICON = SVG_DIR+'service-soundcloud.svg'
DEFAULT_RESULT_ICON = SVG_DIR+'result-music.svg'
DEFAULT_RESULT_MIMETYPE = 'text/html'
DEFAULT_RESULT_TYPE = Unity.ResultType.DEFAULT

c1 = {'id'      :'songs',
      'name'    :_('Songs'),
      'icon'    :SVG_DIR+'group-songs.svg',
      'renderer':Unity.CategoryRenderer.VERTICAL_TILE}
CATEGORIES = [c1]

FILTERS = []

m1 = {'id'   :'album',
      'type' :'s',
      'field':Unity.SchemaFieldType.REQUIRED}
m2 = {'id'   :'artist',
      'type' :'s',
      'field':Unity.SchemaFieldType.REQUIRED}
m3 = {'id'   :'genre',
      'type' :'s',
      'field':Unity.SchemaFieldType.OPTIONAL}
m4 = {'id'   :'label',
      'type' :'s',
      'field':Unity.SchemaFieldType.OPTIONAL}
m5 = {'id'   :'license',
      'type' :'s',
      'field':Unity.SchemaFieldType.OPTIONAL}
m6 = {'id'   :'stream',
      'type' :'s',
      'field':Unity.SchemaFieldType.OPTIONAL}
m7 = {'id'   :'duration',
      'type' :'s',
      'field':Unity.SchemaFieldType.OPTIONAL}
EXTRA_METADATA = [m1, m2, m3, m4, m5, m6, m7]

def search(search, filters):
    '''
    Any search method returning results as a list of tuples.
    Available tuple fields:
    uri (string)
    icon (string)
    title (string)
    comment (string)
    dnd_uri (string)
    mimetype (string)
    category (int)
    result_type (Unity ResultType)
    extras metadata fields (variant)
    '''
    results = []
    if not search:
        return results
    search = urllib.parse.quote(search)
    uri = "%stracks.json?consumer_key=%s&q=%s&order=hotness&limit=30" % (SEARCH_URI, API_KEY, search)
    print(uri)
    data = []
    try:
        response = urllib.request.urlopen(uri).read()
        data = json.loads(response.decode('utf8'))
    except Exception as error:
        print(error)
    checks = ['permalink_url', 'artwork_url',
              'title', 'description', 'stream_url',
              'genre', 'label_name', 'license',
              'user', 'duration']
    for r in data:
        for c in checks:
            if not c in r or not r[c]:
                if c == 'artwork_url':
                    r[c] = r['user']['avatar_url']
                elif c == 'user':
                    if not r[c]['username']:
                        r[c]['username'] = ''
                else:
                    r[c] = ''
        if 'stream_url' in r and r['stream_url'] != '':
                r['stream_url'] = r['stream_url'] + '?consumer_key=%s' % API_KEY
        results.append({'uri':r['permalink_url'],
                       'icon':r['artwork_url'],
                       'title':r['title'],
                       'comment':r['description'],
                       'album':GLib.Variant('s',''),
                       'artist':GLib.Variant('s',r['user']['username']),
                       'genre':GLib.Variant('s',r['genre']),
                       'label':GLib.Variant('s',r['label_name']),
                       'license':GLib.Variant('s',r['license']),
                       'stream':GLib.Variant('s',r['stream_url']),
                       'duration':GLib.Variant('s',str(r['duration']))})
    return results


# Classes below this point establish communication
# with Unity, you probably shouldn't modify them.


class MySearch (Unity.ScopeSearchBase):
    def __init__(self, search_context):
        super (MySearch, self).__init__()
        self.set_search_context (search_context)

    def do_run (self):
        '''
        Adds results to the model
        '''
        try:
            result_set = self.search_context.result_set
            for i in search(self.search_context.search_query,
                            self.search_context.filter_state):
                if not 'uri' in i or not i['uri'] or i['uri'] == '':
                    continue
                if not 'icon' in i or not i['icon'] or i['icon'] == '':
                    i['icon'] = DEFAULT_RESULT_ICON
                if not 'mimetype' in i or not i['mimetype'] or i['mimetype'] == '':
                    i['mimetype'] = DEFAULT_RESULT_MIMETYPE
                if not 'result_type' in i or not i['result_type'] or i['result_type'] == '':
                    i['result_type'] = DEFAULT_RESULT_TYPE
                if not 'category' in i or not i['category'] or i['category'] == '':
                    i['category'] = 0
                if not 'title' in i or not i['title']:
                    i['title'] = ''
                if not 'comment' in i or not i['comment']:
                    i['comment'] = ''
                if not 'dnd_uri' in i or not i['dnd_uri'] or i['dnd_uri'] == '':
                    i['dnd_uri'] = i['uri']
                result_set.add_result(**i)
        except Exception as error:
            print (error)

class Preview (Unity.ResultPreviewer):

    def do_run(self):
        title = self.result.title.strip()
        stream = self.result.metadata['stream'].get_string()
        duration = int(self.result.metadata['duration'].get_string())
        author = self.result.metadata['artist'].get_string()
        description = self.result.comment.strip()
        image = self.result.icon_hint.replace('large.jpg', 'original.jpg')
        preview = Unity.MusicPreview.new(title, description, None)
        if stream != '':
            t = Unity.TrackMetadata.full(stream, 1, title, author, '', duration / 1000)
            preview.add_track(t)
        preview.props.subtitle = author
        preview.props.image_source_uri = image
        icon = Gio.FileIcon.new (Gio.file_new_for_path(PROVIDER_ICON))
        view_action = Unity.PreviewAction.new("view", _("SoundCloud"), icon)
        preview.add_action(view_action)
        return preview

class Scope (Unity.AbstractScope):
    def __init__(self):
        Unity.AbstractScope.__init__(self)

    def do_get_search_hint (self):
        return SEARCH_HINT

    def do_get_schema (self):
        '''
        Adds specific metadata fields
        '''
        schema = Unity.Schema.new ()
        if EXTRA_METADATA:
            for m in EXTRA_METADATA:
                schema.add_field(m['id'], m['type'], m['field'])
        #FIXME should be REQUIRED for credits
        schema.add_field('provider_credits', 's', Unity.SchemaFieldType.OPTIONAL)
        return schema

    def do_get_categories (self):
        '''
        Adds categories
        '''
        cs = Unity.CategorySet.new ()
        if CATEGORIES:
            for c in CATEGORIES:
                cat = Unity.Category.new (c['id'], c['name'],
                                          Gio.ThemedIcon.new(c['icon']),
                                          c['renderer'])
                cs.add (cat)
        return cs

    def do_get_filters (self):
        '''
        Adds filters
        '''
        fs = Unity.FilterSet.new ()
#        if FILTERS:
#            
        return fs

    def do_get_group_name (self):
        return GROUP_NAME

    def do_get_unique_name (self):
        return UNIQUE_PATH

    def do_create_search_for_query (self, search_context):
        se = MySearch (search_context)
        return se

    def do_create_previewer(self, result, metadata):
        rp = Preview()
        rp.set_scope_result(result)
        rp.set_search_metadata(metadata)
        return rp

def load_scope():
    return Scope()
