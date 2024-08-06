@echo off
g++.exe .\jump.cpp -o jump -std=c++11 -O3 -I ../include/ -L ../lib/ -lopengl32 -lraylib -lgdi32 -lwinmm

pause