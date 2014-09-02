#!/bin/bash

thread=$(grep -c ^processor /proc/cpuinfo)
pwd=$(pwd)

mkdir -p test

$(pwd)/mnist -t $thread -k 100 > test/mnist-k100.txt
$(pwd)/validation -t $thread > test/validation-new.txt
#$(pwd)/cross-validation -t $thread -f 5 -d 10000 > test/cross-validation-f5.txt
#$(pwd)/cross-validation -t $thread -f 10 -d 5000 > test/cross-validation-f10.txt
