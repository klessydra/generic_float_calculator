#!/bin/bash

echo BUILDING FP_GM.C

if [ -e build ]
then
	cd build
else
	mkdir build
	cd build
fi

if [ ! -e "../result" ]
then
	mkdir ../result
fi

gcc -std=c99 -g -I ../libs/include -c ../libs/src/error_check.c ../libs/src/float_transform.c ../libs/src/round.c ../libs/src/parser.c ../libs/src/auxiliary.c
ar 	cr libs.a *.o
gcc -std=c99 -g ../app/fp_gm.c -lm  libs.a -I ../libs/include -o fp_gm.out
