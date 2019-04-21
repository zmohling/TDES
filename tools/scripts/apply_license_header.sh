#!/bin/bash

apply () {
    ROOT_DIR=$(cd ../../ && pwd)
    YEAR=`date +%Y`
    LICENSE_HEAD=`sed "s/<year>/$YEAR/g" $ROOT_DIR/data/LICENSE_HEAD`

    file=$1
    [ -e "$file" ] || continue

    FILE_CONTENTS=`cat $file`
    if [[ "$FILE_CONTENTS" != *"Copyright (C)"* ]]; then
        echo -e "$LICENSE_HEAD\n" | cat - $file > temp && mv temp $file
        printf "Applied header to $file\n"
    fi
}

ROOT_DIR=$(cd ../../ && pwd)

export -f apply
find $ROOT_DIR/src/ -regex ".+\.\(c\|cc\|cpp\|h\|hpp\)$" -exec bash -c 'apply "$0"' {} \;
