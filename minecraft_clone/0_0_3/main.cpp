#include <stdio.h>
#include <stdlib.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
//#define RAYGUI_IMPLEMENTATION
//#include <raygui.h>

#include <fstream>
#include <iostream>

#include "blocktypes.h"
#include "draw.h"
#include "third.h"

static const float
    gravity = 24.0f,
    playerW = 0.6f,
    playerH = 1.8f,

    playerEye = playerH*0.4f;

static Cube pCube{{0,0,0},{playerW, playerH, playerW}};

static float dt,
    rotX=0.0f, rotY = 90.0f,
    velY;

static int
    rad = 3,
    cType = 0,
    cRenderDist = 4;

static bool p = 1, atj = 0, // able to jump
    running = true;

Texture2D LoadTex2D(const char* fName, int width, int height) {
    Image img = LoadImage(fName);
    ImageResize(&img, width, height);
    return LoadTextureFromImage(img);
}

#define colorChangeA(color, a) ((Color){color.r, color.g, color.b, a})

static Camera3D camera;
static Vector3 camTarget;
static Vector3 tpPoint;

static const int
    W = 1920,
    H = 1080,
    CX = W/2,
    CY = H/2,

    chunkSize = 32,
    maxChunks = 100;

static Vector3 lookAt = {0, maxChunks*chunkSize-3, 0};

static const bool
    trees = true,
    flat = false;

static std::vector<std::string> textList;
static float textCd = 0.0f;

void newText(std::string txt) {
    if(textList.size() == 0) textCd = 2.0f;
    textList.push_back(txt);
}

// crosshair
static const int
    chSize = 15,
    chLineWidth = 4;

void drawCrosshair(Color color) {
    DrawLineEx({CX, CY-chSize}, {CX, CY+chSize}, chLineWidth, color);
    DrawLineEx({CX-chSize, CY}, {CX+chSize, CY}, chLineWidth, color);
}

static __int8 blocks[maxChunks][chunkSize][chunkSize][chunkSize];

static int blockCount; // counter for block on the map
static const std::string fName = "world";

std::string guiInputTxt(std::string displayText);

// declaration of pause function
void pause();

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
    return B_AIR;
}

int getAtV(Vector3 vec) {
    return getAt((int)vec.x, (int)vec.y, (int)vec.z);
}

void setAt(int x, int y, int z, int type) {
    if(onMap(x,y,z)) blocks[chunkOf(y)][x][y%chunkSize][z] = type;
}

void setAtV(Vector3 vec, int type) {
    setAt((int)vec.x, (int)vec.y, (int)vec.z, type);
}

bool isEmpty(int x, int y, int z) {
    if(onMap(x,y,z)) return (blocks[chunkOf(y)][x][y%chunkSize][z] == -1);
    return 1;
}

bool isEmptyV(Vector3 vec) {
    return isEmpty((int)vec.x, (int)vec.y, (int)vec.z);
}

bool blockCollidePlayer(int x, int y, int z, Vector3 p) {

    float add = 0.6f;
    return (
        p.x > (float)(x-add) &&
        p.x < (float)(x+add) &&

        p.y > (float)(y-playerH*0.25f) &&
        p.y < (float)(y+playerH*1.75f) &&

        p.z > (float)(z-playerW/2) &&
        p.z < (float)(z+playerW/2)
    );


    /*
    return CheckCollisionBoxes(
        // player box
        (BoundingBox) {
            (Vector3) {
                p.x-playerW*0.5f,
                p.y-playerH*0.5f,
                p.z-playerW*0.5f
            },

            (Vector3) {
                p.x+playerW*0.5f,
                p.y+playerH*0.5f,
                p.z+playerW*0.5f
            }
        },

        // block box
        (BoundingBox) {
            (Vector3) {
                (float)x-0.5f,
                (float)y-0.5f,
                (float)z-0.5f,
            },

            (Vector3) {
                (float)x+0.5f,
                (float)y+0.5f,
                (float)z+0.5f,
            }
        }
    );*/


}

