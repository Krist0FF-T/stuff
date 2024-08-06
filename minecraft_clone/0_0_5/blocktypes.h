#ifndef BLOCKTYPES_H
#define BLOCKTYPES_H

#include <raylib.h>

/*
                  BBBBBBBBBBB
             BBBBB           BBBBB
        BBBBB                     BBBBB
      BB                               BB
    BB                                   BB
    BB                                   BB
  BB                                       BB
  BB                                       BB
BB                                           BB
BB                                           BB
BB                                           BB
BB                                           BB
BB                                           BB
  BB                                       BB
  BB                                       BB
    BB                                   BB
    BB                                   BB
      BB                               BB
        BBBBB                     BBBBB
             BBBBB           BBBBB
                  BBBBBBBBBBB
*/

static const int blockTypeCount = 10;

// block types
enum bt {
    bt_air = -1,
    bt_grass,
    bt_dirt,
    bt_sand,
    bt_stone,
    bt_brick,
    bt_log,
    bt_leaf,
    bt_wool_red,
    bt_wool_yellow,
    bt_wool_black
};

enum biome {
    forest = 0,
    desert
};

static const char* blockNames[blockTypeCount] {
    "Grass",
    "Dirt",
    "Sand",
    "Stone",
    "Brick",
    "Log",
    "Leaf",
    "Red wool",
    "Yellow wool",
    "Black wool"
};

#endif