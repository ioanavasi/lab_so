#!/bin/bash

count=0
count2=0

for entry in "$1"/*
do
    if [ -f "$entry" ]
        then
        res=$(cat "$entry" | grep -E "^[A-Z]{1,1}[A-Za-z0-9]+[ ,]*[\.]$" | grep -v "si[\ ]*," | grep -v "n[pb]")
        if [ ! -z "$res" ]
            then
            echo "$entry" >> "$2"
        fi

    elif [ -L "$entry" ]
        then
        count=$((count + 1))

    elif [ -d "$entry" ]
        then
        count2=$(bash "$0" "$entry" "$2")
        count=$((count + count2))
    fi
done

echo "$count"
