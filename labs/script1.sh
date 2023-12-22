#!/bin/bash

if [ "$#" -le 3 ]
	then
	 echo "Invalid no of arguments"
else
	 echo "Correct"
fi

count=0
index=0
for arg in "$@"
	do
	if [ "$index" -eq 0 ] || [ "$index" -eq 1 ]
		then 
		continue
	fi
	if [ "$arg" -gt 10]
		then
		 count=$(count + 1)
	fi
	index=$(index + 1)
	done

sum=0
index1=0
for arg in "$@"
	do
	if [ "$index1" -eq 0 ] || [ "$index1" -eq 1 ]
		then 
		continue
	fi
	if [ `echo "$arg" | grep -E "^[0-9]+$"` ] 
		then
		 sum=$(sum + arg)
	fi
	index1=$(index1 + 1)
	done 

digit_ct=$(echo "$sum" | wc -m)

file="$1"

if [ -f "$file" ]
	 then
    	 echo "$count $sum $digit_ct" > "$file"
fi

dir="$2"
for file in "$dir"/*.txt
do
    if [ -f "$file" ]
	then
        echo "Content of $file:"
        cat "$file"
        echo "----------------------"
    fi
done
