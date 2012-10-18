#!/bin/bash
if [ "$#" == "2" ] ; then
    echo "== BEGIN"
    mkdir -p $2
    for i in $(find $1 -type f) ; do
        filename=$(basename "$i")
        newfile=$2${i:${#1}}
        newdir=$(dirname $newfile)
        if [ ! -d "$newdir" ]; then
            mkdir -p $newdir
        fi
        if [[ "${filename##*.}" =~ (cpp|c) ]]; then
            echo "* Processing $i"
            echo \
"#pragma section INIT=HOST_3RD_INIT, attr=DATA
#pragma section CONST=HOST_3RD_CONST, attr=CONST
#pragma section CODE=HOST_3RD_CODE, attr=CODE
" | cat - $i > $newfile
            echo "* Processing $i DONE"
        else
            echo "* Copying $i to $newfile"
            cp $i $newfile
            echo "* Copying $i to $newfile DONE"
        fi
    done
    echo "== DONE"
else
    echo usage: $0 [srcdir] [dstdir]
fi
