#!/bin/bash
#date
#author
#purpose


g++ ./main.cpp -o ..\jatek\game -std=c++11 -Os -I include/ -I headers/ -L lib/ -lraylib -ffunction-sections -s
# -lwinm -lopengl32 -lgdi32


#END