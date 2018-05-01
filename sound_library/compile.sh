#!/bin/bash
gcc -std=c99 -lm sound_library.c -c
gcc -std=c99 -lm openal_test.c sound_library.o libopenal.so -o openal_test
