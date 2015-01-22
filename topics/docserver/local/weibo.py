# coding: utf-8
__author__ = 'maxint'

import urllib
from requests_oauthlib import OAuth2Session
from oauthlib.oauth2 import WebApplicationClient
import json
import logging

log = logging.getLogger(__file__)

API_HOST = 'https://api.weibo.com/'


class Weibo():
    def __init__(self, client_id, client_secret, access_token=None):
        token = {}
        if access_token:
            token['access_token'] = access_token
        self.oauth = OAuth2Session(redirect_uri='http://127.0.0.1:8000/response',
                                   token=token,
                                   client=WebApplicationClient(client_id=client_id,
                                                               token=token,
                                                               default_token_placement='query'))
        self.client_secret = client_secret

    def url(self, subpath):
        return API_HOST + subpath

    def authorize(self):
        url, _ = self.oauth.authorization_url(self.url('oauth2/authorize'))
        import webbrowser
        webbrowser.open(url)

    def access_token(self, code):
        self.oauth.fetch_token(token_url=self.url('oauth2/access_token'),
                               code=code,
                               client_secret=self.client_secret)

    def store(self, filename):
        s = json.dumps(dict(
            client_id=self.oauth.client_id,
            client_secret=self.client_secret,
            access_token=self.oauth._client.access_token,
        ))
        open(filename, 'wt').write(s)

    def request(self, method, subpath, **kwargs):
        req = self.oauth.request(method, self.url(subpath), headers=kwargs)
        if req.ok:
            return json.loads(req.text)
        else:
            log.error(req.text)


    def get(self, subpath, **kwargs):
        return self.request('GET', subpath, **kwargs)

    def post(self, subpath, **kwargs):
        return self.request('POST', subpath, **kwargs)

    def comments_mentions(self):
        return self.get('2/comments/mentions.json')

    def comments_create(self, comment, id):
        return self.post('comments/create',
                         comment=comment, id=id)

    def statuses_mentions(self):
        return self.get('2/statuses/mentions.json')

def load(filename):
    d = json.load(open(filename, 'rt'))
    return Weibo(d['client_id'], d['client_secret'], d['access_token'])