# -*- coding: utf-8 -*-

from gi.repository import Dee
from unittest import TestCase, main
from src import unity_soundcloud_daemon as scope

d = scope.Daemon()
searches = ['submarine']
mock_data = {'should_pass': 'file:tests/data/mock_soundcloud_pass#',
             'should_fail': 'file:tests/data/mock_soundcloud_fail#'}

class TestSoundcloud(TestCase):

    def test_search(self):
        scope.SEARCH_URI = mock_data['should_pass']
        model = Dee.SequenceModel.new()
        model.set_schema('s', 's', 'u', 'u', 's', 's', 's',
                         's', 's', 's', 's', 's', 's', 's')
        model.set_column_names('uri', 'icon_hint', 'category', 'result_type',
                               'mimetype', 'title', 'comment', 'dnd_uri',
                               'album', 'artist', 'genre', 'label', 'license',
                               'stream')
        scope.Daemon.update_results_model(d, searches[0], model)
        self.assertEqual(model[0][0],
                         'http://soundcloud.com/vibecatt/echos')
        self.assertEqual(model[0][1],
                         '')
        self.assertEqual(model[0][5],
                         'Echos')
        self.assertEqual(model[0][6],
                         '')
        self.assertEqual(model[1][0],
                         'http://soundcloud.com/vibecatt/ganymede')
        self.assertEqual(model[1][1],
                         '')
        self.assertEqual(model[1][5],
                         'Ganymede')
        self.assertEqual(model[1][6],
                         '')

    def test_failing_search(self):
        scope.SEARCH_URI = mock_data['should_fail']
        model = Dee.SequenceModel.new()
        model.set_schema('s', 's', 'u', 'u', 's', 's', 's',
                         's', 's', 's', 's', 's', 's', 's')
        model.set_column_names('uri', 'icon_hint', 'category', 'result_type',
                               'mimetype', 'title', 'comment', 'dnd_uri',
                               'album', 'artist', 'genre', 'label', 'license',
                               'stream')
        scope.Daemon.update_results_model(d, searches[0], model)
        self.assertEqual(len(model), 0)

if __name__ == '__main__':
    main()
