@echo off

g++ main.cpp -o main -std=c++11 -O3 -I ../include/ -L ../lib/ -lopengl32 -lraylib -lgdi32 -lwinmm

pause