Faces getFaces(int x, int y, int z) {
    float
        cx = camera.position.x,
        cy = camera.position.y,
        cz = camera.position.z;

    return {
        x == 0 || (cx < x && isEmpty(x-1, y, z)),
        x == chunkSize-1 || (cx > x && isEmpty(x+1, y, z)),

        y == 0 || (cy < y && isEmpty(x, y-1, z)),
        y == chunkSize*maxChunks-1 || (cy > y && isEmpty(x, y+1, z)),

        z == 0 || (cz < z && isEmpty(x, y, z-1)),
        z == chunkSize-1 || (cz > z && isEmpty(x, y, z+1)),

        //1,1,1,1,1,1

    };
}

Cube blockCube(int x, int y, int z) {
    return Cube{{(float)x,(float)y,(float)z},Vector3One()};
}

Cube blockCubeV(Vector3 pos) {
    return Cube{pos, Vector3One()};
}

void genTree(int x, int y, int z) {
    if(getAt(x,y,z) == B_GREEN) return;
    int w=5,h=6,l=5;

    // 5: dirt
    // 3: green

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
        8,  8,  8,  8,  8,
        8,  8,  8,  8,  8,
       -1, -1, -1, -1, -1,
       -1, -1, -1, -1, -1,

       -1, -1,  8, -1, -1,
       -1, -2,  8, -2, -1,
        8,  8,  8,  8,  8,
        8,  8,  8,  8,  8,
       -1, -1, -1, -1, -1,
       -1, -1, -1, -1, -1,

       -1,  8,  8,  8, -1,
       -1,  8,  8,  8, -1,
        8,  8,  7,  8,  8,
        8,  8,  7,  8,  8,
       -1, -1,  7, -1, -1,
       -1, -1,  7, -1, -1,

       -1, -1,  8, -1, -1,
       -1, -2,  8, -2, -1,
        8,  8,  8,  8,  8,
        8,  8,  8,  8,  8,
       -1, -1, -1, -1, -1,
       -1, -1, -1, -1, -1,

       -1, -1, -1, -1, -1,
       -1, -1, -1, -1, -1,
        8,  8,  8,  8,  8,
        8,  8,  8,  8,  8,
       -1, -1, -1, -1, -1,
       -1, -1, -1, -1, -1,

    };

    int ind = 0;

    for(int j=0; j<w; j++) {
    for(int i=h-1; i>=0; i--) {
    for(int k=0; k<l; k++) {
        if(treeBlocks[ind] != -1 && !(treeBlocks[ind] == -2 && chance(2)) && isEmpty(x+j-w/2,y+i,z+k-l/2)) {
            int type = treeBlocks[ind];
            if(type == -2) type = B_LEAF;
            setAt(x+j-w/2,y+i,z+k-l/2, type); //treeBlocks[x*l*h + y*w + z]
        }

        ind++;
    }}}
    
}

void clearWorld() {
    for(int i=0; i<maxChunks; i++) {
    
        for(int x=0; x<chunkSize; x++) {
        for(int y=0; y<chunkSize; y++) {
        for(int z=0; z<chunkSize; z++) {
            blocks[i][x][y][z] = -1;
        }}}

    }
}

void placePlayer() {

    bool placed = false;

    while(!placed) {
        int x = GetRandomValue(0,31);
        int z = GetRandomValue(0,31);

        for(int y=maxChunks*chunkSize-1; y>0 && !placed; y--) {
            placed = true;
            for(int a=0; a<3 && placed; a++) {
                if(!isEmpty(x,y+a,z))
                    placed = false;
            }

            if(!isFlippable(getAt(x,y-1,z)))
                placed = false;
            
            if(placed) {
                pCube.pos.x = x;
                pCube.pos.y = y+1;
                pCube.pos.z = z;
            }
        }
    }

}

