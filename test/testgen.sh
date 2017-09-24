#!/usr/bin/env bash

for i in $(seq 1 $1)
do
    amount=$(jot -r 1 1000.00 40000.00)
    rate=$(jot -r 1 1.00 15.00)
    term=$(jot -r 1 5 25)
    echo "$i $amount $rate $term"
done
