#!/bin/sh

gcc -O2 -Wall -I./include/ -I./usr/local/include/ $(pkg-config --cflags vte-2.91) ./src/GeminiTerm.c -o gemini $(pkg-config --libs vte-2.91)

mkdir /usr/share/pixmaps/gemini/

cp resource/*.png /usr/share/pixmaps/gemini/

cp gemini.desktop /usr/share/applications/