void genMap2(int seed) {
    int oldSeed = GetRandomValue(0,2100000000);
    SetRandomSeed(seed);

    int biomeMap[maxChunks];

    int biome = BIOME_FOREST;

    for(int i=0; i<maxChunks; i++) {
        if(chance(2)) biome = GetRandomValue(0,4) == 0 ? BIOME_DESERT : BIOME_FOREST;
        biomeMap[i] = biome;
    }

    int imgC = 3;
    Image imgs[imgC*2];
    int tileSize = chunkSize;

    for(int i=0; i<2; i++) {
        for(int j=0; j<imgC; j++) {
            imgs[i*imgC+j] = GenImageCellular(chunkSize, chunkSize*maxChunks, tileSize*(((imgC+1)-j)/(float)(imgC+1)));
        }
    }

    bool xMap[chunkSize][chunkSize*maxChunks];
    bool zMap[chunkSize][chunkSize*maxChunks];

    int g, sum, k = 120;

    for(int x=0; x<chunkSize; x++) {
        for(int y=0; y<chunkSize*maxChunks; y++) {
            if(!flat) {
                // X =================
                sum = 0;
                for(int i=0; i<imgC; i++) {
                    sum += GetImageColor(imgs[i], x,y).r;
                }
                g = sum/imgC;
                xMap[x][y] = ((int) g < k);

                // Z ==================
                sum = 0;
                for(int i=0; i<imgC; i++) {
                    sum += GetImageColor(imgs[i+imgC], x,y).r;
                }
                g = sum/imgC;
                zMap[x][y] = ((int) g < k);
            }

            else {
                xMap[x][y] = y == 0;
                zMap[x][y] = y == 0;
            }

        }
    }

    for(int i=0; i<6; i++)
        UnloadImage(imgs[i]);

    int type;

    //int types[3] = {B_GREEN, B_STONE, B_DIRT};

    for(int x=0; x<chunkSize; x++) {
    for(int y=0; y<chunkSize*maxChunks; y++) {
        int biome = biomeMap[chunkOf(y)];
    for(int z=0; z<chunkSize; z++) {
        type = B_AIR;

        if((xMap[x][y] && zMap[z][y])) {
            // 10% chance to stone, 90 to dirt
            type = (chanceP(10.0f)) ? ((biome == BIOME_FOREST) ? B_DIRT : B_SAND) : B_STONE;
            // if there is no block above
            if(!((y == 0 || y == maxChunks*chunkSize-1) || (xMap[x][y+1]) && zMap[z][y+1])) {
                type = (biome == BIOME_FOREST) ? B_GREEN : B_SAND;
            }

        }

        setAt(x,y,z, type);
    }}}

    // trees

    if(!trees) {
        placePlayer();
        return;
    } 

    std::vector<Vector3> treeVec;
    int treeSb = 6;

    for(int x=0; x<chunkSize; x++) {
    for(int y=0; y<chunkSize*maxChunks; y++) {
        int biome = biomeMap[chunkOf(y)];
        treeSb = (biome == BIOME_FOREST) ? 7 : 3;
    for(int z=0; z<chunkSize; z++) {
        if(!(x > treeSb/2 && x < chunkSize-treeSb/2 && z > treeSb/2 && z < chunkSize-treeSb/2)) continue;

        // is grass, empty above and 1% chance
        if(chance(biome == BIOME_FOREST ? 120 : 60) && isFlippable(getAt(x,y,z)) && getAt(x,y+1,z) == B_AIR) {

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
                if(biome == BIOME_FOREST) {
                    genTree(x,y+1,z);
                } else {

                    int maxHeight = GetRandomValue(2, 6);
                    for(int i=0; i<maxHeight; i++) {
                        if(!isEmpty(x,y+2+i,z)) break;
                        setAt(x,y+1+i,z, B_LEAF);

                    }
                }
            }
        }
    }}}

    placePlayer();

    SetRandomSeed(oldSeed);
}

void genWorld() {
    std::string input = guiInputTxt("World generation seed");
    int seed;
    try {
        seed = std::stoi(input);
    }
    catch (const std::invalid_argument & e) {
        seed = GetRandomValue(0,2100000);
        std::cout << e.what() << "\n";
    }
    catch (const std::out_of_range & e) {
        seed = GetRandomValue(0,2100000);
        std::cout << e.what() << "\n";
    }

    genMap2(seed);
}

