#!/bin/bash

thread=$(grep -c ^processor /proc/cpuinfo)
pwd=$(pwd)

mkdir -p test

$(pwd)/mnist -t $thread -k 1 > test/mnist-k1.txt
$(pwd)/mnist -t $thread -k 2 > test/mnist-k2.txt
$(pwd)/mnist -t $thread -k 3 > test/mnist-k3.txt
$(pwd)/mnist -t $thread -k 4 > test/mnist-k4.txt
$(pwd)/mnist -t $thread -k 5 > test/mnist-k5.txt
$(pwd)/validation -t $thread > test/validation.txt
$(pwd)/cross-validation -t $thread -f 5 -d 10000 > test/cross-validation-f5.txt
$(pwd)/cross-validation -t $thread -f 10 -d 5000 > test/cross-validation-f10.txt
