#ifndef K_DRAW_H
#define K_DRAW_H

#include <raylib.h>
#include <rlgl.h>

#include <iostream>
#include <array>
#include <vector>

//#include <stdlib.h>

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

static const int texC = 11;

static Texture2D textures[texC];;

static std::string
    tpName = "default",
    tNames[texC] {

        "grass_top",
        "grass_side",
        "dirt",
        "sand",
        "stone",
        "log_top",
        "log_side",
        "leaf",
        "wool_red",
        "wool_yellow",
        "wool_black"
};

// type names enum
enum tne {
    grass_top = 0,
    grass_side,
    dirt,
    sand,
    stone,
    log_top,
    log_side,
    leaf,
    wool_red,
    wool_yellow,
    wool_black
};

static const int blockTextureI[blockTypeCount][3] {
    // order: top, side, bottom

    {tne::grass_top, tne::grass_side, tne::dirt},
    {tne::dirt, tne::dirt, tne::dirt},
    {tne::sand, tne::sand, tne::sand},
    {tne::stone, tne::stone, tne::stone},
    {tne::log_top, tne::log_side, tne::log_top},
    {tne::leaf, tne::leaf, tne::leaf},
    {tne::wool_red, tne::wool_red, tne::wool_red},  
    {tne::wool_yellow, tne::wool_yellow, tne::wool_yellow},
    {tne::wool_black, tne::wool_black, tne::wool_black}
};

static void initTextures() {
    Image img;
    std::string tfName;
    std::string cTp;

    for(int i=0; i<texC; i++) {
        cTp = (FileExists(TextFormat("texturePacks/%s/%s.png", tpName.c_str(), tNames[i].c_str()))) ?
            tpName.c_str() :
            "default" ;

        if(FileExists(TextFormat("texturePacks/%s/%s.png", tpName, tNames[i])))
            std::cout << tNames[i] << " not exsits\n";

        UnloadTexture(textures[i]);
        img = LoadImage(TextFormat("texturePacks/%s/%s.png", cTp.c_str(), tNames[i].c_str()));
        ImageFlipVertical(&img);

        textures[i] = LoadTextureFromImage(img);
        UnloadImage(img);
    }

}

static const int flippableC = 4,
    flippable[flippableC] = {bt_dirt,bt_grass,bt_log,bt_sand};

bool isFlippable(int type) {
    for(int i=0; i<flippableC; i++) {
        if(flippable[i] == type)
            return 1;
    }
    return 0;
}

