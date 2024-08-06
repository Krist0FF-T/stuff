#include <stdio.h>
#include <stdlib.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include <iostream>

#include "blocktypes.h"
#include "draw.h"

#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)

#define dCos(x) (cosf(x * PI / 180))
#define dSin(x) (sinf(x * PI / 180))

#define sign(x) (x == 0 ? 0 : (x < 0 ? -1 : 1))

bool vec3eq(Vector3 v1, Vector3 v2) {
  return (v1.x == v2.x && v1.y == v2.y && v1.z == v2.z);
}

#define isIn(x, k1, k2) (x > k1 && x < k2)

float toRad(float deg) { return deg * PI / 180; }

bool chance(int m) { return GetRandomValue(1, m) == 1; }

bool chanceP(float percent) {
  return GetRandomValue(1, (int)round(percent / 100.0f)) == 1;
}

static Font font;

void DrawTextC(const char *text, Vector2 center, float fontSize, Color color) {
  DrawTextEx(
      font, text,
      {center.x - MeasureText(text, fontSize) / 2, center.y - fontSize / 2},
      fontSize, 0, color);
};

static const Color bgc{100, 180, 240, 255};

static const float gravity = 24.0f, fogDens = 0.05f;

static float dt, rotX = 0.0f, rotY = 90.0f, lookX, lookY, lookZ, velY;

static int rad = 3, cType = 0;

static bool p = 1, atj = 0, // able to jump
    running = true;

Texture2D LoadTex2D(const char *fName, int width, int height) {
  Image img = LoadImage(fName);
  ImageResize(&img, width, height);
  return LoadTextureFromImage(img);
}

#define colorChangeA(color, a) ((Color){color.r, color.g, color.b, a})

static Camera3D camera;
static Vector3 camTarget;

static const int W = 1280, H = 720, CX = W / 2, CY = H / 2,

                 mapH = 1, mapSize = 50,
                 // mapVol = mapSize * mapSize * mapSize,

    texR = 16;

static const bool trees = true, flat = false;

static int blocks[mapSize][mapH][mapSize], blockCount;

int getI(int x, int y, int z) {
  return x * mapSize * mapSize + y * mapSize + z;
}

bool onMap(int x, int y, int z) {
  return (isIn(x, -1, mapSize) && isIn(y, -1, mapH) && isIn(z, -1, mapSize));
}

int getAt(int x, int y, int z) {
  if (!onMap(x, y, z))
    return -1;
  return blocks[x][y][z];
}

void setAt(int x, int y, int z, int type) {
  if (onMap(x, y, z))
    blocks[x][y][z] = type;
}

bool isEmpty(int x, int y, int z) { return (blocks[x][y][z] == -1); }

bool posCollBlock(int x, int y, int z, Vector3 p) {
  float add = 0.5f;
  return (p.x + add >= (float)x && p.x <= (float)(x + 1) &&

          p.y + add >= (float)y && p.y <= (float)(y + 1) &&

          p.z + add >= (float)z && p.z <= (float)(z + 1));
}

Faces getFaces(int x, int y, int z) {
  float cx = camera.position.x, cy = camera.position.y, cz = camera.position.z;

  /*
      left,     // X -
      right,    // X +

      bottom,   // Y -
      top,      // Y +

      front,    // Z -
      back;     // Z +
  */

  return {
      x == 0 || (cx < x && isEmpty(x - 1, y, z)),
      x == mapSize - 1 || (cx > x && isEmpty(x + 1, y, z)),

      y == 0 || (cy < y && isEmpty(x, y - 1, z)),
      y == mapH - 1 || (cy > y && isEmpty(x, y + 1, z)),

      z == 0 || (cz < z && isEmpty(x, y, z - 1)),
      z == mapSize - 1 || (cz > z && isEmpty(x, y, z + 1)),

      // 1,1,1,1,1,1

  };
}

