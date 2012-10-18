#!/bin/bash
if [ "$#" == "2" ] ; then
    echo "== BEGIN"
    mkdir -p $2
    for i in $(find $1 -regex ".*\.\(h\)") ; do
        echo "* Processing $i"
        newfile=$2${i:${#1}}
        newdir=$(dirname $newfile)
        if [ ! -d "$newdir" ]; then
            mkdir -p $newdir
        fi
        sed 's/\/\/\(.*\)$/\/*\1*\//p' $i > $newfile
        echo "* Processing $i Done"
    done
    for i in $(find $1 -regex ".*\.\(cpp\|c\)") ; do
        echo "* Processing $i"
        newfile=$2${i:${#1}}
        newdir=$(dirname $newfile)
        if [ ! -d "$newdir" ]; then
            mkdir -p $newdir
        fi
        echo "
#pragma section INIT=HOST_3RD_INIT, attr=DATA
#pragma section CONST=HOST_3RD_CONST, attr=CONST
#pragma section CODE=HOST_3RD_CODE, attr=CODE" \
    | cat - $i | sed 's/\/\/\(.*\)$/\/*\1*\//p' > $newfile
        echo "* Processing $i Done"
    done
    echo "== DONE"
else
    echo usage: $0 [srcdir] [dstdir]
fi
