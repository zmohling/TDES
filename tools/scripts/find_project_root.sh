#!/bin/bash

find_root () {
    counter=0
    while [ ! -f ./.base ] && [ ! -d ./root ]
    do
        cd ..
        counter=$(( $counter + 1 ))
    done

    echo $(pwd)
}

find_root