void genTree(int x, int y, int z) {
  if (getAt(x, y, z) == B_GREEN)
    return;
  int w = 5, h = 6, l = 5;

  // 5: dirt
  // 3: green

  int treeBlocks[w * h * l] = {
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

      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 8,  8,  8,  8,  8,
      8,  8,  8,  8,  8,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,

      -1, -1, 8,  -1, -1, -1, -2, 8,  -2, -1, 8,  8,  8,  8,  8,
      8,  8,  8,  8,  8,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,

      -1, 8,  8,  8,  -1, -1, 8,  8,  8,  -1, 8,  8,  7,  8,  8,
      8,  8,  7,  8,  8,  -1, -1, 7,  -1, -1, -1, -1, 7,  -1, -1,

      -1, -1, 8,  -1, -1, -1, -2, 8,  -2, -1, 8,  8,  8,  8,  8,
      8,  8,  8,  8,  8,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,

      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 8,  8,  8,  8,  8,
      8,  8,  8,  8,  8,  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,

  };

  int ind = 0;

  for (int j = 0; j < w; j++) {
    for (int i = h - 1; i >= 0; i--) {
      for (int k = 0; k < l; k++) {
        if (treeBlocks[ind] != -1 && !(treeBlocks[ind] == -2 && chance(2)) &&
            isEmpty(x + j - w / 2, y + i, z + k - l / 2)) {
          int type = treeBlocks[ind];
          if (type == -2)
            type = B_LEAF;
          setAt(x + j - w / 2, y + i, z + k - l / 2,
                type); // treeBlocks[x*l*h + y*w + z]
        }

        ind++;
      }
    }
  }
}

void genMap() {
  Image img1 = GenImageCellular(mapSize, mapSize, 64);
  Image img2 = GenImageCellular(mapSize, mapSize, 48);
  Image img3 = GenImageCellular(mapSize, mapSize, 32);

  Image heightMap = GenImageColor(mapSize, mapSize, WHITE);

  for (int x = 0; x < mapSize; x++) {
    for (int y = 0; y < mapSize; y++) {
      unsigned char g =
          (GetImageColor(img1, x, y).r + GetImageColor(img2, x, y).r +
           GetImageColor(img3, x, y).r) /
          3;
      ImageDrawPixel(&heightMap, x, y, {g, g, g, 255});
    }
  }

  UnloadImage(img1);
  UnloadImage(img2);
  UnloadImage(img3);

  float mult = 20;
  int add = 2;

  int type;
  int height = 1;

  // int types[3] = {B_STONE, B_GREEN, B_DIRT};

  for (int x = 0; x < mapSize; x++) {

    // height += chance(2) ? 1 : -1;
    // height = dCos(x*10)*(mapSize/4)+mapSize/2;

    // height = min(max(height, 10), mapSize-3);

    for (int y = 0; y < mapH; y++) {
      for (int z = 0; z < mapSize; z++) {

        if (!flat)
          height = (int)round(GetImageColor(heightMap, x, z).r / 255.0f * mult +
                              add);

        type = B_AIR;

        if (y == height)
          type = B_GREEN;

        else if (y < height)
          type = chance(10) ? B_STONE : B_DIRT;

        type = B_GREEN;

        blocks[x][y][z] = type;
      }
    }
  }

  if (trees) {

    int treeMap[mapSize][mapSize];

    for (int x = 0; x < mapSize; x++) {
      for (int y = 0; y < mapSize; y++) {
        treeMap[x][y] = 0;
      }
    }

    int sb = 5;

    bool tree = false;
    int emptyC = 0;

    for (int x = sb / 2; x < mapSize - sb / 2; x++) {
      for (int z = sb / 2; z < mapSize - sb / 2; z++) {
        tree = false;
        if (chance(20)) {
          emptyC = 0;

          for (int j = -sb; j < sb + 1; j++) {
            for (int i = -sb; i < sb + 1; i++) {
              int ax = x + j;
              int az = z + i;
              if (ax < 0 || ax > mapSize - 1 || az < 0 || az > mapSize - 1 ||
                  treeMap[ax][az] == 0)
                emptyC++;

              else
                break;
            }
          }

          if (emptyC == pow(sb * 2 + 1, 2)) {
            tree = true;
            if (!flat)
              height = (int)round(
                  GetImageColor(heightMap, x, z).r / 255.0f * mult + add);
            genTree(x, height + 1, z);
          }
        }

        treeMap[x][z] = tree;
        // printf("%i", tree);
      } /*printf("\n");*/
    }
  }

  UnloadImage(heightMap);
}

