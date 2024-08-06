#ifndef WORLD_H
#define WORLD_H

#include <raylib.h>
#include <raymath.h>

#include "third.h"
#include "blocktypes.h"

static const int
    maxChunks = 100,
    chunkSize = 32;

static const bool
    devMode = true,
    trees = true,
    flat = false;

static Vector3 tpPoint;

static char blocks[maxChunks][chunkSize][chunkSize][chunkSize];
static int blockCount;

struct FallingB {
    float vel;
    Vector3 pos;
    int type;
};

static std::vector<FallingB> fallingBlocks;

bool onMap(int x, int y, int z) {
    return (isIn(x, -1, chunkSize) && isIn(y, -1, chunkSize*maxChunks) && isIn(z, -1, chunkSize));
}

bool onMapV(Vector3 vec) {
    return onMap((int)vec.x, (int)vec.y, (int)vec.z);
}

int chunkOf(int y) {
    return (int) y/chunkSize;
}

int getAt(int x, int y, int z) {
    if(onMap(x,y,z)) return blocks[chunkOf(y)][x][y%chunkSize][z];
    return bt_air;
}

int getAtV(Vector3 vec) {
    return getAt((int)vec.x, (int)vec.y, (int)vec.z);
}

void setAt(int x, int y, int z, char type) {
    if(onMap(x,y,z)) blocks[chunkOf(y)][x][y%chunkSize][z] = type;
}

void setAtV(Vector3 vec, int type) {
    setAt((int)vec.x, (int)vec.y, (int)vec.z, type);
}

bool isEmpty(int x, int y, int z) {
    //return 0;
    if(onMap(x,y,z)) return (getAt(x,y,z) == bt_air);
    return 1;
}

bool isEmptyV(Vector3 vec) {
    return isEmpty((int)vec.x, (int)vec.y, (int)vec.z);
}

Cube blockCube(int x, int y, int z) {
    return Cube{{(float)x,(float)y,(float)z},Vector3One()};
}

Cube blockCubeV(Vector3 pos) {
    return Cube{pos, Vector3One()};
}

c4v genC4v(int x, int y, int z, int faceN) {
    bool
        top, bottom, left, right,
        topright, bottomright,
        topleft, bottomleft;

    switch(faceN) {
        case face_top: {
            top =    !isEmpty(x,y+1,z-1);
            bottom = !isEmpty(x,y+1,z+1);
            left =   !isEmpty(x-1,y+1,z);
            right =  !isEmpty(x+1,y+1,z);

            topleft  = !isEmpty(x-1,y+1,z-1);
            topright = !isEmpty(x+1,y+1,z-1);
            bottomleft  = !isEmpty(x-1,y+1,z+1);
            bottomright = !isEmpty(x+1,y+1,z+1);

            break;
        }

        case face_bottom: {
            top =    !isEmpty(x,y-1,z-1);
            bottom = !isEmpty(x,y-1,z+1);
            left =   !isEmpty(x-1,y-1,z);
            right =  !isEmpty(x+1,y-1,z);

            topleft  = !isEmpty(x-1,y-1,z-1);
            topright = !isEmpty(x+1,y-1,z-1);
            bottomleft  = !isEmpty(x-1,y-1,z+1);
            bottomright = !isEmpty(x+1,y-1,z+1);

            break;
        }

        case face_left: {

            top =    !isEmpty(x-1,y+1,z);
            bottom = !isEmpty(x-1,y-1,z);

            left  = !isEmpty(x-1,y,z-1);
            right = !isEmpty(x-1,y,z+1);

            topleft  = !isEmpty(x-1,y+1,z-1);
            topright = !isEmpty(x-1,y+1,z+1);

            bottomleft  = !isEmpty(x-1,y-1,z-1);
            bottomright = !isEmpty(x-1,y-1,z+1);

            break;
        }
    
        case face_right: {

            top =    !isEmpty(x+1,y+1,z);
            bottom = !isEmpty(x+1,y-1,z);

            left  = !isEmpty(x+1,y,z-1);
            right = !isEmpty(x+1,y,z+1);

            topleft  = !isEmpty(x+1,y+1,z-1);
            topright = !isEmpty(x+1,y+1,z+1);

            bottomleft  = !isEmpty(x+1,y-1,z-1);
            bottomright = !isEmpty(x+1,y-1,z+1);

            break;
        }

        // Z --
        case face_front: {

            top = !isEmpty(x, y+1, z-1);
            bottom = !isEmpty(x, y-1, z-1);

            left = !isEmpty(x-1,y,z-1);
            right = !isEmpty(x+1,y,z-1);

            topleft = !isEmpty(x-1,y+1,z-1);
            topright = !isEmpty(x+1,y+1,z-1);

            bottomleft = !isEmpty(x-1,y-1,z-1);
            bottomright = !isEmpty(x+1,y-1,z-1);

            break;
        }

        // Z ++
        case face_back: {
            top = !isEmpty(x, y+1, z+1);
            bottom = !isEmpty(x, y-1, z+1);

            left = !isEmpty(x-1,y,z+1);
            right = !isEmpty(x+1,y,z+1);

            topleft = !isEmpty(x-1,y+1,z+1);
            topright = !isEmpty(x+1,y+1,z+1);

            bottomleft = !isEmpty(x-1,y-1,z+1);
            bottomright = !isEmpty(x+1,y-1,z+1);

            break;
        }

        default: {
            top         = false;
            bottom      = false;
            left        = false;
            right       = false;
            topright    = false;
            bottomright = false;
            topleft     = false;
            bottomleft  = false;
            break;
        }
    }

    float level = 0.24f;

    float
        ctl = 1.0f-(top + left + topleft)*level,
        cbl = 1.0f-(top + right + topright)*level,
        cbr = 1.0f-(bottom + left + bottomleft)*level,
        ctr = 1.0f-(bottom + right + bottomright)*level;
    
    return {ctl,cbl,ctr,cbr};

}

