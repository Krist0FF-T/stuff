#ifndef THIRD_H
#define THIRD_H

#include <math.h>
#include <raylib.h>

#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)

#define dCos(x) (cosf(x*PI/180.0f))
#define dSin(x) (sinf(x*PI/180.0f))

#define sign(x) (x == 0 ? 0 : (x < 0 ? -1 : 1))

static Font font;
static const char* fontFName = "fonts/retro.ttf";

static const Color bgc{100,180,240,255};

bool vec3eq(Vector3 v1, Vector3 v2) {
    return (v1.x == v2.x && v1.y == v2.y && v1.z == v2.z);
}

#define isIn(x, k1, k2) (x > k1 && x < k2)

float toRad(float deg) {return deg*PI/180;}

float fFloor(float x) {
    return (float) ((int)x);
}

float fCeil(float x) {
    return (float) ((int)x+1);
}

void DrawTextC(const char* text, Vector2 center, float fontSize, Color color) {
    DrawTextEx(font, text, {center.x - MeasureText(text, fontSize)/2, center.y-fontSize/2}, fontSize, 2, color);
};

bool chance(int m) {
    return GetRandomValue(1,m) == 1;
}

bool chanceP(float percent) {
    return GetRandomValue(1, (int) round(percent/100.0f)) == 1;
}

int choiceInt(int count, int *arr) {
    return arr[GetRandomValue(0,count-1)];
}

struct Cube {
    Vector3 pos, size;

    // top left front
    Vector3 getTLF() {
        return {pos.x-size.x/2, pos.y-size.y/2, pos.z-size.z/2};
    }

    // bottom right back
    Vector3 getBRB() {
        return {pos.x+size.x/2, pos.y+size.y/2, pos.z+size.z/2};
    }

    void setCenter(Vector3 center) {
        pos.x = center.x-size.x*0.5f;
        pos.y = center.y-size.y*0.5f;
        pos.z = center.z-size.z*0.5f;
    }

    bool collide(Cube c2) {

        Vector3 tl = getTLF(),
                c2tl = c2.getTLF();

        return {
            tl.x < (c2tl.x+c2.size.x) &&
            (tl.x+size.x) > c2tl.x &&

            tl.y < (c2tl.y+c2.size.y) &&
            (tl.y+size.y) > c2tl.y &&

            tl.z < (c2tl.z+c2.size.z) &&
            (tl.z+size.z) > c2tl.z
        };
    }

    void draw(Color color) {
        DrawCubeV(pos, size, color);
    }

    Cube movedCopy(float x, float y, float z) {
        return Cube{Vector3Add(pos, {x,y,z}), size};
    }

    Cube movedCopyV(Vector3 delta) {
        return Cube{Vector3Add(pos, delta), size};
    }
};



#endif