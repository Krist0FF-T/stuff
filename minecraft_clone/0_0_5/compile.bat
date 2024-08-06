@echo off
g++.exe .\main.cpp -o ..\jatek\game -std=c++11 -O3 -I include/ -L lib/ -lopengl32 -lraylib -lgdi32 -lwinmm

pause