void genTree(int x, int y, int z) {
    if(getAt(x,y,z) == bt_grass) return;
    int w=5,h=6,l=5;

    // 5: dirt
    // 3: green

    int uBlocks[2] = {bt_log, bt_leaf};
    
    int treeBlocks[w*h*l] = {
        /*
        -2,  8, -2,
        8,   8,  8,
        -1, -1, -1,

        8,  8,  8,
        8,  7,  8,
        -1, 7, -1,

        -2, 8, -2,
        8,  8,  8,
        -1, -1, -1,
        */

       -1, -1, -1, -1, -1,
       -1, -1, -1, -1, -1,
        1,  1,  1,  1,  1,
        1,  1,  1,  1,  1,
       -1, -1, -1, -1, -1,
       -1, -1, -1, -1, -1,

       -1, -1,  1, -1, -1,
       -1, -2,  1, -2, -1,
        1,  1,  1,  1,  1,
        1,  1,  1,  1,  1,
       -1, -1, -1, -1, -1,
       -1, -1, -1, -1, -1,

       -1,  1,  1,  1, -1,
       -1,  1,  1,  1, -1,
        1,  1,  0,  1,  1,
        1,  1,  0,  1,  1,
       -1, -1,  0, -1, -1,
       -1, -1,  0, -1, -1,

       -1, -1,  1, -1, -1,
       -1, -2,  1, -2, -1,
        1,  1,  1,  1,  1,
        1,  1,  1,  1,  1,
       -1, -1, -1, -1, -1,
       -1, -1, -1, -1, -1,

       -1, -1, -1, -1, -1,
       -1, -1, -1, -1, -1,
        1,  1,  1,  1,  1,
        1,  1,  1,  1,  1,
       -1, -1, -1, -1, -1,
       -1, -1, -1, -1, -1,

    };

    int ind = 0;

    for(int j=0; j<w; j++) {
    for(int i=h-1; i>=0; i--) {
    for(int k=0; k<l; k++) {
        if(treeBlocks[ind] != -1 && !(treeBlocks[ind] == -2 && chance(2)) && isEmpty(x+j-w/2,y+i,z+k-l/2)) {
            int type;
            if(treeBlocks[ind] == -2) type = bt_leaf;
            else type = uBlocks[treeBlocks[ind]];
            if(type != bt_air) blockCount++;
            setAt(x+j-w/2,y+i,z+k-l/2, type); //treeBlocks[x*l*h + y*w + z]
        }

        ind++;
    }}}
    
}

