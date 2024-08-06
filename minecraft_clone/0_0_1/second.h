#pragma once

#include <raylib.h>
#include <raymath.h>

#include "third.h"

static const int
    W = 1920, // window width
    H = 1080, // window height
    CX = W/2,
    CY = H/2,
    maxBlocks = 3000,

    reloadDist = 100;

static Vector3 camTarget = Vector3Zero();
static float dt, rotation, a, h=0, dx, dy, rad=4.0f;;

static float sandCooldown = 0.0f;

static Camera3D camera;
static const Color textColor = WHITE;

static Texture2D sandTexture;

static int cBlocks = 0;

struct Block {
    int type;
    Vector3 pos;
    Faces faces;
};

Block blocks[maxBlocks];

bool isEmpty(Vector3 pos) {
    if(pos.y < 0.0f) return 1;
    for(int i=0; i<maxBlocks; i++) {
        if(vec3eq(pos, blocks[i].pos))
            return blocks[i].type == -1;

        }

    return 1;
}

static Faces getFaces(Vector3 *bp) {

    Vector3 *cp = &camera.position;

    return {
        cp->x < bp->x && isEmpty({bp->x-1.0f, bp->y, bp->z}),
        cp->x > bp->x && isEmpty({bp->x+1.0f, bp->y, bp->z}),

        cp->y < bp->y && isEmpty({bp->x, bp->y-1.0f, bp->z}),
        cp->y > bp->y && isEmpty({bp->x, bp->y+1.0f, bp->z}),

        cp->z < bp->z && isEmpty({bp->x, bp->y, bp->z-1.0f}),
        cp->z > bp->z && isEmpty({bp->x, bp->y, bp->z+1.0f}),
    };
}

// update & draw ================================

void update() {
    if(sandCooldown > 0.0f)
        sandCooldown -= dt;

    cBlocks = 0;
    for(int i=0; i<maxBlocks; i++) {
        if(blocks[i].type != -1) cBlocks++;
        if(sandCooldown > 0) continue;
        if(blocks[i].type == 1) {

            if(blocks[i].pos.y < 0.0f) {
                blocks[i].type = -1;
                continue;
            }

            // ha alatta üres, akkor esik
            if (isEmpty(Vector3Add(blocks[i].pos, {0.0f, -1.0f, 0.0f})))
                blocks[i].pos.y += -1.0f;

            // ha nem:
            else if(chance(5)) {
            // kiválaszt 1-et a 8 irányból
            switch(GetRandomValue(1,8)) {

            // X ++
            case 1: {
                if(isEmpty(Vector3Add(blocks[i].pos, {1.0f,-1.0f,0.0f}))) {
                    blocks[i].pos.x += 1.0f;
                    blocks[i].pos.y += -1.0f;
                }
                break;
            }

            // X --
            case 2: {
                if (isEmpty(Vector3Add(blocks[i].pos, {-1.0f,-1.0f,0.0f}))) {
                    blocks[i].pos.x += -1.0f;
                    blocks[i].pos.y += -1.0f;
                }
                break;
            }
    
            // Z++
            case 3: {
                if (isEmpty(Vector3Add(blocks[i].pos, {0.0f,-1.0f,1.0f}))) {
                    blocks[i].pos.z += 1.0f;
                    blocks[i].pos.y += -1.0f;
                }
                break;
            }

            // Z --
            case 4: {
                if (isEmpty(Vector3Add(blocks[i].pos, {0.0f,-1.0f,-1.0f}))) {
                    blocks[i].pos.z += -1.0f;
                    blocks[i].pos.y += -1.0f;
                }
                break;
            }


            // X ++ ; Z++
            case 5: {
                if (isEmpty(Vector3Add(blocks[i].pos, {1.0f,-1.0f,1.0f}))) {
                    blocks[i].pos.x += 1.0f;
                    blocks[i].pos.z += 1.0f;

                    blocks[i].pos.y += -1.0f;
                }
                break;
            }

            // X ++ ; Z --
            case 6: {
                if (isEmpty(Vector3Add(blocks[i].pos, {1.0f,-1.0f,-1.0f}))) {
                    blocks[i].pos.x += 1.0f;
                    blocks[i].pos.z += -1.0f;

                    blocks[i].pos.y += -1.0f;
                }
                break;
            }

            // X -- ; Z ++
            case 7: {
                if (isEmpty(Vector3Add(blocks[i].pos, {-1.0f,-1.0f,1.0f}))) {
                    blocks[i].pos.x += -1.0f;
                    blocks[i].pos.z += 1.0f;

                    blocks[i].pos.y += -1.0f;
                }
                break;
            }

            // X -- ; Z --
            case 8: {
                if (isEmpty(Vector3Add(blocks[i].pos, {-1.0f,-1.0f,-1.0f}))) {
                    blocks[i].pos.x += -1.0f;
                    blocks[i].pos.z += -1.0f;

                    blocks[i].pos.y += -1.0f;
                }
                break;
            }

            } // switch
            } // else-if
        }

    }
    if(!(sandCooldown > 0.0f))
        sandCooldown = 0.02f;
}

void reloadNear(Vector3 pos, float maxD) {
    for(int i=0; i<maxBlocks; i++) {
        Block *b = &blocks[i];
        if(b->type != -1 &&
            abs(b->pos.x-pos.x)<maxD &&
            abs(b->pos.y-pos.y)<maxD &&
            abs(b->pos.z-pos.z)<maxD) {

                b->faces = getFaces(&b->pos);

        }
        
    }
}

void draw() {

}

// Init & deInit ================================

void init() {
    InitWindow(W, H, "Epito jatek");
    //SetWindowState(FLAG_VSYNC_HINT);

    font = LoadFont("fffforwa.ttf");

    Image img = GenImageChecked(3,3,1,1, {194,178,128,255}, {184,169,121,255});
    sandTexture = LoadTextureFromImage(img);
    UnloadImage(img);

    camera.position = (Vector3){ 0.0f, 10.0f, 0.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    for(int i=0; i<maxBlocks; i++) {
        blocks[i].type = -1;
        blocks[i].pos = Vector3Zero();
    }
}

void deInit() {
    UnloadTexture(sandTexture);
    UnloadFont(font);
}
