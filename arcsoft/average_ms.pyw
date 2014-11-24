#! /usr/bin/env python
# coding:utf-8
# Copyright (C) 2014 maxint <NOT_SPAM_lnychina@gmail.com>
# Distributed under terms of the MIT license.

"""
Calculate average ms from adb log in clip board.
e.g.
XXX: jni/ot-jni.cpp:ASOT_Tracking cost time 17.011ms
XXX: jni/ot-jni.cpp:ASOT_Tracking cost time 15.649ms

Usage:

1. Copy adb time log to system clipboard (Ctrl+C).
2. Run this program, and show the averge time consume.
3. Paste to Excel. (Ctrl+V).

"""

from Tkinter import Tk
import tkMessageBox
import re

NUM_RE = re.compile(r'.*\s([.0-9]+)\s?ms$')


def extract_time(line):
    return float(NUM_RE.match(line).group(1))


def totimes(log):
    if re.match(r'^[.0-9\n]+$', log):
        return map(float, log.splitlines(False))
    else:
        return map(extract_time, log.splitlines(False))

# copy from system clipboard
r = Tk()
r.withdraw()
log = r.clipboard_get()

try:
    times = totimes(log)

    avg_time = reduce(lambda x, y: x + y, times) / len(times)
    msg = '''Averge time: {}ms.
Note: Formated times is ready for Excel in the clipboard.
Ctrl+V in Excel to get them.'''.format(avg_time)
    tkMessageBox.showinfo('Result', msg)

    # copy to system clipboard
    log = '\n'.join(map(str, times))
    log = 'Averge: ' + str(avg_time) + '\n' + log
    r.clipboard_clear()
    r.clipboard_append(log)
except:
    tkMessageBox.showwarning('Warning', 'Unknown format of clipboard data')

r.destroy()

print 'Done!'