void clearWorld() {
    blockCount = 0;
    for(int i=0; i<maxChunks; i++) {
    
        for(int x=0; x<chunkSize; x++) {
        for(int y=0; y<chunkSize; y++) {
        for(int z=0; z<chunkSize; z++) {
            blocks[i][x][y][z] = -1;
        }}}

    }
}

void genMap2(int seed) {

    if(flat) {
        int type;

        for(int x=0; x<chunkSize; x++) {
        for(int y=0; y<chunkSize*maxChunks; y++) {
        for(int z=0; z<chunkSize; z++) {
            type = (y==0) ? bt_grass : bt_air;

            if(type != bt_air)
                blockCount++;

            setAt(x,y,z, type);
        }}}

        printf("flat.\n");
        return;
    }

    int oldSeed = GetRandomValue(0,2100000000);
    SetRandomSeed(seed);

    int biomeMap[maxChunks];

    int biome = biome_forest;

    for(int i=0; i<maxChunks; i++) {
        if(chance(2)) biome = GetRandomValue(0,4) == 0 ? biome_desert : biome_forest;
        biomeMap[i] = biome;
    }

    int imgC = 2;
    Image imgs[imgC*2];
    int tileSize = 32;//chunkSize;

    for(int i=0; i<2; i++) {
        for(int j=0; j<imgC; j++) {
            imgs[i*imgC+j] = GenImageCellular(chunkSize, chunkSize*maxChunks, tileSize*(((imgC+1)-j)/(float)(imgC+1)));
        }
    }

    bool xMap[chunkSize][chunkSize*maxChunks];
    bool zMap[chunkSize][chunkSize*maxChunks];

    int g, sum, k = 100;

    for(int x=0; x<chunkSize; x++) {
        for(int y=0; y<chunkSize*maxChunks; y++) {
            if(!flat) {
                // X =================
                sum = 0;
                for(int i=0; i<imgC; i++) {
                    sum += GetImageColor(imgs[i], x,y).r;
                }
                g = sum/imgC;
                xMap[x][y] = !((int)g < k);

                // Z ==================
                sum = 0;
                for(int i=0; i<imgC; i++) {
                    sum += GetImageColor(imgs[i+imgC], x,y).r;
                }
                g = sum/imgC;
                zMap[x][y] = !((int)g < k);

            } else {
                xMap[x][y] = y == 0;
                zMap[x][y] = y == 0;
            }

        }
    }
    
    for(int i = 0; i < (imgC*2); i++) {
        ExportImage(imgs[i], TextFormat("img%i.png", i));
        UnloadImage(imgs[i]);
    }
        

    int type;

    //int types[3] = {B_GREEN, B_STONE, bt_dirt};
    for(int x=0; x<chunkSize; x++) {
    for(int y=0; y<chunkSize*maxChunks; y++) {
        int biome = biomeMap[chunkOf(y)];
    for(int z=0; z<chunkSize; z++) {
        type = bt_air;

        if((xMap[x][y] && zMap[z][y])) {
            // 10% chance to stone, 90 to dirt
            // 2. bt_dirt: bt_sand
            
            // if there is no block above
            if(!((y == 0 || y == maxChunks*chunkSize-1) || (xMap[x][y+1]) && zMap[z][y+1])) {
                type = (biome == biome_forest) ? bt_grass : bt_stone;
                
            
            } else {
                type = (chanceP(10.0f)) ? ((biome == biome_forest) ? bt_dirt : bt_dirt) : bt_stone;
            }

        }

        setAt(x,y,z, type);
    }}}

    // trees
    if(!trees) {
        //placePlayer();
        return;
    } 

    std::vector<Vector3> treeVec;
    int treeSb = 6;

    for(int x=0; x<chunkSize; x++) {
    for(int y=0; y<chunkSize*maxChunks; y++) {
        int biome = biomeMap[chunkOf(y)];
        treeSb = (biome == biome_forest) ? 7 : 3;
    for(int z=0; z<chunkSize; z++) {
        if(!(x > treeSb/2 && x < chunkSize-treeSb/2 && z > treeSb/2 && z < chunkSize-treeSb/2)) continue;

        // is grass, empty above and 1% chance
        if(chance(biome == biome_forest ? 60 : 30) && isFlippable(getAt(x,y,z)) && getAt(x,y+1,z) == bt_air) {

            bool tree = true;
            for(int i=0; i<treeVec.size()&&tree; i++) {
                if(abs(treeVec[i].y-(y+1)) < treeSb &&
                   abs(treeVec[i].x-x) < treeSb &&
                   abs(treeVec[i].z-z) < treeSb)
                        tree = false;
            }

            for(int i=1; i<7 && tree; i++) {
                if(getAt(x,y+i,z) != -1 || !onMap(x,y+i,z))
                    tree = false;
            }

            if(tree) {
                treeVec.push_back((Vector3){(float)x, (float)y+1, (float)z});
                if(biome == biome_forest) {
                    genTree(x,y+1,z);
                } else {

                    int maxHeight = GetRandomValue(2, 6);
                    for(int i=0; i<maxHeight; i++) {
                        if(!isEmpty(x,y+2+i,z)) break;
                        setAt(x,y+1+i,z, bt_leaf);

                    }
                }
            }
        }
    }}}

    //placePlayer();

    // count blocks
    for(int c=0; c<maxChunks; c++) {
        for(int x=0; x<chunkSize; x++) {
        for(int y=0; y<chunkSize; y++) {
        for(int z=0; z<chunkSize; z++) {
            if(blocks[c][x][y][z] != bt_air)
                blockCount++;
        }}}
    }

    SetRandomSeed(oldSeed);
}

