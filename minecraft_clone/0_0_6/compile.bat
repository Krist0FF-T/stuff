@echo off
g++.exe .\main.cpp -o ..\jatek\game -std=c++11 -Os -I include/ -I headers/ -L lib/ -lopengl32 -lraylib -lgdi32 -lwinmm -ffunction-sections -s
pause