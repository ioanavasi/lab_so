#!/bin/bash

while read linie
do
    text=$(echo "$linie" | grep -E "^[A-Z]{1,1}[A-Za-z0-9]+[ ,]*[\.]$" | grep -v "si[\ ]*," | grep -v "n[pb]")

    echo "$text"
done