#!/bin/bash

ROOT_DIR=`cd ../ && pwd`
YEAR=`date +%Y`
LICENSE_HEAD=`sed "s/<year>/$YEAR/g" $ROOT_DIR/data/LICENSE_HEAD`

for file in $ROOT_DIR/src/*; do
    [ -e "$file" ] || continue

    FILE_CONTENTS=`cat $file`
    if [[ "$FILE_CONTENTS" != *"Copyright (C)"* ]]; then
        echo -e "$LICENSE_HEAD\n" | cat - $file > temp && mv temp $file
        printf "Applied header to $file\n"
    fi
done
