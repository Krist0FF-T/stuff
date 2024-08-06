#pragma once

#include <raymath.h>
#include <raylib.h>
#include <rlgl.h>

static Font font;

Texture2D LoadTex2D(const char* fName, int width, int height) {
    Image img = LoadImage(fName);
    ImageResize(&img, width, height);
    return LoadTextureFromImage(img);
}

#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)

#define dCos(x) (cosf(x*PI/180))
#define dSin(x) (sinf(x*PI/180))

bool vec3eq(Vector3 v1, Vector3 v2) {
    return (v1.x == v2.x && v1.y == v2.y && v1.z == v2.z);
}

void DrawTextC(const char* text, Vector2 center, float fontSize, Color color) {
    DrawTextEx(font, text, {center.x - MeasureText(text, fontSize)/2, center.y-fontSize/2}, fontSize, 0, color);
};

int minmax(int x, int k1, int k2) {return min(max(x,k1),k2);}
bool isIn(int x, int k1, int k2){return (x > k1 && x < k2);};
float toRad(float deg) {return deg*PI/180;}

bool chance(int m) {
    return GetRandomValue(1,m) == 1;
}

struct Faces {
    bool
        left,     // X -
        right,    // X +

        bottom,   // Y -
        top,      // Y +

        front,    // Z -
        back;     // Z +
};



void drawCubeFaces(Vector3 position, Faces faces, Color color) {
    
    if(!faces.back && !faces.bottom && !faces.front && !faces.left && !faces.right && !faces.top)
        return;
    
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    float
        width = 1.0f,
        height = 1.0f,
        length = 1.0f;

    rlCheckRenderBatchLimit(36);

    rlPushMatrix();
        // NOTE: Transformation is applied in inverse order (scale -> rotate -> translate)
        rlTranslatef(position.x, position.y, position.z);
        //rlRotatef(45, 0, 1, 0);
        //rlScalef(1.0f, 1.0f, 1.0f);   // NOTE: Vertices are directly scaled on definition

        rlBegin(RL_TRIANGLES);
            rlColor4ub((unsigned char)(color.r*0.9f), (unsigned char)(color.g*0.9f),
                        (unsigned char)(color.b*0.9f), color.a);

            // Front face
            if(faces.back) {
                rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Left
                rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Right
                rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Left

                rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Right
                rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Left
                rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Right
            }

            // Back face
            if(faces.front) {
                
                rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Left
                rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left
                rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Right

                rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right
                rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Right
                rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left
            }

            rlColor4ub(color.r, color.g, color.b, color.a);
            // Top face
            if(faces.top) {
                
                rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left
                rlVertex3f(x - width/2, y + height/2, z + length/2);  // Bottom Left
                rlVertex3f(x + width/2, y + height/2, z + length/2);  // Bottom Right

                rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right
                rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left
                rlVertex3f(x + width/2, y + height/2, z + length/2);  // Bottom Right
            }

            rlColor4ub((unsigned char)(color.r*0.8f), (unsigned char)(color.g*0.8f),
                        (unsigned char)(color.b*0.8f), color.a);

            // Bottom face
            if(faces.bottom) {
                rlVertex3f(x - width/2, y - height/2, z - length/2);  // Top Left
                rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Right
                rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Left

                rlVertex3f(x + width/2, y - height/2, z - length/2);  // Top Right
                rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Right
                rlVertex3f(x - width/2, y - height/2, z - length/2);  // Top Left
            }

            // Right face
            if(faces.right) {
                rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Right
                rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right
                rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Left

                rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Left
                rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Right
                rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Left
            }


            // Left face
            if(faces.left) {
                rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Right
                rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Left
                rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Right

                rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Left
                rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Left
                rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Right
            }

        rlEnd();
    rlPopMatrix();
}
