#ifndef K_DRAW_H
#define K_DRAW_H

#include <raylib.h>
#include <rlgl.h>

// own
#include "blocktypes.h"

struct Faces {
    bool
        left,     // X -
        right,    // X +

        bottom,   // Y -
        top,      // Y +

        front,    // Z -
        back;     // Z +
};

static const int texC = 12;

static Texture2D textures[texC];

static const int blockTextureI[blockTypeCount][6] {
    {4, 4, 4, 4, 4, 4},  // Red
    {7, 7, 7, 7, 7, 7},  // Yellow
    {5, 5, 5, 5, 5, 5},  // Black
    {0, 0, 1, 2, 0, 0},  // Grass
    {3, 3, 3, 3, 3, 3},  // Stone
    {1, 1, 1, 1, 1, 1},  // Dirt
    {6, 6, 6, 6, 6, 6},  // Sand
    {8, 8, 9, 9, 8, 8},  // Log
    {10,10,10,10,10,10}, // Leaf
    {11,11,11,11,11,11}, // Glass
};

void drawCubeTextureFaces(Vector3 position, Faces faces, int type) {

    float
        x = position.x,
        y = position.y,
        z = position.z,

        width = 1.0f,
        height = 1.0f,
        length = 1.0f;

    unsigned char a = isTransparent(type) ? 128 : 255;

    int c[3] = {204, 255, 153}; //{230, 255, 204};

    rlCheckRenderBatchLimit(18);

    // 0: left
    // 1: right
    // 2: bottom
    // 3: top
    // 4: front
    // 5: back

    
    //rlPushMatrix();
        // NOTE: Transformation is applied in inverse order (scale -> rotate -> translate)
        //rlTranslatef(2.0f, 0.0f, 0.0f);
        //rlRotatef(45, 0, 1, 0);
        //rlScalef(2.0f, 2.0f, 2.0f);

        
        rlBegin(RL_QUADS);

            rlColor4ub(c[0], c[0], c[0], a);
            // Front Face
            rlSetTexture(textures[blockTextureI[type][4]].id);
            if(faces.back) {
                rlNormal3f(0.0f, 0.0f, 1.0f);                  // Normal Pointing Towards Viewer
                rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad
                rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad
                rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Right Of The Texture and Quad
                rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Left Of The Texture and Quad    
            }

            // Back Face
            if(blockTextureI[type][5] != blockTextureI[type][4]) rlSetTexture(textures[blockTextureI[type][5]].id);
            if(faces.front) {
                rlNormal3f(0.0f, 0.0f, - 1.0f);                  // Normal Pointing Away From Viewer
                rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Right Of The Texture and Quad
                rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad
                rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad
                rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Left Of The Texture and Quad
            }

            rlColor4ub(c[1], c[1], c[1], a);
            // Top Face
            if(blockTextureI[type][3] != blockTextureI[type][5]) rlSetTexture(textures[blockTextureI[type][3]].id);
            if(faces.top) {
                rlNormal3f(0.0f, 1.0f, 0.0f);                  // Normal Pointing Up
                rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad
                rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);  // Bottom Left Of The Texture and Quad
                rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);  // Bottom Right Of The Texture and Quad
                rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad    
            }

            rlColor4ub(c[2], c[2], c[2], a);

            // Bottom Face
            if(blockTextureI[type][2] != blockTextureI[type][3]) rlSetTexture(textures[blockTextureI[type][2]].id);
            if(faces.bottom) {
                rlNormal3f(0.0f, - 1.0f, 0.0f);                  // Normal Pointing Down
                rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);  // Top Right Of The Texture and Quad
                rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);  // Top Left Of The Texture and Quad
                rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad
                rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad
            }

            // Right face
            if(blockTextureI[type][1] != blockTextureI[type][2]) rlSetTexture(textures[blockTextureI[type][1]].id);
            if(faces.right) {
                rlNormal3f(1.0f, 0.0f, 0.0f);                  // Normal Pointing Right
                rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Right Of The Texture and Quad
                rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad
                rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Left Of The Texture and Quad
                rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad
            }

            // Left Face
            if(blockTextureI[type][0] != blockTextureI[type][1]) rlSetTexture(textures[blockTextureI[type][0]].id);
            if(faces.left) {
                rlNormal3f( - 1.0f, 0.0f, 0.0f);                  // Normal Pointing Left
                rlTexCoord2f(0.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Left Of The Texture and Quad
                rlTexCoord2f(1.0f, 0.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad
                rlTexCoord2f(1.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Right Of The Texture and Quad
                rlTexCoord2f(0.0f, 1.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad    
            }

        rlEnd();
    //rlPopMatrix();

    rlSetTexture(0);
}

void drawCubeColorFaces(Vector3 position, Faces faces, Color color) {
    
    if(!faces.back && !faces.bottom && !faces.front && !faces.left && !faces.right && !faces.top)
        return;

    float x=0, y=0, z=0;

    float
        width = 1.0f,
        height = 1.0f,
        length = 1.0f;

    int c[3] = {230, 255, 204};

    rlCheckRenderBatchLimit(18); // 36

    rlPushMatrix();
        // NOTE: Transformation is applied in inverse order (scale -> rotate -> translate)
        rlTranslatef(position.x, position.y, position.z);
        //rlRotatef(45, 0, 1, 0);
        //rlScalef(1.0f, 1.0f, 1.0f);   // NOTE: Vertices are directly scaled on definition

        rlBegin(RL_TRIANGLES);
            rlColor4ub((unsigned char) (int) color.r *0.9f,
                       (unsigned char) (int) color.g *0.9f,
                       (unsigned char) (int) color.r *0.9f,
                       color.a);

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


            rlColor4ub((unsigned char) (int) color.r *0.8f,
                       (unsigned char) (int) color.g *0.8f,
                       (unsigned char) (int) color.r *0.8f,
                       color.a);

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

#endif