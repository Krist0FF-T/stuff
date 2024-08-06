#include <iostream>

#include <raylib.h>
#include <raymath.h>

static const int
    W = 500,
    H = 500,
    CX = W/2,
    CY = H/2,
    
    pW = 40,
    pH = 60;

static const float
    tickTime = 0.05f,
    fontSize = 40.0f;

static float
    maxJumpH = 0.0f,
    maxJumpTime = 0.0f,
    maxVel = 0.0f,
    
    velY = 0.0f,
    
    tickTimer = 0.0f,
    dt = 0.0f,

    y = 256.0f,
    jumpTime = 0.0f;

static bool tick = 0, atj = 0, started = false;

int main() {

    InitWindow(W, H, "jump");

    while(!WindowShouldClose()) {
        dt = GetFrameTime();

        BeginDrawing();
            ClearBackground(BLACK);

            DrawText(TextFormat("velY: %.3f", velY), 10, 10, fontSize, WHITE);
            DrawText(TextFormat("y: %.3f", y), 10, 10+fontSize*1, fontSize, WHITE);
            DrawText(TextFormat("jumpH: %.3f", maxJumpH), 10, 10+fontSize*2, fontSize, WHITE);
            DrawText(TextFormat("jumpTime: %.3f", maxJumpTime), 10, 10+fontSize*3, fontSize, WHITE);
            DrawText(TextFormat("maxVel: %.3f", maxVel), 10, 10+fontSize*4, fontSize, WHITE);


        EndDrawing();

        if(IsKeyPressed(KEY_SPACE))
            started = 1;
        
        if(!started)
            continue;

        tick = false;
        tickTimer += dt;

        if(tickTimer >= tickTime) {
            tickTimer = 0.0f;
            tick = true;
        }

        if(tick) {
            velY -= 0.08f;
            velY *= 0.98f;

            // printf("tick\n");
        }

        float dy = velY*0.05f;

        if(y+dy < 0.0f) {
            y = 0.0f;
            velY = 0.0f;
            dy = 0.0f;
            atj = 1;
            jumpTime = 0.0f;
        }

        y += dy;

        jumpTime += dt;

        if(IsKeyDown(KEY_SPACE) && atj) {
            velY = 0.5081f;
            atj = 0;
        }

        maxJumpH = __max(maxJumpH, y); 
        maxJumpTime = __max(maxJumpTime, jumpTime);
        maxVel = __max(maxVel, abs(velY*20.0f));

    }

    CloseWindow();

    return 0;
}