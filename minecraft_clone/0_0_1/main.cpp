// built in
#include <math.h>
#include <stdio.h>
// 3rd party
#include <raylib.h>
#include <raymath.h>
// own
#include "second.h"

int main(void) {
    init();
    HideCursor();    

    unsigned char m = 255;
    int typeC = 9;

    const char* names[typeC] {
        "T.N.T.",
        "Sand",
        "Stone",
        "Dirt",
        "Grass",
        "White",
        "Black",
        "Red",
        "Glass"
    };

    Color colors[typeC] {
        {0,0,0,m},       // bombaaa
        {194,178,128,m}, // sand
        {120,120,120,m}, // gray
        {140,65,35,m},   // brown
        {0,255,0,m},     // green
        {255,255,255,m}, // white
        {0,0,0,m},       // black
        {255,0,0,m},     // red
        {0,180,230,120}, // glass
    };

    Color bgc{100,180,240,m};

    Image img = GenImageChecked(4, 4, 1, 1, RED, WHITE);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);

    bool p=0;

    int cType = 0;

    Vector3 lookAt = Vector3Zero();

    while (!WindowShouldClose()) {
        dt = GetFrameTime();
        lookAt.x = round(camTarget.x);
        lookAt.y = round(camTarget.y);
        lookAt.z = round(camTarget.z);

        UpdateCamera(&camera);

        BeginDrawing();

            ClearBackground(bgc);

            BeginMode3D(camera);
                for(int i=0; i<maxBlocks; i++) {
                    if(blocks[i].type == -1) continue;

                    float dist = (
                        abs(camera.position.x - blocks[i].pos.x) +
                        abs(camera.position.y - blocks[i].pos.y) +
                        abs(camera.position.z - blocks[i].pos.z)
                    );

                    if(dist > 512.0f) continue;
                    blocks[i].faces = getFaces(&blocks[i].pos);

                    if(blocks[i].type != 0) {
                        drawCubeFaces(blocks[i].pos, blocks[i].faces, colors[blocks[i].type]);
                        //DrawCube(blocks[i].pos, 1, 1, 1, colors[blocks[i].type]);
                        //if(dist < 128.0f) DrawCubeWires(blocks[i].pos, 1, 1, 1, BLACK);

                    } else {
                        DrawCubeTexture(tex, blocks[i].pos, 1.0f, 1.0f, 1.0f, WHITE);
                    }

                    

                }

                DrawCubeWires(lookAt, 1, 1, 1, WHITE);
                //DrawCubeTexture(tex, lookAt, ts, ts, ts, WHITE);

            EndMode3D();

            DrawTextEx(font, TextFormat("fps: %i", GetFPS()), {10, 10}, 20, 0, textColor);

            DrawTextEx(font, TextFormat("xyz: %.1f, %.1f, %.1f", camera.position.x, camera.position.y, camera.position.z),
                            {10, 40}, 20, 0, textColor);

            DrawTextEx(font, TextFormat("rot: %.1f; %.1f", rotation, h), {10, 70}, 20, 0, textColor);

            DrawTextEx(font, TextFormat("p: %i", p), {10, 100}, 20, 0, textColor);

            DrawTextEx(font, names[cType], {10, 130}, 20, 0, colors[cType]);
            DrawTextEx(font, TextFormat("blocks: %i", cBlocks), {10, 160}, 20, 0, textColor);

            DrawTextEx(font, TextFormat("looking at: %i, %i, %i", (int)lookAt.x, (int)lookAt.y, (int)lookAt.z),
                        {10,190}, 20, 0, textColor);


        EndDrawing();        

        // change current type
        cType += (IsKeyPressed(KEY_X) - IsKeyPressed(KEY_C));
        cType = min(max(cType, 0), typeC-1);

        // change reach
        rad -= (IsKeyPressed(KEY_F) - IsKeyPressed(KEY_G));
        rad = max(rad, 2.0f);

        // change rotation
        rotation += GetMouseDelta().x/10;//(IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT))*camera.fovy*dt;
        if(rotation > 360) rotation -= 360;
        if(rotation < 0) rotation += 360;

        // move back & forward
        dx = dCos(rotation);
        dy = dSin(rotation);

        bool forward = IsKeyDown(KEY_W),
             back    = IsKeyDown(KEY_S);

        camera.position.x += dx*10*dt*(forward-back);
        camera.position.z += dy*10*dt*(forward-back);

        // move up & down
        camera.position.y += (IsKeyDown(KEY_E) - IsKeyDown(KEY_Q))*10*dt;

        // view up & down
        h += GetMouseDelta().y/10; //(IsKeyDown(KEY_UP) - IsKeyDown(KEY_DOWN))*dt*camera.fovy/3;
        h = min(max(h, 1.0f), 179.0f);

        camTarget.y = dCos(h)*rad + camera.position.y;

        camTarget.x = dSin(h)*dCos(rotation)*rad+camera.position.x;
        camTarget.z = dSin(h)*dSin(rotation)*rad+camera.position.z;

        //camTarget.y = h;

        camera.target = camTarget;

        // Toggle fullscreen
        if(IsKeyPressed(KEY_F11)) ToggleFullscreen();

        // Zoom  ======================
        if(IsKeyDown(KEY_C)) 
            camera.fovy += GetMouseWheelMove()*2.0f;
        
        if(IsKeyReleased(KEY_C)) camera.fovy = 60.0f;

        if(IsKeyPressed(KEY_P)) p = !p;

        if((IsMouseButtonDown(MOUSE_BUTTON_RIGHT)*p || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)*(!p)) && lookAt.y > -1.0f) {
            if(isEmpty(lookAt)) {
                for(int i=0; i<maxBlocks; i++) {
                    if(blocks[i].type < 0) {
                        blocks[i].type = cType;
                        blocks[i].pos = {lookAt.x, lookAt.y, lookAt.z};
                        break;
                    }
                }
            }

            else {
                for(int i=0; i<maxBlocks; i++) {
                    if(blocks[i].type != 0 || !vec3eq(lookAt, blocks[i].pos)) continue;

                    for(int j=0; j<maxBlocks; j++) {
                        if(blocks[j].type < 0) continue;

                        if(Vector3Distance(blocks[i].pos, blocks[j].pos) < 6) {
                            blocks[j].type = -1;
                        }
                    }
                }
            }

        } else if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)*p || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)*(!p)) {

            if(!isEmpty(lookAt)) {
                for(int i=0; i<maxBlocks; i++) {
                    if(blocks[i].type < 0 || !vec3eq(lookAt, blocks[i].pos)) continue;

                    blocks[i].type = -1;
                    blocks[i].pos = {0.0f, 0.0f};

                }
            }

        } else if(IsKeyPressed(KEY_O)) {
            for(int y=0; y<6; y++) {
                for(int k=0; k<2; k++) {
                    int s = (y+k)*2+3;
                    for(int x = -s/2; x<s/2+1; x++) {
                        for(int z = -s/2; z<s/2+1; z++) {
                            if(abs(z) < s/2 && abs(x) < s/2 || (y==6 && k==1)) continue;
                            Vector3 pos{lookAt.x+(float)x, lookAt.y+(float)y, lookAt.z+(float)z};
                            if(!isEmpty(pos)) continue;
                            
                            for(int i=0; i<maxBlocks; i++) {
                                if(blocks[i].type == -1) {
                                    blocks[i].type = 2;
                                    blocks[i].pos = pos;
                                    break;
                                }
                            }


                        }
                    }
                }
                

            }
        }

        SetMousePosition(CX,CY);
        update();
    }

    UnloadTexture(tex);
    deInit();
    UnloadFont(font);
    CloseWindow();

    return 0;
}