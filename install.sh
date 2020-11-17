#!/bin/sh

sudo gcc -O2 -Wall -I./include/ -I./usr/local/include/ $(pkg-config --cflags vte-2.91) ./src/GeminiTerm.c -o /bin/gemini $(pkg-config --libs vte-2.91)

sudo mkdir /usr/share/pixmaps/gemini/

sudo cp resource/*.png /usr/share/pixmaps/gemini/

sudo cp gemini.desktop /usr/share/applications/