/*
void genMap3() {
    printf("%s\n", "line 315.");
    int tileSize = chunkSize,
        imgC = 2,
        sides = 4,
        sum, g, h,

        add = 1;

    float mult = 3;
    Image imgs[sides][imgC];

    int sideMaps[sides][chunkSize][chunkSize*maxChunks];

    // generate cellular images
    for(int i=0; i<sides; i++) {
        for(int j=0; j<imgC; j++) {
            imgs[i][j] = GenImageCellular(chunkSize, chunkSize*maxChunks, tileSize*(((imgC+1)-j)/(float)(imgC+1)));
        }
    }

    // generate side maps
    for(int i=0; i<sides; i++) {
        for(int x=0; x<chunkSize; x++) {
        for(int y=0; y<chunkSize*maxChunks; y++) {
            // calculate average
            int sum = 0;
            for(int j=0; j<imgC; j++) {
                sum += (int) GetImageColor(imgs[i][j], x,y).r;
            }
            g = sum/imgC;

            // calculate height
            h = g / 255.0f * mult + add;

            sideMaps[i][x][y] = h;
            
        }}
    }

    // unload cellular images
    for(int i=0; i<sides; i++) {
        for(int j=0; j<imgC; j++) {
            UnloadImage(imgs[i][j]);
        }
    }

    int type, chunk;
    bool tempMap[maxChunks][chunkSize][chunkSize][chunkSize];

    for(int x=0; x<chunkSize; x++) {
    for(int y=0; y<chunkSize*maxChunks; y++) {
        chunk = chunkOf(y);
    for(int z=0; z<chunkSize; z++) {
        type = false;

        if(
            x < sideMaps[0][z][y] ||
            chunkSize-x-1 < sideMaps[2][z][y] ||

            z < sideMaps[1][x][y] ||
            chunkSize-z-1 < sideMaps[3][x][y]

            ) type = true;

        tempMap[chunk][x][y%chunkSize][z] = type;

    }}}

    for(int x=0; x<chunkSize; x++) {
    for(int y=0; y<chunkSize*maxChunks; y++) {
        chunk = chunkOf(y);
    for(int z=0; z<chunkSize; z++) {
        type = B_AIR;

        if(tempMap[chunk][x][y%chunkSize][z]) {
            type = 0;
        }

        setAt(x,y,z, type);

    }}}

}*/

int loadWorld(std::string fName) {
	std::ifstream inputData;
	inputData.open("worlds/" + fName + ".save");
	if (inputData) {
        int16_t pos[3];
        inputData.read(reinterpret_cast<char *>(pos), sizeof(int16_t) * 3);
        pCube.pos.x = (float) ((int)pos[0]*0.1f);
        pCube.pos.y = (float) ((int)pos[1]*0.1f);
        pCube.pos.z = (float) ((int)pos[2]*0.1f);
        tpPoint = pCube.pos;

        velY = 0.0f;

        for(int chunk=0; chunk<maxChunks; chunk++) {

            for(int x=0; x<chunkSize; x++) {
            for(int y=0; y<chunkSize; y++) {
                inputData.read(reinterpret_cast<char *>(blocks[chunk][x][y]), sizeof(__int8) * chunkSize);
            }}
        }
		
		inputData.close();
		return 0;
	}
	else
		return -1;
}

int saveWorld(std::string fName) {
	std::ofstream outputData;
	outputData.open("worlds/" + fName + ".save");
	if (outputData) {
        int16_t pos[3] {
            (int16_t) (pCube.pos.x*10.0f),
            (int16_t) (pCube.pos.y*10.0f),
            (int16_t) (pCube.pos.z*10.0f),
        };
        
        outputData.write(reinterpret_cast<const char *>(pos), sizeof(int16_t)*3);

        for(int i=0; i<maxChunks; i++) {

            for(int x=0; x<chunkSize; x++) {
            for(int y=0; y<chunkSize; y++) {
                outputData.write(reinterpret_cast<const char *>(blocks[i][x][y]), sizeof(__int8) * chunkSize);
            }}
        }

		
		outputData.close();
		return 0;
	}
	else
		return -1;
}