void initTextures() {
  Image tileSheet = LoadImage("img/blocks.png"), img;

  for (int i = 0; i < texC; i++) {
    img = GenImageColor(texR, texR, WHITE);

    for (int x = 0; x < texR; x++) {
      for (int y = 0; y < texR; y++) {
        ImageDrawPixel(&img, x, y,
                       GetImageColor(tileSheet, i * texR + x, (texR - 1) - y));
      }
    }

    textures[i] = LoadTextureFromImage(img);
    UnloadImage(img);
  }

  printf("loadTextures: done :D");

  UnloadImage(tileSheet);
}

void update() {
  blockCount = 0;

  // change rad
  rad += (IsKeyPressed(KEY_F) - IsKeyPressed(KEY_G));
  rad = min(max(rad, 1), 30);

  // change rotation
  rotX += GetMouseDelta().x /
          10.0f; //(IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT))*camera.fovy*dt;
  if (rotX > 360)
    rotX -= 360;
  if (rotX < 0)
    rotX += 360;

  cType += sign(GetMouseWheelMove());
  cType = min(max(cType, 0), blockTypeCount - 1);

  bool forward = IsKeyDown(KEY_W), back = IsKeyDown(KEY_S);

  // move
  velY -= gravity * dt;
  if (atj && IsKeyDown(KEY_SPACE)) {
    velY += 15.0f; // 20.0f;
    atj = false;
  }

  velY = max(velY, -20.0f);

  float dx = dCos(rotX) * 10 * dt * (forward - back);
  float dz = dSin(rotX) * 10 * dt * (forward - back);
  float dy = velY * dt; //(IsKeyDown(KEY_E) - IsKeyDown(KEY_Q))*10*dt;

  int cx = (int)round(camera.position.x);
  int cy = (int)round(camera.position.y);
  int cz = (int)round(camera.position.z);
  int testW = (int)ceilf(dx) + 1, testH = (int)ceilf(dy) + 1,
      testL = (int)ceilf(dz) + 1;

  for (int x = cx - testW; x < cx + testW; x++) {
    for (int y = cy - testH; y < cy + testH; y++) {
      for (int z = cz - testL; z < cz + testL; z++) {

        if (onMap(x, y, z) && blocks[x][y][z] != -1) {
          if (dx != 0.0f &&
              posCollBlock(x, y, z,
                           Vector3Add(camera.position, {dx, 0.0f, 0.0f}))) {
            dx = 0.0f;
          }

          if (velY != 0.0f &&
              posCollBlock(x, y, z,
                           Vector3Add(camera.position, {0.0f, dy, 0.0f}))) {

            dy = 0.0f;
            velY = 0.0f;
            if (camera.position.y > y)
              atj = 1; // make the player able to jump
          }

          if (dz != 0.0f &&
              posCollBlock(x, y, z,
                           Vector3Add(camera.position, {0.0f, 0.0f, dz})))
            dz = 0.0f;
        }
      }
    }
  }

  camera.position.x += dx;
  camera.position.y += dy;
  camera.position.z += dz;

  if (camera.position.y < -20)
    camera.position.y = mapSize;

  // view up & down
  rotY += GetMouseDelta().y / 10;
  rotY = min(max(rotY, 1.0f), 179.0f);
  camTarget.y = dCos(rotY) * rad + camera.position.y;
  camTarget.x = dSin(rotY) * dCos(rotX) * rad + camera.position.x;
  camTarget.z = dSin(rotY) * dSin(rotX) * rad + camera.position.z;

  camera.target = camTarget;

  // Toggle fullscreen
  if (IsKeyPressed(KEY_F11))
    ToggleFullscreen();

  SetMousePosition(CX, CY);

  lookX = round(camTarget.x);
  lookY = round(camTarget.y);
  lookZ = round(camTarget.z);

  if ((p && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) ||
      (!p && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))) {

    if (!isEmpty(lookX, lookY, lookZ))
      setAt(lookX, lookY, lookZ, -1);
  }

  if (onMap(lookX, lookY, lookZ) &&
      (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) * p ||
       IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) * (!p)) &&
      isEmpty((int)lookX, (int)lookY, (int)lookZ))
    setAt((int)lookX, (int)lookY, (int)lookZ, cType);
}

