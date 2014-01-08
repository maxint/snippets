#!/usr/bin/env python
# coding: utf-8

import urllib, urllib2, cookielib
import re
import sys
import getpass

rooturl = 'http://doc-server'
username = 'lny1856'

def usage():
    print 'usage: checkin.py [username] <passwd>'

def checkin(passwd):
    # get field name
    cj = cookielib.CookieJar()
    opener = urllib2.build_opener(urllib2.HTTPCookieProcessor(cj))
    opener.addheaders = [
            ('Host', 'doc-server'), 
            ('User-Agent', 'Mozilla/5.0 (Windows NT 6.1; WOW64; rv:26.0) Gecko/20100101 Firefox/26.0'),
            ('Accept', 'text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8'),
            ('Accept-Language', 'en-us,en;q=0.5'),  
            ('Accept-Encoding', 'gzip, deflate'),  
            ('Accept-Charset', 'ISO-8859-1,utf-8;q=0.7,*;q=0.7'),  
            ('Referer', 'http://doc-server/login.asp'),
            ('Connection', 'keep-alive'),
            ]
    res = opener.open(rooturl + '/login.asp')
    fieldname = re.findall(r'id="(userName[^"]*)"', res.read())[0]

    # check in
    login_data = urllib.urlencode({'push_type': '2', fieldname: username, 'password': passwd})
    res = opener.open(rooturl + '/confirm.asp', login_data)
    #import ipdb; ipdb.set_trace()
    if res.geturl() == rooturl + '/confirm.asp':
        errstr = re.findall(r'alert\("([^"]+)', res.read())[0]
        return False, errstr
    else:
        return True, 'Success!'

if __name__ == '__main__':
    if len(sys.argv) == 2:
        passwd = sys.argv[1]
    elif len(sys.argv) == 3:
        username = sys.argv[1]
        passwd = sys.argv[2]
    elif len(sys.argv) != 1:
        usage()
        sys.exit(-1)
    else:
        passwd = None

    count = 0
    print 'Username: {}'.format(username)
    while count < 3:
        if passwd is None:
            passwd = getpass.getpass()
        r, msg = checkin(passwd)
        print msg
        if r:
            break
        else:
            passwd = None
            count += 1

    import time
    time.sleep(1)