void update() {
    textCd -= dt;
    if(textCd < 0.0f && textList.size() > 0) {
        textList.erase(textList.begin());
        textCd = 1.0f;
    }

    blockCount = 0;

    // change rotation
    rotX += GetMouseDelta().x/10.0f;//(IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT))*camera.fovy*dt;
    if(rotX > 360) rotX -= 360;
    if(rotX < 0)   rotX += 360;

    cType += sign(GetMouseWheelMove());
    cType = min(max(cType, 0), blockTypeCount-1);

    bool forward = IsKeyDown(KEY_W),
         back    = IsKeyDown(KEY_S),
         right   = IsKeyDown(KEY_D),
         left    = IsKeyDown(KEY_A);

    // move
    velY -= gravity*dt;
    if(atj && IsKeyDown(KEY_SPACE))  {
        velY += 15.0f;//20.0f;
        atj = false;
    } 

    velY = max(velY, -20.0f);

    float dx = dCos(rotX)*10*dt*(forward-back);
    float dz = dSin(rotX)*10*dt*(forward-back);

    dx += dCos((rotX+90.0f))*10*dt*(right-left);
    dz += dSin((rotX+90.0f))*10*dt*(right-left);
    float dy = velY*dt;
    
    int testW = (int) ceilf(dx)+1,
        testH = (int) ceilf(dy)+1,
        testL = (int) ceilf(dz)+1;

    for(int x=(int)fFloor(pCube.getTLF().x)-1; x<(int)fCeil(pCube.getBRB().x)+2; x++) {
    for(int y=(int)fFloor(pCube.getTLF().y)-1; y<(int)fCeil(pCube.getBRB().y)+2+testH+1; y++) {
    for(int z=(int)fFloor(pCube.getTLF().z)-1; z<(int)fCeil(pCube.getBRB().z)+2+testL+1; z++) {

        if(onMap(x,y,z) && !isEmpty(x,y,z)) {
            Cube cube = blockCube(x,y,z);

            if(pCube.movedCopy(dx,0,0).collide(cube)) {
                dx = 0;
            }

            if(pCube.movedCopy(0,dy,0).collide(cube)) {
                dy = 0;
                atj = true;
                velY = 0;
            }

            if(pCube.movedCopy(0,0,dz).collide(cube)) {
                dz = 0;
            }

        }

    }}}
    
    pCube.pos.x += dx;
    pCube.pos.y += dy;
    pCube.pos.z += dz;

    pCube.pos.x = min(max(pCube.pos.x, 0.0f), chunkSize-1.0f);
    pCube.pos.z = min(max(pCube.pos.z, 0.0f), chunkSize-1.0f);

    if(pCube.pos.y < -10) pCube.pos.y = chunkSize*maxChunks+2;

    //pCube.pos = {camera.position.x, camera.position.y+1, camera.position.z};
    camera.position = {pCube.pos.x, pCube.pos.y+playerEye, pCube.pos.z};

    // view up & down
    rotY += GetMouseDelta().y/10;
    rotY = min(max(rotY, 1.0f), 179.0f);
    camTarget.y = dCos(rotY)*rad + camera.position.y;
    camTarget.x = dSin(rotY)*dCos(rotX)*rad+camera.position.x;
    camTarget.z = dSin(rotY)*dSin(rotX)*rad+camera.position.z;

    camera.target = camTarget;

    // Toggle fullscreen
    if(IsKeyPressed(KEY_F11)) ToggleFullscreen();

    SetMousePosition(CX, CY);
    
    float reach = 8.0f;
    float step = 0.1f;
    Vector3 placeAdd{0,0,0};
    for(float cRad=0; cRad < reach+step; cRad += step) {
        Vector3 lookAtf {
            dSin(rotY)*dCos(rotX)*cRad+camera.position.x,
            dCos(rotY)*cRad + camera.position.y,
            dSin(rotY)*dSin(rotX)*cRad+camera.position.z
        };
        lookAt = (Vector3) {
            roundf(lookAtf.x),
            roundf(lookAtf.y),
            roundf(lookAtf.z)
        };

        // block found
        if(!isEmptyV(lookAt) || !onMapV(lookAt)) {

            dx = lookAtf.x-lookAt.x;
            dy = lookAtf.y-lookAt.y;
            dz = lookAtf.z-lookAt.z;

            int x = lookAt.x;
            int y = lookAt.y;
            int z = lookAt.z;

            float add = 0.4f;

            if(dx < -add && isEmpty(x-1,y,z))
                placeAdd = (Vector3) {-1,0,0};

            else if(dx > add && isEmpty(x+1,y,z))
                placeAdd = (Vector3) {1,0,0};

            else if(dy < -add && isEmpty(x,y-1,z))
                placeAdd = (Vector3) {0,-1,0};

            else if(dy > add && isEmpty(x,y+1,z))
                placeAdd = (Vector3) {0,1,0};

            else if(dz < -add && isEmpty(x,y,z-1))
                placeAdd = (Vector3) {0,0,-1};

            else if(dz > add && isEmpty(x,y,z+1))
                placeAdd = (Vector3) {0,0,1};

            break;
        }

        if(cRad >= reach)
            lookAt.x = -1;
    }

    if(IsKeyPressed(KEY_P)) p = !p;

    if(IsKeyPressed(KEY_T)) {
        pCube.pos = tpPoint;
        newText(TextFormat("Teleported to: %.1f, %.1f, %.1f", tpPoint.x, tpPoint.y, tpPoint.z));
    }
    
    Vector3 placeAt = Vector3Add(lookAt, placeAdd);

    if(IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) && onMapV(lookAt) && !isEmptyV(lookAt))
        cType = getAtV(lookAt);

    if((IsMouseButtonDown(MOUSE_BUTTON_LEFT)*p) || (!p && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))) {

        if(!isEmpty((int)lookAt.x, (int)lookAt.y, (int)lookAt.z) && onMapV(lookAt) &&
            onMapV(lookAt))
            // set to air (delete)
            setAtV(lookAt, B_AIR);
    }

    if(onMapV(placeAt) && (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)*p || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)*(!p))
        && isEmptyV(placeAt) && !pCube.collide(blockCubeV(placeAt)))
        setAtV(placeAt, cType);

    if(IsKeyPressed(KEY_ESCAPE))
        pause();

    if(IsKeyDown(KEY_LEFT_CONTROL)) {
        // save world
        if(IsKeyPressed(KEY_S)) {
            if(saveWorld(guiInputTxt("save as")) == 0) {
                std::cout << "[+] world saved.\n";
                newText("World saved");
            } else {
                newText("Failed saving world");
                std::cout << "[ERROR] world NOT saved.\n";
            }
        }

        // load world
        if(IsKeyPressed(KEY_L)) {
            if(loadWorld(guiInputTxt("load from")) == 0) {
                newText("World loaded");
                std::cout << "[+] world loaded.\n";
            } else {
                newText("Failed loading world");
                std::cout << "[ERROR] world NOT loaded.\n";
            }
        }

        // Generate world
        if(IsKeyPressed(KEY_G))
            genWorld();

        // clear world
        if(IsKeyPressed(KEY_C))
            clearWorld();
    }

}

