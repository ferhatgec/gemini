#!/bin/sh

gcc -O2 -Wall $(pkg-config --cflags vte-2.91) GeminiTerm.c -o gemini $(pkg-config --libs vte-2.91)