void draw3D() {

  int drawHeight = mapSize; // min(30, mapSize);

  // rlEnableWireMode();
  for (int x = 0; x < mapSize; x++) {
    for (int y = 0; y < mapH; y++) {
      for (int z = 0; z < mapSize; z++) {

        // i = getI(x,y,z);

        if (!isEmpty(x, y, z)) {
          // getFaces(x,y,z);
          drawCubeTextureFaces({(float)x, (float)y, (float)z},
                               {0, 0, 0, 1, 0, 0}, blocks[x][y][z]);

          // drawCubeColorFaces({(float) x, (float) y, (float) z},
          // getFaces(x,y,z), blockColors[getAt(x,y,z)]);
          // DrawCubeTexture(textures[0], {(float) x, (float) y, (float) z},
          // 1,1,1, WHITE);
          if (dt < 0.1f)
            blockCount++;
        }

        // i++;
      }
    }
  }

  DrawCubeWires({lookX, lookY, lookZ}, 1, 1, 1,
                onMap(lookX, lookY, lookZ) ? WHITE : RED);

  // rlDisableWireMode();
}

void draw2D() {
  DrawTextEx(font, TextFormat("fps: %i", (int)round(1 / dt)), {10, 10}, 20, 0,
             WHITE);
  DrawTextEx(font,
             TextFormat("xyz: %.1f %.1f %.1f", camera.position.x,
                        camera.position.y, camera.position.z),
             {10, 40}, 20, 0, WHITE);

  DrawTextEx(
      font,
      TextFormat("looking at: %i %i %i", (int)lookX, (int)lookY, (int)lookZ),
      {10, 70}, 20, 0, WHITE);

  DrawTextEx(font, TextFormat("blocks: %i", blockCount), {10, 100}, 20, 0,
             WHITE);

  DrawTextEx(font, blockNames[cType], {10, 130}, 20, 0, blockColors[cType]);

  DrawTextEx(font, TextFormat("Y velocity: %.1f m/s", velY), {10, 160}, 20, 0,
             WHITE);
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
  // SetWindowState(FLAG_VSYNC_HINT);

  HideCursor();

  font = LoadFont("fffforwa.ttf");
  initTextures();

  camera.position = (Vector3){mapSize / 2, 20.0f, mapSize / 2};
  camera.target = (Vector3){0.0f, 0.0f, 0.0f};
  camera.up = (Vector3){0.0f, 1.0f, 0.0f};
  camera.fovy = 75.0f;
  camera.projection = CAMERA_PERSPECTIVE;

  genMap();
}

void deInit() {
  UnloadFont(font);

  for (int i = 0; i < 7; i++) {
    UnloadTexture(textures[i]);
  }

  CloseWindow();
}

int main() {
  init();

  // main loop
  while (running) {

    dt = GetFrameTime();
    // UpdateCamera(&camera);

    draw();

    if (dt < 0.1f) {
      update();
    }

    running = !WindowShouldClose();
  }

  deInit();
}