int saveWorld(std::string fName, Cube *pCube) {
	std::ofstream outputData;
	outputData.open("worlds/" + fName + ".save");
	if (outputData) {
        int16_t pos[3] {
            (int16_t) (pCube->pos.x*10.0f),
            (int16_t) (pCube->pos.y*10.0f),
            (int16_t) (pCube->pos.z*10.0f),
        };
        
        outputData.write(reinterpret_cast<const char *>(pos), sizeof(int16_t)*3);

        for(int i=0; i<maxChunks; i++) {

            for(int x=0; x<chunkSize; x++) {
            for(int y=0; y<chunkSize; y++) {
                outputData.write(reinterpret_cast<const char *>(blocks[i][x][y]), sizeof(char) * chunkSize);
            }}
        }

		
		outputData.close();
		return 0;
	}
	else
		return -1;
}

int loadWorld(std::string fName, Cube *pCube) {
	std::ifstream inputData;
	inputData.open("worlds/" + fName + ".save");
	if (inputData) {
        blockCount = 0;
        fallingBlocks.clear();
        int16_t pos[3];
        inputData.read(reinterpret_cast<char *>(pos), sizeof(int16_t) * 3);
        pCube->pos.x = (float) ((int)pos[0]*0.1f);
        pCube->pos.y = (float) ((int)pos[1]*0.1f);
        pCube->pos.z = (float) ((int)pos[2]*0.1f);
        tpPoint = pCube->pos;

        for(int chunk=0; chunk<maxChunks; chunk++) {

            for(int x=0; x<chunkSize; x++) {
            for(int y=0; y<chunkSize; y++) {
                inputData.read(reinterpret_cast<char *>(blocks[chunk][x][y]), sizeof(char) * chunkSize);
                for(int z=0; z<chunkSize; z++) {
                    if(blocks[chunk][x][y][z] != bt_air)
                        blockCount++;
                }
            }}
        }
		
		inputData.close();
		return 0;
	}
	else
		return -1;
}

#endif