void draw3D() {

    int cc = (int) round(camera.position.y/chunkSize);

    bool lookUp = (rotY < 90.0f);

    // minimum
    int k1 = lookUp ? max(cc-1, 0) : max(cc-cRenderDist,0);
    // maximum
    int k2 = lookUp ? min(cc+cRenderDist, maxChunks-1) : min(cc, maxChunks-1);

    for(int c = k1; c < k2+1; c++) {
        for(int x=0; x<chunkSize; x++) {
        // i = y from chunk bottom
        for(int i=0; i<chunkSize; i++) {
        for(int z=0; z<chunkSize; z++) {

        int y = c*chunkSize+i;

        if(!isEmpty(x,y,z)) {
            Faces faces = getFaces(x,y,z);
            if(!faces.back && !faces.bottom && !faces.front && !faces.left && !faces.right && !faces.top) continue;
            drawCubeTextureFaces({(float) x, (float) y, (float) z}, faces, blocks[c][x][i][z]);

            //drawCubeColorFaces({(float) x, (float) y, (float) z}, getFaces(x,y,z), blockColors[getAt(x,y,z)]);
            //DrawCubeTexture(textures[0], {(float) x, (float) y, (float) z}, 1,1,1, WHITE);
            if(dt < 0.1f) blockCount++;
        }

    }}}
    }

    DrawCubeWiresV(pCube.pos, pCube.size, RED);


    if(onMapV(lookAt))
        DrawCubeWires({lookAt.x, lookAt.y, lookAt.z}, 1,1,1,{255,255,255,128});
}

