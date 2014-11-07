#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim:fenc=utf-8
#
# Copyright (C) 2014 maxint <NOT_SPAM_lnychina@gmail.com>
#
# Distributed under terms of the MIT license.

"""
Generate compare result between ground true and tracking result for
Object Tracking project.

v0.0.1

"""

import subprocess
import re
import glob
import os

quiet = False


class Rect():
    def __init__(self, left, top, right, bottom):
        self.xmin = left
        self.ymin = top
        self.xmax = right
        self.ymax = bottom

    def formCenterSize(x, y, width, height):
        return Rect(x - width / 2,
                    y - height / 2,
                    x + width / 2,
                    y + height / 2)

    def cx(self):
        return (self.xmin + self.xmax) / 2

    def cy(self):
        return (self.ymin + self.ymax) / 2

    def center(self):
        return self.cx(), self.cy()

    def width(self):
        return self.xmax - self.xmin

    def height(self):
        return self.ymax - self.ymin

    def size(self):
        return self.width(), self.height()

    def area(self):
        return self.width() * self.height()

    def empty(self):
        return self.xmin == self.xmax or self.ymin == self.ymax

    def array(self):
        return self.xmin, self.ymin, self.xmax, self.ymax

    def zero(self):
        return all(map(lambda x: x == 0, self.array()))

    def __and__(rc1, rc2):
        return Rect(max(rc1.xmin, rc2.xmin),
                    max(rc1.ymin, rc2.ymin),
                    min(rc1.xmax, rc2.xmax),
                    min(rc1.ymax, rc2.ymax))

    def __or__(rc1, rc2):
        return Rect(min(rc1.xmin, rc2.xmin),
                    min(rc1.ymin, rc2.ymin),
                    max(rc1.xmax, rc2.xmax),
                    max(rc1.ymax, rc2.ymax))

    def __str__(self):
        return 'Rect ({})'.format(','.join(map(str, self.array())))


def valid_rect(rc):
    return rc.zero() or not rc.empty()


def parse_result(path):
    patt = re.compile(r'^(-?\d+),\s*(-?\d+),\s*(-?\d+),\s*(-?\d+)$')
    data = []
    with open(path) as f:
        for line in f:
            m = patt.match(line)
            assert m, line
            g = m.groups()
            rect = Rect(*map(int, [g[0], g[1], g[2], g[3]]))
            assert valid_rect(rect), line
            data.append(rect)
    return data


def parse_marks(path):
    patt = re.compile(r'^(\d+)\s+\S*\s+\((\d+),(\d+),(\d+),(\d+)\)')
    data = []
    with open(path) as f:
        for line in f:
            m = patt.match(line)
            assert m, line
            g = m.groups()
            idx = int(g[0])
            rect = Rect(*map(int, [g[1], g[2], g[3], g[4]]))
            assert valid_rect(rect), line
            if idx == len(data):
                data.append(rect)
            elif idx == len(data) - 1:
                if not quiet:
                    print '[W] duplicate mark: ', line
                data[idx] = rect
            else:
                raise Exception('Wrong data: ' + line)
    return data


def overlap(rc1, rc2):
    if rc1.zero() and rc2.zero():
        return 1.0
    else:
        return float((rc1 & rc2).area()) / max(1, (rc1 | rc2).area())


def overlap_only_pos(rc1, rc2):
    if rc1.zero() and rc2.zero():
        return 1.0
    else:
        rc1 = Rect.formCenterSize(rc1.center(), rc2.size())
        return float((rc1 & rc2).area()) / max(1, (rc1 | rc2).area())


def compare(marks, result, overlapfn=overlap):
    assert len(marks) == len(result)
    return [(rc1, rc2, overlapfn(rc1, rc2)) for rc1, rc2 in zip(marks, result)]


