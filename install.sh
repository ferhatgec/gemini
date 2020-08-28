#!/bin/sh

gcc -O2 -Wall -I./usr/local/include/ $(pkg-config --cflags vte-2.91) GeminiTerm.c -o /bin/gemini $(pkg-config --libs vte-2.91)