void draw2D() {
    DrawTextEx(font, "Version 0.0.3", {10,10}, 20, 2, WHITE);
    //DrawTextEx(font, TextFormat("fps: %i", (int) round(1/dt)), {10,10}, 20, 2, WHITE);
    DrawTextEx(font, TextFormat("xyz: %.1f %.1f %.1f", camera.position.x, camera.position.y, camera.position.z),
                {10,40}, 20, 2, WHITE);

    DrawTextEx(font, TextFormat("looking at: %i %i %i", (int)lookAt.x, (int)lookAt.y, (int)lookAt.z),
                {10,70}, 20, 2, WHITE);
    
    DrawTextEx(font, TextFormat("blocks: %i", blockCount),
                {10,100}, 20, 2, WHITE);
    
    DrawTextEx(font, blockNames[cType],
                {10,130}, 20, 2, blockColors[cType]);

    DrawTextEx(font, TextFormat("Y velocity: %i m/s", (int) velY),
                {10,160}, 20, 2, WHITE);

    DrawTextEx(font, TextFormat("Horizontal rot: %i'", (int) rotX),
                {10,190}, 20, 2, WHITE);
    
    DrawTextEx(font, TextFormat("Vertical rot: %i'", (int) rotY),
                {10,220}, 20, 2, WHITE);

    for(int i=0; i<textList.size(); i++) {
        int i2 = textList.size()-1-i;
        unsigned char a = (i == 0) ? (unsigned char) (int) (min(max(textCd, 0.0f),1.0f)*255.0f) : 255;
        DrawTextC(textList[i].c_str(), {CX, 60+(float)i2*30}, 20,
            (Color){255,255,255,a});
    }

    if(textList.size() == 0)
        DrawTextC(TextFormat("%i", (int)(1/dt)), {CX, 30}, 30, WHITE);

    drawCrosshair(WHITE);


}

void draw() {
    BeginDrawing();
        ClearBackground(bgc);

        BeginMode3D(camera);

            draw3D();

        EndMode3D();

        draw2D();

    EndDrawing();
}

void init() {
    InitWindow(W, H, "Game");
    SetExitKey(KEY_F4);
    SetWindowState(FLAG_VSYNC_HINT);

    HideCursor();

    font = LoadFont(fontFName);
    initTextures();
    printf("loadTextures: done :D\n");

    camera.position = (Vector3){ chunkSize/2.0f, 20.0f, chunkSize/2.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 75.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    ToggleFullscreen();
    genWorld();

    //clearWorld();
    //loadWorld(guiInputTxt("Load map from"));
    printf("map generating: done\n");
}

void deInit() {
    UnloadFont(font);

    for(int i=0; i<7; i++) {
        UnloadTexture(textures[i]);
    }

    CloseWindow();
}

int main () {
    init();

    // main loop
    while(running) {
        
        dt = GetFrameTime();
        //UpdateCamera(&camera);

        draw();

        if(dt < 0.1f) {
            update();
        }

        running = !WindowShouldClose();

    }

    deInit();
}

std::string guiInputTxt(std::string displayText) {

    bool done;
    int key;
    std::string txt;

    bool isTyping = true;

    while(isTyping) {

        if(WindowShouldClose())
            deInit();

        BeginDrawing();

            ClearBackground(bgc);

            DrawTextC(displayText.c_str(), {CX, CY-40}, 80, WHITE);
            DrawTextC(txt.c_str(), {CX,CY+40}, 80, WHITE);

            DrawFPS(0,0);

        EndDrawing();

        key = GetCharPressed();

        // input text
        while (key > 0) {
            if ((key >= 32) && (key <= 125)) {
                txt.push_back((char)key);

                key = GetCharPressed();
            }

        }

        if (IsKeyPressed(KEY_BACKSPACE) && txt.length() > 0) {
            txt.resize(txt.size()-sizeof(char));
        }

        if(IsKeyPressed(KEY_F11))
            ToggleFullscreen();

        if(IsKeyReleased(KEY_ENTER))
            isTyping = false;


    }

    return txt;
}

void pause() {
    bool isPaused = true;
    while(isPaused) {

        if(WindowShouldClose()) {
            deInit();
        }

        BeginDrawing();
            ClearBackground(bgc);

            DrawRectangle(100, 100, 100, 100, RED);


        EndDrawing();
        


        if(IsKeyPressed(KEY_ESCAPE))
            isPaused = false;
    }
}