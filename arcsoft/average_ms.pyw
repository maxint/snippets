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


def second_max(times):
    return times[0] if len(times) == 1 else sorted(times)[-2]


def split_times_by_name(times):
    keys = set(k for k, _ in times)
    k_times = dict((k, list()) for k in keys)
    for k, v in times:
        k_times[k].append(v)
    results = [(k, times, get_average_time(times), max(times)) for k, times in k_times.items()]

    import operator
    return sorted(results, key=operator.itemgetter(2), reverse=True)


def compose_lite_report(items):
    assert len(items) > 0

    msg = 'Average Time List:\n\n'
    msg += '\n'.join(map(lambda (k, times, avg_t, max_t): '[{}] {:.2f}/{:.2f}/{:.2f} #{}'.format(k, avg_t, min(times), max_t, len(times)), items))
    msg += '\n\n'
    msg += 'Note: Formatted times is ready for Excel in the clipboard.\n'
    msg += 'Ctrl+V in Excel to get them.'
    return msg


def compose_full_report(items):
    if not items:
        return

    msg = '\t'.join('{} (avg:{}/max:{})'.format(t[0], t[2], t[3]) for t in items)
    max_lines = max(len(t[1]) for t in items)
    times_v = [t[1] for t in items]
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
        import time
        time_str = time.strftime('%a, %d %b %Y %H:%M:%S')
        tkMessageBox.showinfo('Average Time Summary (%s)' % time_str , compose_lite_report(items))
    else:
        tkMessageBox.showwarning('No Valid Data is Found', '''No line in valid format is found in clipboard!

Valid formats for example:

XXX: I/PRF(7410): CCTracker::apply : 6.008583ms
XXX: D/OT(7410): CCTracker::learn
XXX: V/JNI(7410): ASOT_Tracking cost time 17.472ms
...

''')

    # copy to system clipboard
    full_report = compose_full_report(items)
    r.clipboard_clear()
    r.clipboard_append(full_report)

    r.destroy()

if __name__ == '__main__':
    try:
        main()
    except Exception, e:
        tkMessageBox.showerror('Python Exception', str(e))
    print 'Done!'
