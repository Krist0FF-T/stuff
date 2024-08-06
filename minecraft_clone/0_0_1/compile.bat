@echo off
gcc main.cpp -o game -O1 -I include/ -L lib/ -lopengl32 -lraylib -lgdi32 -lwinmm -mwindows