void drawCubeTextureFaces(Vector3 position, Faces faces, int type) {

    if(!faces.back && !faces.bottom && !faces.front && !faces.left && !faces.right && !faces.top)
        return;

    int fx = 0;

    if(isFlippable(type)) {
        SetRandomSeed((unsigned int) (abs(round((int) position.x % 2 == 0 ? position.y : position.z))));
        fx = GetRandomValue(0,1);
    }

    float
        x = position.x,
        y = position.y,
        z = position.z,

        width = 1.0f,
        height = 1.0f,
        length = 1.0f;

    int c[3] = {(int)(255*0.8f), 255, (int)(255*0.9f)}; //{230, 255, 204};

    rlCheckRenderBatchLimit(3*(faces.back + faces.bottom + faces.front + faces.left + faces.right + faces.top));

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

            rlColor4ub(c[0], c[0], c[0], 255);
            // Front Face
            rlSetTexture(textures[blockTextureI[type][1]].id);
            if(faces.back) {
                rlNormal3f(0.0f, 0.0f, 1.0f);                  // Normal Pointing Towards Viewer
                rlTexCoord2f(0.0f+fx, 0.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad
                rlTexCoord2f(1.0f-fx, 0.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad
                rlTexCoord2f(1.0f-fx, 1.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Right Of The Texture and Quad
                rlTexCoord2f(0.0f+fx, 1.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Left Of The Texture and Quad    
            }

            // Back Face
            //if(blockTextureI[type][5] != blockTextureI[type][4]) rlSetTexture(textures[blockTextureI[type][5]].id);
            if(faces.front) {
                rlNormal3f(0.0f, 0.0f, - 1.0f);                  // Normal Pointing Away From Viewer
                rlTexCoord2f(1.0f-fx, 0.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Right Of The Texture and Quad
                rlTexCoord2f(1.0f-fx, 1.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad
                rlTexCoord2f(0.0f+fx, 1.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad
                rlTexCoord2f(0.0f+fx, 0.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Left Of The Texture and Quad
            }

            rlColor4ub(c[1], c[1], c[1], 255);
            // Top Face
            if(blockTextureI[type][0] != blockTextureI[type][1]) rlSetTexture(textures[blockTextureI[type][0]].id);
            if(faces.top) {
                rlNormal3f(0.0f, 1.0f, 0.0f);                  // Normal Pointing Up
                rlTexCoord2f(0.0f+fx, 1.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad
                rlTexCoord2f(0.0f+fx, 0.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);  // Bottom Left Of The Texture and Quad
                rlTexCoord2f(1.0f-fx, 0.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);  // Bottom Right Of The Texture and Quad
                rlTexCoord2f(1.0f-fx, 1.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad    
            }

            rlColor4ub(c[2], c[2], c[2], 255);

            // Bottom Face
            if(blockTextureI[type][2] != blockTextureI[type][0]) rlSetTexture(textures[blockTextureI[type][2]].id);
            if(faces.bottom) {
                rlNormal3f(0.0f, - 1.0f, 0.0f);                  // Normal Pointing Down
                rlTexCoord2f(1.0f-fx, 1.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);  // Top Right Of The Texture and Quad
                rlTexCoord2f(0.0f+fx, 1.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);  // Top Left Of The Texture and Quad
                rlTexCoord2f(0.0f+fx, 0.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad
                rlTexCoord2f(1.0f-fx, 0.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad
            }

            // Right face
            if(blockTextureI[type][1] != blockTextureI[type][2]) rlSetTexture(textures[blockTextureI[type][1]].id);
            if(faces.right) {
                rlNormal3f(1.0f, 0.0f, 0.0f);                  // Normal Pointing Right
                rlTexCoord2f(1.0f-fx, 0.0f); rlVertex3f(x + width/2, y - height/2, z - length/2);  // Bottom Right Of The Texture and Quad
                rlTexCoord2f(1.0f-fx, 1.0f); rlVertex3f(x + width/2, y + height/2, z - length/2);  // Top Right Of The Texture and Quad
                rlTexCoord2f(0.0f+fx, 1.0f); rlVertex3f(x + width/2, y + height/2, z + length/2);  // Top Left Of The Texture and Quad
                rlTexCoord2f(0.0f+fx, 0.0f); rlVertex3f(x + width/2, y - height/2, z + length/2);  // Bottom Left Of The Texture and Quad
            }

            // Left Face
            //if(blockTextureI[type][0] != blockTextureI[type][1]) rlSetTexture(textures[blockTextureI[type][0]].id);
            if(faces.left) {
                rlNormal3f( - 1.0f, 0.0f, 0.0f);                  // Normal Pointing Left
                rlTexCoord2f(0.0f+fx, 0.0f); rlVertex3f(x - width/2, y - height/2, z - length/2);  // Bottom Left Of The Texture and Quad
                rlTexCoord2f(1.0f-fx, 0.0f); rlVertex3f(x - width/2, y - height/2, z + length/2);  // Bottom Right Of The Texture and Quad
                rlTexCoord2f(1.0f-fx, 1.0f); rlVertex3f(x - width/2, y + height/2, z + length/2);  // Top Right Of The Texture and Quad
                rlTexCoord2f(0.0f+fx, 1.0f); rlVertex3f(x - width/2, y + height/2, z - length/2);  // Top Left Of The Texture and Quad    
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