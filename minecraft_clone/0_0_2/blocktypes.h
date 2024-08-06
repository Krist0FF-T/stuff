#ifndef BLOCKTYPES_H
#define BLOCKTYPES_H

#include <raylib.h>

static const int blockTypeCount = 10;

#define B_AIR -1
#define B_RED 0
#define B_YELLOW 1
#define B_BLACK 2
#define B_GREEN 3
#define B_STONE 4
#define B_DIRT 5
#define B_SAND 6
#define B_LOG 7
#define B_LEAF 8
#define B_GLASS 9

static const char* blockNames[blockTypeCount] {
    "Red",
    "Yellow",
    "Black",
    "Grass",
    "Stone",
    "Dirt",
    "Sand",
    "Log",
    "Leaf",
    "Glass"
};

static const Color blockColors[blockTypeCount] {
//  [R]   [G]  [B]    [A]

    {255,  20,  0,    255}, // Red
    {255,  230, 0,    255}, // Yellow
    {0,    0,   0,    255}, // Black
    {70,   180, 0,    255}, // Grass
    {128,  128, 128,  255}, // Stone
    {140,  65,  35,   255}, // Dirt
    {194,  178, 128,  255}, // Sand
    {135,  60,  30,   255}, // Log
    {60,   200, 0,    255}, // Leaf
    {0,    127, 255,  127}, // Glass

};

static const int transParentC = 1,
    transparentL[transParentC] = {9};

static bool isTransparent(int type) {
    for(int i=0; i<transParentC; i++) {
        if(type == transparentL[i])
            return 1;
    }
    return 0;
}

#endif