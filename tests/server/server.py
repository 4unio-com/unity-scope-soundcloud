#!/usr/bin/env python3

import http.server
import os
import sys
import urllib.parse

def read_file(path):
    file = os.path.join(os.path.dirname(__file__), path)
    if os.path.isfile(file):
        with open(file, 'rb') as fp:
            content = fp.read()
    else:
        content = b''

    return content

class MyRequestHandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        sys.stderr.write("GET: %s\n" % self.path)
        sys.stderr.flush()

        url = urllib.parse.urlparse(self.path)
        query = dict(urllib.parse.parse_qsl(url.query))
        if url.path == '/tracks.json':
            self.handle_track_search(query)
        elif url.path == '/me/activities/tracks/affiliated.json':
            self.handle_activity(query)
        else:
            self.send_response(404)
            self.send_header("Content-type", "text/html")
            self.end_headers()
            self.wfile.write(b'ERROR')

    def handle_track_search(self, query):
        self.send_response(200)
        self.send_header("Content-type", "application/json")
        self.end_headers()
        if query.get('q'):
            self.wfile.write(
                read_file('search/{}.json'.format(query['q'])))
        else:
            self.wfile.write(
                read_file('genre/{}.json'.format(query['genres'])))

    def handle_activity(self, query):
        self.send_response(200)
        self.send_header("Content-type", "application/json")
        self.end_headers()
        self.wfile.write(read_file('activity/tracks.json'))

def main(argv):
    server = http.server.HTTPServer(("127.0.0.1", 0), MyRequestHandler)
    sys.stdout.write('%d\n' % server.server_address[1])
    sys.stdout.flush()
    server.serve_forever()

if __name__ == "__main__":
    sys.exit(main(sys.argv))


