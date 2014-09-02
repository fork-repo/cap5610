#!/bin/bash

thread=$(grep -c ^processor /proc/cpuinfo)
pwd=$(pwd)

mkdir -p test

$(pwd)/mnist -t $thread -k 5 > test/mnist-k5.txt
$(pwd)/mnist -t $thread -k 10 > test/mnist-k10.txt
$(pwd)/mnist -t $thread -k 15 > test/mnist-k15.txt
$(pwd)/mnist -t $thread -k 20 > test/mnist-k20.txt
$(pwd)/mnist -t $thread -k 25 > test/mnist-k25.txt
$(pwd)/validation -t $thread > test/validation.txt
$(pwd)/cross-validation -t $thread -f 5 -d 10000 > test/cross-validation-f5.txt
$(pwd)/cross-validation -t $thread -f 10 -d 5000 > test/cross-validation-f10.txt
