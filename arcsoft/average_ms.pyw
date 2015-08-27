# coding: utf-8
# author: maxint <NOT_SPAM_lnychina@gmail.com>
# version: 1.0

"""
Calculate average ms from adb log in clip board.
e.g.
08-27 14:58:50.087: I/PRF(7410): CCTracker::apply : 6.008583ms
08-27 14:58:50.092: D/OT(7410): CCTracker::learn
08-27 14:58:50.096: V/JNI(7410): jni/ot-jni.cpp:ASOT_Tracking cost time 17.472ms


Usage:

1. Copy adb time log to system clipboard (Ctrl+C).
2. Run this program, and show the average time consume.
3. Paste to Excel. (Ctrl+V).

"""

from Tkinter import Tk
import tkMessageBox
import re

NAME_NUM_RE = re.compile(r'.*\(\d+\):\s(\S+)\s.*\s+([.0-9]+)\s?ms$')


def to_times(log):
    times = list()
    for line in log.splitlines(False):
        m = NAME_NUM_RE.match(line)
        if m:
            times.append((m.group(1), float(m.group(2))))
    return times


def get_average_time(times):
    return reduce(lambda x, y: x + y, times) / len(times)


def split_times_by_name(times):
    keys = set(k for k, _ in times)
    k_times = dict((k, list()) for k in keys)
    for k, v in times:
        k_times[k].append(v)

    return [(k, times, get_average_time(times)) for k, times in k_times.iteritems()]


def compose_lite_report(items):
    assert len(items) > 0

    msg = 'Average Time List:\n\n'
    msg += '\n'.join(map(lambda (k, _, avg): '[{}] {}'.format(k, avg), items))
    msg += '\n\n'
    msg += 'Note: Formatted times is ready for Excel in the clipboard.\n'
    msg += 'Ctrl+V in Excel to get them.'
    return msg


def compose_full_report(items):
    if not items:
        return

    msg = '\t'.join('{} ({})'.format(k, avg) for k, _, avg in items)
    max_lines = max(len(times) for _, times, _ in items)
    times_v = [times for _, times, _ in items]
    for i in xrange(max_lines):
        msg += '\n' + '\t'.join(str(times[i]) if i < len(times) else '' for times in times_v)
    return msg


def main():
    # copy from system clipboard
    r = Tk()
    r.withdraw()
    log = r.clipboard_get()

    items = split_times_by_name(to_times(log))

    # show summary
    if items:
        tkMessageBox.showinfo('Average Time Summary', compose_lite_report(items))
    else:
        tkMessageBox.showwarning('No Valid Data is Found', '''No line in valid format is found in clipboard!

Valid formats for example:

XXX: I/PRF(7410): CCTracker::apply : 6.008583ms
XXX: D/OT(7410): CCTracker::learn
XXX: V/JNI(7410): ASOT_Tracking cost time 17.472ms
...

''')

    # copy to system clipboard
    r.clipboard_clear()
    r.clipboard_append(compose_full_report(items))

    r.destroy()

if __name__ == '__main__':
    main()
    print 'Done!'
