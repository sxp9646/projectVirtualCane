#!/bin/bash
gcc -std=c99 sound_library.c
gcc -std=c99 openal_test.c sound_library.o libopenal.so -o openal_test