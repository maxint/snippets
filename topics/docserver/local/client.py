# coding: utf-8
__author__ = 'maxint'

import BaseHTTPServer
import weibo
import urlparse
import pprint

CACHED_FILE = '.token.json'
PP = pprint.PrettyPrinter()


def pprint(m):
    PP.pprint(m)


class Main():
    def __init__(self):
        try:
            self.wb = weibo.load(CACHED_FILE)
        except:
            self.wb = weibo.Weibo('3150443457', 'e9a369d1575e399cb1d06c0a79685e67')
            self.wb.authorize()

    def do(self):
        mentions = self.wb.statuses_mentions()
        status = mentions['statuses']
        pprint(mentions)
        for s in status:
            self.wb.comments_create('OK', s['idstr'])


class ServerRequestHandler(BaseHTTPServer.BaseHTTPRequestHandler):
    def echoHTML(self, content):
        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.end_headers()
        self.wfile.write(content)
        self.wfile.close()

    def do_GET(self):
        qs = urlparse.parse_qs(urlparse.urlsplit(self.path).query)
        if 'code' in qs:
            main.wb.access_token(qs['code'][0])
            main.wb.store(CACHED_FILE)
            main.do()
            self.echoHTML('OK')
            httpd.server_close()

if __name__ == '__main__':
    host = '127.0.0.1'
    port = 8000
    main = Main()
    if main.wb.oauth.authorized:
        main.do()
    else:
        httpd = BaseHTTPServer.HTTPServer((host, port), ServerRequestHandler)
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            pass
        httpd.server_close()