def save_compare(path, results, overlapThreshold):
    passed = [o > overlapThreshold for _, _, o in results]
    tpassed = passed.count(True)
    with open(path, 'wt') as f:
        count = len(results)
        pratio = float(tpassed) / max(1, count)
        f.write('TOTAL frames: {}\n'.format(count))
        f.write('Passed frames: {}\n'.format(tpassed))
        f.write('Passed ratio: {}\n'.format(pratio))
        f.write('#NO.({}), #Ground True,,,, #Tracking Result,,,, #Overlap, ' +
                '#Passed(>{})\n'.format(count, overlapThreshold))

        def rcout(rc):
            return ','.join(map(str, rc.array()))

        for idx, (passed, (rc1, rc2, o)) in enumerate(zip(passed, results)):
            f.write('{}, {}, {}, '.format(idx, rcout(rc1), rcout(rc2)) +
                    '{}, {}\n'.format(o, 1 if passed else 0))

        return (path, count, tpassed, pratio)


def save_summary(spath, results):
    with open(spath, 'wt') as f:
        f.write('#NO.({}), #Path, '.format(len(results)) +
                '#Total, #Passed, #Ratio\n')
        tcount = 0
        tpassed = 0
        for idx, (path, count, passed, ratio) in enumerate(results):
            f.write('{}, {}, {}, {}, {}\n'.format(idx,
                                                  path,
                                                  count,
                                                  passed,
                                                  ratio))
            tcount += count
            tpassed += passed

        pratio = float(tpassed) / max(1, tcount)
        f.write('TOTAL frames: {}\n'.format(tcount))
        f.write('Passed frames: {}\n'.format(tpassed))
        f.write('Passed ratio: {}\n'.format(pratio))


def get_markpaths(dpath):
    return glob.glob(os.path.join(dpath, '*_fingerMark.txt'))


def main(dpath, overlapfn, overlapThreshold=0.5):
    markpaths = get_markpaths(dpath)
    spath = os.path.join(dpath, 'summary.csv')
    tresults = []
    for mpath in markpaths:
        mpath_noext = os.path.splitext(mpath)[0]
        rpath = mpath_noext + '_res.txt'
        cpath = mpath_noext + '_res.csv'
        marks = parse_marks(mpath)
        result = parse_result(rpath)
        cresult = compare(marks, result, overlapfn)
        res = save_compare(cpath, cresult, overlapThreshold)
        tresults.append(res)

    save_summary(spath, tresults)


def copy_from_mobile(src, dst):
    cmd = 'adb pull {} {}'.format(src, dst)
    subprocess.check_call(cmd)


def get_unused_path(path):
    i = 0
    tpath = path
    while os.path.exists(tpath):
        tpath = path + str(i)
        i += 1
    return tpath


def test():
    assert Rect(0, 0, 0, 0).zero()
    assert not Rect(1, 0, 0, 0).zero()


if __name__ == '__main__':
    import argparse

    def restricted_float(start, end):
        def impl(x):
            x = float(x)
            if x < start or x > end:
                msg = "%r not in range [%r, %r]".format(x, start, end)
                raise argparse.ArgumentTypeError(msg)
            return x
        return impl

    mdir = '/sdcard/Arcsoft/com.arcsoft.objecttrackingload/ConfigFile'

    parser = argparse.ArgumentParser(description='OT result report')
    parser.add_argument('--nocopy', '-n', action='store_true', default=False,
                        help='do not copy data from mobile')
    parser.add_argument('--source', '-s', default=mdir,
                        help='source data path in android device')
    parser.add_argument('--overlap',
                        choices=['rect', 'pos'], default='pos',
                        help='overlap function')
    parser.add_argument('--threshold', '-t',
                        type=restricted_float(0.1, 0.9), default=0.5,
                        help='overlap threshold')
    parser.add_argument('--quiet', action='store_true', default=False,
                        help='no warnning')
    args = parser.parse_args()

    datadir = 'data'
    quiet = args.quiet
    if not args.nocopy:
        mdir = '/sdcard/Arcsoft/com.arcsoft.objecttrackingload/ConfigFile'
        datadir = get_unused_path(datadir)
        copy_from_mobile(args.source, datadir)

    main(datadir,
         dict(rect=overlap, pos=overlap_only_pos)[args.overlap],
         args.threshold)

    os.system('pause')
    print 'Done!'
