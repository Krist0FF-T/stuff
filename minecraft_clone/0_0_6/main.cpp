
#include <random>
#include <stdio.h>
#include <stdlib.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#define GLSL_VERSION 330

#include <fstream>
// #include <iostream>
#include <algorithm>
// #include <chrono>
// #include <io.h>
#include <unordered_map>

// own
#include "blocktypes.h"
#include "commands.h"
#include "draw.h"
#include "second.h"
#include "sound.h"
#include "third.h"
#include "world.h"

#define VERSION "0.0.6 (DEVMODE)"

static std::ofstream logF;

void cLog(int type, std::string str) {

  std::vector<std::string> s{"", "[+]", "[-]"};

  logF << s[type] << ' ' << str << '\n';
}

static const float gravity = 9.8f, playerW = 0.6f, playerH = 1.8f,

                   playerEye = playerH * 0.4f,

                   defFovY = 90.0f, reachStep = 0.01f, defPlayerSpeed = 5.612f;

static float reach = 5.0f;

static float playerSpeed = defPlayerSpeed;

static float dt, rotX = 0.0f, rotY = 90.0f, velY,

                 tickTime = 0.05f, tickTimer = 0.0f,

                 fallTime = 0.0f, maxFallTime = 0.0f;

static Cube pCube{{0, 0, 0}, {playerW, playerH, playerW}};

static int rad = 3, cType = 0, cRenderDist = 2;

static bool p = 0,
            atj = 0, // able to jump
    skyEnabled = 0, running = true, paused = false, fall = true,
            smoothLighting = true, flying = false;

static int tick = 0;

static Model skybox;

static Camera3D camera;
static Vector3 camTarget;

static const int cTypeShowSize = (float)(H / 13.5f), fontSize = H / 40;

static Vector3 lookAt = {0, maxChunks *chunkSize - 3, 0};

enum gameStates {
  GS_MAIN_MENU = 0,
  GS_PLAY,
  GS_SELECT_BLOCK,
  GS_COMMAND,
  GS_PAUSE,
};

static int gameState = GS_MAIN_MENU;

static std::vector<std::string> textList;
static float textCd = 0.0f;

void newText(std::string txt) {
  if (textList.size() == 0)
    textCd = 2.0f;
  textList.push_back(txt);
}

// crosshair
static const int chSize = H / 72, chLineWidth = H / 270;

void drawCrosshair(Color color) {
  DrawLineEx({(float)CX, (float)CY - chSize}, {(float)CX, (float)CY + chSize},
             chLineWidth, color);
  DrawLineEx({(float)CX - chSize, (float)CY}, {(float)CX + chSize, (float)CY},
             chLineWidth, color);
}

static const std::string fName = "world";

void drawSky() {
  float rot = (float)GetTime() * 3.2f;

  rlDisableBackfaceCulling();
  rlDisableDepthMask();
  rlRotatef(rot, 0, 1, 2);
  DrawModel(skybox, (Vector3){0, 0, 0}, 1.0f, WHITE);
  rlEnableBackfaceCulling();
  rlEnableDepthMask();

  rlRotatef(-rot, 0, 1, 2);
}

std::string guiInputTxt(std::string displayText);

// declaration of pause function
void pause();
void blockSelectionMenu();

bool blockCollidePlayer(int x, int y, int z, Vector3 p) {

  float add = playerW;
  return (p.x > (float)(x - add) && p.x < (float)(x + add) &&

          p.y > (float)(y - playerH * 0.25f) &&
          p.y < (float)(y + playerH * 1.75f) &&

          p.z > (float)(z - playerW / 2) && p.z < (float)(z + playerW / 2));
}

void placePlayer() {

  bool placed = false;

  while (!placed) {
    int x = rand() % 32;
    int z = rand() % 32;

    for (int y = maxChunks * chunkSize - 1; y > -1 && !placed; y--) {
      placed = true;
      for (int a = 1; a < 3 && placed; a++) {
        if (!isEmpty(x, y + a, z))
          placed = false;
      }

      if (isEmpty(x, y, z))
        placed = false;

      if (placed) {
        pCube.pos.x = x;
        pCube.pos.y = y + playerH;
        pCube.pos.z = z;
      }
    }
  }
}

void genWorld() {
  std::string input = guiInputTxt("World generation seed");
  bool err = false;

  int seed = rand();
  try {
    seed = std::stoi(input);
  } catch (const std::invalid_argument &e) {
    printf("%s\n", e.what());
  } catch (const std::out_of_range &e) {
    printf("%s\n", e.what());
  }

  genMap2(seed);

  fallingBlocks.clear();
  placePlayer();
}

void update() {

  // tick
  tick = 0;
  tickTimer += dt;
  while (tickTimer >= tickTime) {
    tickTimer -= tickTime;
    tick++;
  }
  // ----

  textCd -= dt;
  if (textCd < 0.0f && textList.size() > 0) {
    textList.erase(textList.begin());
    textCd = 1.0f;
  }

  // blockCount = 0;

  cType += sign(GetMouseWheelMove());
  cType = min(max(cType, 0), blockTypeCount - 1);

  bool forward = IsKeyDown(KEY_W), back = IsKeyDown(KEY_S),
       right = IsKeyDown(KEY_D), left = IsKeyDown(KEY_A),

       up = IsKeyDown(KEY_SPACE), down = IsKeyDown(KEY_LEFT_SHIFT);

  // move ===

  // jump
  if (atj && up) {
    velY = 0.5081f;
    atj = false;
  }

  if (abs(velY) > 0.1f)
    atj = false;

  if (pCube.pos.y < -10)
    flying = true;

  if (flying)
    velY = (up - down) * playerSpeed * tickTime;

  // -jump

  for (int i = 0; i < tick; i++) {
    if (!flying) {
      velY -= 0.08f;
      velY *= 0.98f;
    }

    // player chunk
    int pc = pCube.pos.y / chunkSize, type;

    for (int j = 0; fall && j < fallingBlocks.size(); j++) {
      FallingB *fb = &fallingBlocks[j];
      fb->vel -= 0.08f;
      fb->vel *= 0.98f;
      if (fb->pos.y < (int)camera.position.y - 1000) {
        fallingBlocks.erase(fallingBlocks.begin() + j);
        j--;
        blockCount--;
      }
      // fb->pos.y += fb->vel;
    }

    for (int c = min(max(pc - 1, 0), maxChunks - 1);
         c < min(max(pc + 1, 0), maxChunks - 1) + 1; c++) {
      for (int x = 0; x < chunkSize; x++) {
        for (int y = 0; y < chunkSize; y++) {
          for (int z = 0; z < chunkSize; z++) {
            type = blocks[c][x][y][z];
            if (type == bt_air)
              continue;

            switch (type) {

            case bt_grass: {
              if (getAt(x, y + c * chunkSize + 1, z) != bt_air)
                blocks[c][x][y][z] = bt_dirt;

              break;
            }

            case bt_dirt: {
              if (getAt(x, y + 1 + c * chunkSize, z) != bt_air)
                break;

              // grass neighbours
              int gn = 0;

              for (int x2 = -1; x2 < 2; x2++) {
                for (int y2 = -1; y2 < 2; y2++) {
                  for (int z2 = -1; z2 < 2; z2++) {
                    if (x2 + z2 == 0 ||
                        !onMap(x + x2, y + y2 + c * chunkSize, z + z2))
                      continue;
                    if (getAt(x + x2, y + y2 + c * chunkSize, z + z2) ==
                        bt_grass) {
                      gn++;
                    }
                  }
                }
              }

              if (gn > 0 &&
                  chanceP(
                      gn)) { // || gn > 0 && chance((int)(1.0f/(float)gn))) {
                setAt(x, y + c * chunkSize, z, bt_grass);
              }

              break;
            }

            case bt_sand: {
              if (c == 0 && y == 0)
                break;
              if (getAt(x, y + c * chunkSize - 1, z) == bt_air) {

                fallingBlocks.push_back((FallingB){
                    0.0f,
                    (Vector3){(float)x, (float)(y + c * chunkSize), (float)z},
                    type});

                setAt(x, y + c * chunkSize, z, bt_air);
              }
              break;
            }

            default: {
              break;
            }
            }
          }
        }
      }
    }
  }

  for (int j = 0; j < fallingBlocks.size(); j++) {
    FallingB *fb = &fallingBlocks[j];
    float dy = fb->vel * dt * 20.0f;
    if (getAt(fb->pos.x, (int)(fb->pos.y + dy), fb->pos.z) == bt_air) {
      if (fall)
        fb->pos.y += dy;
      continue;
    }

    else {
      // if(blockCubeV(fb->pos).collide(blockCube((int)fb->pos.x,
      // (int)(fb->pos.y-1.0f), (int)fb->pos.z))) {
      setAt(fb->pos.x, (int)(fb->pos.y - dy), fb->pos.z, fb->type);
      fallingBlocks.erase(fallingBlocks.begin() + j);
      j--;
      continue;
    }
  }

  float dx =
      (dCos(rotX) * (forward - back) + dCos((rotX + 90.0f)) * (right - left)) *
      playerSpeed * dt;
  float dz =
      (dSin(rotX) * (forward - back) + dSin((rotX + 90.0f)) * (right - left)) *
      playerSpeed * dt;

  // dx += dCos((rotX+90.0f))*playerSpeed*dt*(right-left);
  // dz += dSin((rotX+90.0f))*playerSpeed*dt*(right-left);
  float dy = velY * dt * 20.0f; // velY*dt;

  int testW = (int)ceilf(dx) + 1, testH = (int)ceilf(dy) + 1,
      testL = (int)ceilf(dz) + 1;

  for (int x = (int)fFloor(pCube.getTLF().x) - 1;
       x < (int)fCeil(pCube.getBRB().x) + 2; x++) {
    for (int y = (int)fFloor(pCube.getTLF().y) - 1;
         y < (int)fCeil(pCube.getBRB().y) + 2 + testH + 1; y++) {
      for (int z = (int)fFloor(pCube.getTLF().z) - 1;
           z < (int)fCeil(pCube.getBRB().z) + 2 + testL + 1; z++) {

        if (onMap(x, y, z) && !isEmpty(x, y, z)) {
          Cube cube = blockCube(x, y, z);

          if (pCube.movedCopy(dx, 0, 0).collide(cube)) {
            dx = 0;
          }

          if (pCube.movedCopy(0, dy, 0).collide(cube)) {
            dy = 0;
            velY = 0;

            if (pCube.getTLF().y + playerH > (float)y) {
              atj = true;
              if (getAt(x, y, z) == bt_slime)
                velY = 4;
            }
          }

          if (pCube.movedCopy(0, 0, dz).collide(cube)) {
            dz = 0;
          }
        }
      }
    }
  }

  pCube.pos.x += dx;
  pCube.pos.y += dy;
  pCube.pos.z += dz;

  if (!flying) {
    pCube.pos.x = min(max(pCube.pos.x, 0.0f), chunkSize - 1.0f);
    pCube.pos.z = min(max(pCube.pos.z, 0.0f), chunkSize - 1.0f);
  }

  // if(!flying && pCube.pos.y < -10) pCube.pos.y = chunkSize*maxChunks+2;

  // pCube.pos = {camera.position.x, camera.position.y+1, camera.position.z};
  camera.position = {pCube.pos.x, pCube.pos.y + playerEye, pCube.pos.z};

  // view up & down

  rotX += GetMouseDelta().x *
          0.1f; //(IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT))*camera.fovy*dt;
  if (rotX > 360)
    rotX -= 360;
  if (rotX < 0)
    rotX += 360;

  rotY += GetMouseDelta().y / 10;
  rotY = min(max(rotY, 0.1f), 179.9f);
  camTarget.y = dCos(rotY) * rad + camera.position.y;
  camTarget.x = dSin(rotY) * dCos(rotX) * rad + camera.position.x;
  camTarget.z = dSin(rotY) * dSin(rotX) * rad + camera.position.z;

  camera.target = camTarget;

  // Toggle fullscreen
  if (IsKeyPressed(KEY_F11))
    ToggleFullscreen();

  SetMousePosition(CX, CY);

  // zoom
  if (IsKeyPressed(KEY_C))
    camera.fovy = defFovY * 0.3f;
  if (IsKeyReleased(KEY_C))
    camera.fovy = defFovY;
  // ---- :D

  Vector3 placeAdd{0, 0, 0};
  for (float cRad = 0; cRad < reach + reachStep; cRad += reachStep) {
    Vector3 lookAtf{dSin(rotY) * dCos(rotX) * cRad + camera.position.x,
                    dCos(rotY) * cRad + camera.position.y,
                    dSin(rotY) * dSin(rotX) * cRad + camera.position.z};
    lookAt = (Vector3){roundf(lookAtf.x), roundf(lookAtf.y), roundf(lookAtf.z)};

    // block found
    if (!isEmptyV(lookAt)) {

      dx = lookAtf.x - lookAt.x;
      dy = lookAtf.y - lookAt.y;
      dz = lookAtf.z - lookAt.z;

      int x = lookAt.x;
      int y = lookAt.y;
      int z = lookAt.z;

      float add = 0.5f - reachStep;

      if (dx < -add && isEmpty(x - 1, y, z))
        placeAdd = (Vector3){-1, 0, 0};
      else if (dx > add && isEmpty(x + 1, y, z))
        placeAdd = (Vector3){1, 0, 0};
      else if (dy < -add && isEmpty(x, y - 1, z))
        placeAdd = (Vector3){0, -1, 0};
      else if (dy > add && isEmpty(x, y + 1, z))
        placeAdd = (Vector3){0, 1, 0};
      else if (dz < -add && isEmpty(x, y, z - 1))
        placeAdd = (Vector3){0, 0, -1};
      else if (dz > add && isEmpty(x, y, z + 1))
        placeAdd = (Vector3){0, 0, 1};

      break;
    }

    if (!onMapV(lookAt)) {
      lookAt.x = -1;
      placeAdd = (Vector3){0, 0, 0};
      break;
    }

    if (cRad >= reach) {
      lookAt.x = -1;
      placeAdd = (Vector3){0, 0, 0};
    }
  }

  if (IsKeyPressed(KEY_P))
    p = !p;

  if (IsKeyPressed(KEY_T)) {
    pCube.pos = tpPoint;
    newText(TextFormat("Teleported to: %.1f, %.1f, %.1f", tpPoint.x, tpPoint.y,
                       tpPoint.z));
  }

  Vector3 placeAt = Vector3Add(lookAt, placeAdd);

  if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) && onMapV(lookAt) &&
      !isEmptyV(lookAt))
    cType = getAtV(lookAt);

  if ((IsMouseButtonDown(MOUSE_BUTTON_LEFT) * p) ||
      (!p && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))) {

    if (!isEmpty((int)lookAt.x, (int)lookAt.y, (int)lookAt.z) &&
        onMapV(lookAt) && onMapV(lookAt)) {
      // set to air (delete)
      setAtV(lookAt, bt_air);
      blockCount--;
      // PlaySound(sounds[sid_break]);
    }
  }

  if (onMapV(placeAt) &&
      (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) * p ||
       IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) * (!p)) &&
      isEmptyV(placeAt) && !pCube.collide(blockCubeV(placeAt))) {
    setAtV(placeAt, cType);
    // fallingBlocks.push_back({0, {placeAt.x, placeAt.y+0.5f, placeAt.z},
    // cType});
    blockCount++;
    // PlaySound(sounds[sid_place]);
  }

  if (onMapV(placeAt) && IsKeyPressed(KEY_I))
    genTree(placeAt.x, placeAt.y, placeAt.z);

  if (IsKeyPressed(KEY_ESCAPE))
    pause();
  // paused = true;

  if (IsKeyPressed(KEY_G))
    fall = !fall;

  if (IsKeyDown(KEY_Q) && lookAt.x != -1) {
    char *b = &blocks[chunkOf(lookAt.y)][(int)lookAt.x]
                     [(int)lookAt.y % chunkSize][(int)lookAt.z];
    *b = cType;
  }

  if (IsKeyPressed(KEY_F2))
    takeScreenshot();

  if (IsKeyDown(KEY_F3)) {

    // toggle flying
    if (IsKeyPressed(KEY_F))
      flying = !flying;

    // save world
    if (IsKeyPressed(KEY_S)) {
      std::string fName = guiInputTxt("save as");
      if (fName != "" && saveWorld(fName, &pCube) == 0) {
        printf("[+] worlds saved.\n");
        newText("World saved");
      } else {
        newText("Failed saving world");
        printf("[ERROR] world NOT saved.\n");
      }
    }

    // load world
    if (IsKeyPressed(KEY_L)) {
      std::string fName = guiInputTxt("load from");
      if (fName != "" && loadWorld(fName, &pCube) == 0) {
        velY = 0;
        newText("World loaded");
        printf("[+] world loaded.\n");

      } else {
        newText("Failed loading world");
        printf("[ERROR] world NOT loaded.\n");
      }
    }

    // Toggle smooth lighting
    if (IsKeyPressed(KEY_M))
      smoothLighting = !smoothLighting;

    // Generate world
    if (IsKeyPressed(KEY_G))
      genWorld();

    // clear world
    if (IsKeyPressed(KEY_C))
      clearWorld();

    // change texture pack
    if (IsKeyPressed(KEY_Z)) {
      tpName = guiInputTxt("texture pack name");
      initTextures();
    }

    // toggle sky
    if (IsKeyPressed(KEY_D)) {
      skyEnabled = !skyEnabled;
      if (skyEnabled) {
        printf("creating skybox...\n");
        skybox = createSkybox();
      }

      else {
        printf("unloading skybox...\n");
        UnloadShader(skybox.materials[0].shader);
        UnloadTexture(skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture);
        UnloadModel(skybox);
        skybox = (Model){0};
      }
    }

    if (IsKeyPressed(KEY_B)) {
      std::string command = guiInputTxt("command:");
      int cmdId;

      try {
        cmdId = commands.at(command);
      } catch (std::exception &e) {
        printf("[ERROR] %s\n", e.what());
        cmdId = -1;
      }

      printf("command ID: %i\n", cmdId);

      switch (cmdId) {

      case (-1): {
        break;
      }

      case COMMAND_FILL: {
        int x1, y1, z1, x2, y2, z2;
        char type;
        try {

          std::vector<std::string> inputPos1 =
              splitStr(guiInputTxt("pos1: [x] [y] [z]"), ' ');
          if (inputPos1.size() != 3) {
            newText("correct format: [x] [y] [z]");
            break;
          }

          std::vector<std::string> inputPos2 =
              splitStr(guiInputTxt("pos2: [x] [y] [z]"), ' ');
          if (inputPos2.size() != 3) {
            newText("correct format: [x] [y] [z]");
            break;
          }

          x1 = std::stoi(inputPos1[0]);
          y1 = std::stoi(inputPos1[1]);
          z1 = std::stoi(inputPos1[2]);

          x2 = std::stoi(inputPos2[0]);
          y2 = std::stoi(inputPos2[1]);
          z2 = std::stoi(inputPos2[2]);

          type = std::stoi(guiInputTxt("type"));

          if (type < -1 || type > blockTypeCount - 1) {
            newText("Type out of range.");
            break;
          }
        } catch (const std::invalid_argument &e) {
          newText(e.what());
          break;
        } catch (const std::out_of_range &e) {
          newText(e.what());
          break;
        }

        if (!onMap(x1, y1, z1) || !onMap(x2, y2, z2)) {
          newText("Not on map.");
          break;
        }

        Vector3 p1{(float)min(x1, x2), (float)min(y1, y2), (float)min(z1, z2)};

        Vector3 p2{(float)max(x1, x2), (float)max(y1, y2), (float)max(z1, z2)};

        int c, rY, oldType;

        for (int x = (int)p1.x; x < (int)p2.x + 1; x++) {
          for (int y = (int)p1.y; y < (int)p2.y + 1; y++) {
            c = (int)(y / chunkSize);
            rY = (y % chunkSize);
            for (int z = (int)p1.z; z < (int)p2.z + 1; z++) {
              oldType = blocks[c][x][rY][z];
              if (oldType != type) {
                if (oldType == bt_air)
                  blockCount++;

                else if (type == bt_air)
                  blockCount--;
              }
              blocks[c][x][rY][z] = type;

              // printf("%i placed at %i %i %i\n", (int) type, x, y, z);
            }
          }
        }

        break;
      }

      case COMMAND_REPLACE: {
        char t1, t2;

        try {
          t1 = std::stoi(guiInputTxt("from"));
          t2 = std::stoi(guiInputTxt("to"));
        } catch (std::exception &e) {
          printf("[ERROR] %s", e.what());
          newText(TextFormat("[ERROR] %s", e.what()));
        }

        for (int c = 0; c < maxChunks; c++) {
          for (int x = 0; x < chunkSize; x++) {
            for (int y = 0; y < chunkSize; y++) {
              for (int z = 0; z < chunkSize; z++) {
                if (blocks[c][x][y][z] == t1)
                  blocks[c][x][y][z] = t2;
              }
            }
          }
        }

        break;
      }

      case COMMAND_TP: {
        int x, y, z;
        try {
          std::vector<std::string> input =
              splitStr(guiInputTxt("teleport to: [x] [y] [z]"), ' ');

          if (input.size() != 3) {
            newText("Enter 3 numbers please");
            break;
          }

          x = std::stoi(input[0]);
          y = std::stoi(input[1]);
          z = std::stoi(input[2]);
        } catch (const std::invalid_argument &e) {
          newText(e.what());
          break;
        } catch (const std::out_of_range &e) {
          newText(e.what());
          break;
        }

        pCube.pos = {(float)x, (float)y, (float)z};

        break;
      }

      case COMMAND_REACH: {
        std::string input = guiInputTxt("reach value");
        try {
          reach = std::stof(input);
        } catch (std::exception &e) {
          printf("[ERROR] %s\n", e.what());
          newText(TextFormat("[ERROR] %s", e.what()));
        }
        break;
      }

      case COMMAND_SPEED: {
        std::string input = guiInputTxt("speed value (m/s)");
        try {
          playerSpeed = std::stof(input);
        } catch (std::exception &e) {
          printf("[ERROR] %s\n", e.what());
          newText(TextFormat("[ERROR] %s", e.what()));
        }
        break;
      }

      case COMMAND_RAND_REPLACE: {
        std::string input = guiInputTxt("speed value (m/s)");
        try {
          int rbid = std::stof(input);

          for (int c = 0; c < maxChunks; c++) {
            for (int y = 0; y < chunkSize; y++) {
              for (int x = 0; x < chunkSize; x++) {
                for (int z = 0; z < chunkSize; z++) {

                  if (blocks[c][x][y][z] == rbid) {
                    blocks[c][x][y][z] = rand() % blockTypeCount;
                  }
                }
              }
            }
          }

        } catch (std::exception &e) {
          printf("[ERROR] %s\n", e.what());
          newText(TextFormat("[ERROR] %s", e.what()));
        }
        break;
      }

      default: {
        newText("Invalid command.");
      }
      }
    }
  }

  if (IsKeyPressed(KEY_E))
    blockSelectionMenu();
}

Faces getFaces(int x, int y, int z) {
  float cx = camera.position.x, cy = camera.position.y, cz = camera.position.z;

  bool lt = false;

  return {
      x > 0 && (cx < x && (isEmpty(x - 1, y, z) ||
                           (lt && blockData[getAt(x - 1, y, z)].translucent))),
      x < chunkSize - 1 &&
          (cx > x && (isEmpty(x + 1, y, z) ||
                      (lt && blockData[getAt(x + 1, y, z)].translucent))),

      (cy < y && (isEmpty(x, y - 1, z) ||
                  (y > 0 && lt && blockData[getAt(x, y - 1, z)].translucent))),
      (cy > y &&
       (isEmpty(x, y + 1, z) || (y < chunkSize * maxChunks && lt &&
                                 blockData[getAt(x, y + 1, z)].translucent))),

      (cz < z && (isEmpty(x, y, z - 1) ||
                  (z > 0 && lt && blockData[getAt(x, y, z - 1)].translucent))),
      (cz > z &&
       (isEmpty(x, y, z + 1) ||
        (z < chunkSize && lt && blockData[getAt(x, y, z + 1)].translucent))),

  };
}

c4v getC4v(int x, int y, int z, int faceN) {
  return (smoothLighting ? genC4v(x, y, z, faceN) : (c4v){1, 1, 1, 1});
}

void draw3D() {
  if (skyEnabled)
    drawSky();

  int oldSeed = rand();

  int cc = (int)round(camera.position.y / chunkSize);

  bool lookUp = (rotY < 90.0f);

  // minimum
  int k1 = lookUp ? max(cc - 1, 0) : max(cc - cRenderDist, 0);
  // maximum
  int k2 =
      lookUp ? min(cc + cRenderDist, maxChunks - 1) : min(cc, maxChunks - 1);

  int fx = 0;

  float width = 1.0f, height = 1.0f, length = 1.0f;

  float b0 = 1,  // top
      b1 = 0.9f, // left, right
      b2 = 0.8f, // front, back
      b3 = 0.7f; // bottom

  float c;

  bool rgb = 1;
  int lastT = 0, t;

  Vector3 *cp = &camera.position;

  // rlCheckRenderBatchLimit((blockCount + fallingBlocks.size())*3);
  rlPushMatrix();
  rlBegin(RL_QUADS);

  for (int i = 0; i < fallingBlocks.size(); i++) {
    FallingB *fb = &fallingBlocks[i];

    if (0 && abs(fb->pos.y - camera.position.y) > cRenderDist * chunkSize)
      continue;

    Faces f{
        cp->x<fb->pos.x, cp->x> fb->pos.x,
        cp->y<fb->pos.y, cp->y> fb->pos.y,
        cp->z<fb->pos.z, cp->z> fb->pos.z,
    };

    drawCubeTextureFaces(fb->pos, f, fb->type);
  }

  for (int chunk = k1; chunk < k2 + 1; chunk++) {
    for (int x = 0; x < chunkSize; x++) {
      // i = y from chunk bottom
      for (int i = 0; i < chunkSize; i++) {
        int y = chunk * chunkSize + i;
        for (int z = 0; z < chunkSize; z++) {

          if (blocks[chunk][x][i][z] != bt_air) {
            // printf("%i,%i,%i,%i\n",chunk,x,y,z);
            Faces faces = getFaces(x, y, z);
            int type = blocks[chunk][x][i][z];
            if (!(faces.back || faces.bottom || faces.front || faces.left ||
                  faces.right || faces.top))
              continue;

            // drawCubeTextureFaces({(float) x, (float) y, (float) z}, faces,
            // type);

            /*  c0 = getLOf(x,y,z,0),
                c1 = getLOf(x,y,z,1),
                c2 = getLOf(x,y,z,2),
                c3 = getLOf(x,y,z,3),
                c4 = getLOf(x,y,z,4),
                c5 = getLOf(x,y,z,5);*/

            fx = 0;

            if (isFlippable(type)) {
              srand((unsigned int)(x % 8 + y % 8 + z % 8));
              srand(rand());
              fx = rand() % 2;
            }

            // === DRAW FACES ===

            // Top Face
            if (faces.top) {
              t = textures[blockData[type].sides[0]].id;
              if (lastT != t) {
                rlSetTexture(t);
                lastT = t;
              }
              c = 255;
              c4v vc = getC4v(x, y, z, face_top);

              rlNormal3f(0.0f, 1.0f, 0.0f); // Normal Pointing Up

              color4ubGF(vc.tl * b0);
              rlTexCoord2f(0.0f + fx, 0.0f);
              rlVertex3f(x - 0.5f, y + 0.5f, z - 0.5f); // Top Left of the Quad

              color4ubGF(vc.br * b0);
              rlTexCoord2f(0.0f + fx, 1.0f);
              rlVertex3f(x - 0.5f, y + 0.5f,
                         z + 0.5f); // Bottom Left Of The Quad

              color4ubGF(vc.tr * b0);
              rlTexCoord2f(1.0f - fx, 1.0f);
              rlVertex3f(x + 0.5f, y + 0.5f,
                         z + 0.5f); // Bottom Right Of The Quad

              color4ubGF(vc.bl * b0);
              rlTexCoord2f(1.0f - fx, 0.0f);
              rlVertex3f(x + 0.5f, y + 0.5f, z - 0.5f); // Top Right Of The Quad
            }

            // Right face
            t = textures[blockData[type].sides[1]].id;
            if (t != lastT &&
                (faces.right || faces.left || faces.back || faces.front)) {
              rlSetTexture(t);
              lastT = t;
            }

            if (faces.right) {
              c = 255;
              // c = getLOf(x+1,y,z,1);
              rlNormal3f(1.0f, 0.0f, 0.0f); // Normal Pointing Right
              // color4ubG(c*b1, 255, 0);

              c4v vc = getC4v(x, y, z, face_right);

              color4ubGF(vc.br * b1);
              rlTexCoord2f(1.0f - fx, 1.0f);
              rlVertex3f(x + 0.5f, y - 0.5f,
                         z - 0.5f); // Bottom Right Of The Texture and Quad

              color4ubGF(vc.tl * b1);
              rlTexCoord2f(1.0f - fx, 0.0f);
              rlVertex3f(x + 0.5f, y + 0.5f,
                         z - 0.5f); // Top Right Of The Texture and Quad

              color4ubGF(vc.bl * b1);
              rlTexCoord2f(0.0f + fx, 0.0f);
              rlVertex3f(x + 0.5f, y + 0.5f,
                         z + 0.5f); // Top Left Of The Texture and Quad

              color4ubGF(vc.tr * b1);
              rlTexCoord2f(0.0f + fx, 1.0f);
              rlVertex3f(x + 0.5f, y - 0.5f,
                         z + 0.5f); // Bottom Left Of The Texture and Quad
            }

            // Left Face
            // if(blockTextureI[type][0] != blockTextureI[type][1])
            // rlSetTexture(textures[blockTextureI[type][0]].id);
            if (faces.left) {
              c = 255;
              rlNormal3f(-1.0f, 0.0f, 0.0f); // Normal Pointing Left
              // c = getLOf(x-1,y,z,0);
              // color4ubG(c*b1, 255, 0);

              c4v vc = getC4v(x, y, z, face_left);

              color4ubGF(vc.br * b1);
              rlTexCoord2f(0.0f + fx, 1.0f);
              rlVertex3f(x - 0.5f, y - 0.5f,
                         z - 0.5f); // Bottom Left Of The Texture and Quad

              color4ubGF(vc.tr * b1);
              rlTexCoord2f(1.0f - fx, 1.0f);
              rlVertex3f(x - 0.5f, y - 0.5f,
                         z + 0.5f); // Bottom Right Of The Texture and Quad

              color4ubGF(vc.bl * b1);
              rlTexCoord2f(1.0f - fx, 0.0f);
              rlVertex3f(x - 0.5f, y + 0.5f,
                         z + 0.5f); // Top Right Of The Texture and Quad

              color4ubGF(vc.tl * b1);
              rlTexCoord2f(0.0f + fx, 0.0f);
              rlVertex3f(x - 0.5f, y + 0.5f,
                         z - 0.5f); // Top Left Of The Texture and Quad
            }

            // Front Face

            if (faces.back) {
              c = 255;
              // c = getLOf(x,y,z+1,5);
              // color4ubG(c*b2, 255, 0);
              rlNormal3f(0.0f, 0.0f, 1.0f); // Normal Pointing Towards Viewer

              c4v vc = getC4v(x, y, z, face_back);

              color4ubGF(vc.br * b2);
              rlTexCoord2f(0.0f + fx, 1.0f);
              rlVertex3f(x - 0.5f, y - 0.5f,
                         z + 0.5f); // Bottom Left Of The Texture and Quad

              color4ubGF(vc.tr * b2);
              rlTexCoord2f(1.0f - fx, 1.0f);
              rlVertex3f(x + 0.5f, y - 0.5f,
                         z + 0.5f); // Bottom Right Of The Texture and Quad

              color4ubGF(vc.bl * b2);
              rlTexCoord2f(1.0f - fx, 0.0f);
              rlVertex3f(x + 0.5f, y + 0.5f,
                         z + 0.5f); // Top Right Of The Texture and Quad

              color4ubGF(vc.tl * b2);
              rlTexCoord2f(0.0f + fx, 0.0f);
              rlVertex3f(x - 0.5f, y + 0.5f,
                         z + 0.5f); // Top Left Of The Texture and Quad
            }

            // Back Face
            if (faces.front) {
              c = 255;
              // color4ubG(c*b2, 255, 0);
              // rlNormal3f(0.0f, 0.0f, - 1.0f);                  // Normal
              // Pointing Away From Viewer

              c4v vc = getC4v(x, y, z, face_front);

              color4ubGF(vc.br * b2);
              rlTexCoord2f(1.0f - fx, 1.0f);
              rlVertex3f(x - 0.5f, y - 0.5f,
                         z - 0.5f); // Bottom Right Of The Texture and Quad

              color4ubGF(vc.tl * b2);
              rlTexCoord2f(1.0f - fx, 0.0f);
              rlVertex3f(x - 0.5f, y + 0.5f,
                         z - 0.5f); // Top Right Of The Texture and Quad

              color4ubGF(vc.bl * b2);
              rlTexCoord2f(0.0f + fx, 0.0f);
              rlVertex3f(x + 0.5f, y + 0.5f,
                         z - 0.5f); // Top Left Of The Texture and Quad

              color4ubGF(vc.tr * b2);
              rlTexCoord2f(0.0f + fx, 1.0f);
              rlVertex3f(x + 0.5f, y - 0.5f,
                         z - 0.5f); // Bottom Left Of The Texture and Quad
            }

            // Bottom Face
            if (faces.bottom) {
              t = textures[blockData[type].sides[2]].id;
              if (lastT != t) {
                rlSetTexture(t);
                lastT = t;
              }

              c = 255;
              // color4ubG(c*b3, 255, rgb);
              rlNormal3f(0.0f, -1.0f, 0.0f); // Normal Pointing Down

              c4v vc = getC4v(x, y, z, face_bottom);
              color4ubGF(vc.tl * b3);
              rlTexCoord2f(1.0f - fx, 0.0f);
              rlVertex3f(x - 0.5f, y - 0.5f,
                         z - 0.5f); // Top Right Of The Texture and Quad

              color4ubGF(vc.bl * b3);
              rlTexCoord2f(0.0f + fx, 0.0f);
              rlVertex3f(x + 0.5f, y - 0.5f,
                         z - 0.5f); // Top Left Of The Texture and Quad

              color4ubGF(vc.tr * b3);
              rlTexCoord2f(0.0f + fx, 1.0f);
              rlVertex3f(x + 0.5f, y - 0.5f,
                         z + 0.5f); // Bottom Left Of The Texture and Quad

              color4ubGF(vc.br * b3);
              rlTexCoord2f(1.0f - fx, 1.0f);
              rlVertex3f(x - 0.5f, y - 0.5f,
                         z + 0.5f); // Bottom Right Of The Texture and Quad
            }

            // if(dt < 0.1f) blockCount++;

            if (0 && type == bt_stone && isEmpty(x, y * chunkSize + 1, z)) {
              DrawBillboard(
                  camera, textures[blockData[bt_wool_red].sides[1]],
                  {(float)x, (float)(chunk * chunkSize + y + 1), (float)z}, 1,
                  WHITE);
            }
          }
        }
      }
    }
  }

  rlEnd();
  rlPopMatrix();

  if (onMapV(lookAt)) {
    Color c = BLACK; // ColorFromHSV((float)GetTime()*20.0f, 1.0f, 1.0f);
    // DrawCube(lookAt, 1,1,1, c);
    float las = 1.05f; // 1.05f;
    DrawCubeWiresV({lookAt.x, lookAt.y, lookAt.z}, {las, las, las}, c);
  }

  SetRandomSeed(oldSeed);
}

void draw2D() {
  std::vector<std::string> texts{
      TextFormat("Version: %s", VERSION),
      TextFormat("xyz: %.1f %.1f %.1f", camera.position.x, camera.position.y,
                 camera.position.z),
      TextFormat("looking at: %s",
                 (lookAt.x == -1) ? "----"
                                  : TextFormat("%i, %i, %i (%i)", (int)lookAt.x,
                                               (int)lookAt.y, (int)lookAt.z,
                                               getAtV(lookAt))),
      TextFormat("blocks: %i", blockCount),
      blockData[cType].name,
      TextFormat("Y velocity: %.2f m/s (%.2f km/h)", velY * 20.0f,
                 velY * 20.0f * 3.6f),
      TextFormat("Rot (DEG): %.2f, %.2f", rotX, rotY),
      TextFormat("Fast place/break: %s", onOff[p].c_str()),
      TextFormat("Smooth lighting: %s", onOff[smoothLighting].c_str()),
      TextFormat("flying: %s", onOff[flying].c_str())};

  for (int i = 0; i < texts.size(); i++) {
    // printf("%i: %s\n", i, texts[i].c_str());
    DrawTextEx(font, texts[i].c_str(),
               {10, (float)(10 + i * (fontSize * 1.2f))}, fontSize, 2, WHITE);
  }

  for (int i = 0; i < textList.size(); i++) {
    int i2 = textList.size() - 1 - i;
    unsigned char a =
        (i == 0) ? (unsigned char)(int)(min(max(textCd, 0.0f), 1.0f) * 255.0f)
                 : 255;
    DrawTextC(textList[i].c_str(), {(float)CX, 60 + (float)i2 * 30}, fontSize,
              (Color){255, 255, 255, a});
  }

  // draw selected block
  Texture2D *tex = &textures[blockData[cType].sides[1]];

  DrawRectangleRec({15, (float)H - (cTypeShowSize + 25),
                    (float)cTypeShowSize + 10, (float)cTypeShowSize + 10},
                   BLACK);
  DrawTexturePro(*tex, {0.0f, 0.0f, (float)tex->width, (float)tex->height},
                 {20, (float)H - cTypeShowSize - 20, (float)cTypeShowSize,
                  (float)cTypeShowSize},
                 {0, 0}, 0, WHITE);

  // draw crosshair
  drawCrosshair(WHITE);

  drawKeystrokes({W - W * 0.03f * 3.3f, 0}, W * 0.03f);
}

void draw() {
  BeginDrawing();
  ClearBackground(bgc);

  BeginMode3D(camera);

  draw3D();

  EndMode3D();

  if (!paused)
    draw2D();

  else {
    DrawTextC("Paused!", {(float)CX, (float)CY}, fontSize, WHITE);
  }

  EndDrawing();
}

void init() {
  logF.open("log.txt");

  if (!logF) {
    printf("failed to open log file.\n");
    exit(1);
  }

  cLog(0, "==== INIT ====");

  InitWindow(W, H, "Game");
  SetExitKey(KEY_F4);
  SetWindowState(FLAG_VSYNC_HINT);

  HideCursor();

  font = LoadFont(fontFName);
  initTextures();
  cLog(1, "default texture pack loaded");
  // printf("loadTextures: done :D\n");

  // InitAudioDevice();
  // initSounds();

  // skybox = createSkybox();
  // cLog(1, "skybox created");

  camera.position = (Vector3){chunkSize / 2.0f, 20.0f, chunkSize / 2.0f};
  camera.target = (Vector3){0.0f, 0.0f, 0.0f};
  camera.up = (Vector3){0.0f, 1.0f, 0.0f};
  camera.fovy = defFovY;
  camera.projection = CAMERA_PERSPECTIVE;
  // ToggleFullscreen();
  genWorld();

  // clearWorld();
  // loadWorld(guiInputTxt("Load map from"));
  printf("map generating: done\n");
  cLog(1, "world generated");

  // loadWorld("flat", &pCube);
}

void deInit() {

  cLog(0, "==== DE-INIT ====");
  UnloadFont(font);
  cLog(1, "font unloaded");

  for (int i = 0; i < texC; i++)
    UnloadTexture(textures[i]);

  UnloadModel(skybox);
  cLog(1, "sky unloaded");
  // deInitSounds();
  // cLog(1, "sounds unloaded");

  CloseWindow();
  cLog(1, "closed without any errors");
}

int main() {
  init();
  while (running) {

    dt = GetFrameTime();
    draw();

    if (!paused && dt < 0.1f)
      update();

    running = !WindowShouldClose() && running;
  }

  deInit();
}

std::string guiInputTxt(std::string displayText) {

  bool done;
  int key;
  std::string txt;

  bool isTyping = true;

  double start, end;

  while (isTyping) {

    if (WindowShouldClose())
      deInit();

    BeginDrawing();

    ClearBackground(bgc);

    if (skyEnabled) {
      BeginMode3D(camera);
      drawSky();
      EndMode3D();
    }

    DrawTextC(displayText.c_str(), {(float)CX, (float)CY - 40}, fontSize * 4,
              WHITE);
    DrawTextC(txt.c_str(), {(float)CX, (float)CY + 40}, fontSize * 4, WHITE);

    // DrawFPS(0,0);

    EndDrawing();

    key = GetCharPressed();

    // input text
    while (key > 0) {
      // 25, 125
      if ((key >= 25) && (key <= 125)) {
        txt.push_back((char)key);

        key = GetCharPressed();
      } else
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE) && txt.length() > 0) {
      txt.resize(txt.size() - sizeof(char));
    }

    if (IsKeyPressed(KEY_F11))
      ToggleFullscreen();

    if (IsKeyReleased(KEY_ENTER))
      isTyping = false;

    if (IsKeyPressed(KEY_ESCAPE)) {
      return "";
    }
  }

  return txt;
}

void pause() {
  bool isPaused = true;
  while (isPaused) {

    if (WindowShouldClose()) {
      isPaused = false;
      running = false;
    }

    BeginDrawing();
    ClearBackground(bgc);
    DrawRectangle(100, 100, 100, 100, RED);

    EndDrawing();

    if (IsKeyPressed(KEY_ESCAPE))
      return;
  }
}

void blockSelectionMenu() {
  if (IsCursorHidden())
    ShowCursor();

  bool run = true;

  int cols = 9, rows = (int)ceilf((float)blockTypeCount / (float)cols);
  int sb = W / 100;

  float scroll = 0;

  while (run) {
    scroll += GetMouseWheelMove() * 10.0f;
    scroll = min(max(scroll, 0), H - 100);
    BeginDrawing();
    ClearBackground(bgc);

    if (skyEnabled) {
      BeginMode3D(camera);
      drawSky();
      EndMode3D();
    }

    DrawCircle(CX, CY + 20, 10, YELLOW);

    for (int row = 0; row < rows; row++) {
      for (int col = 0; col < cols; col++) {
        int type = row * cols + col;
        if (type > blockTypeCount - 1)
          break;

        Texture2D *tex = &textures[blockData[type].sides[1]];

        Vector2 center = {
            (float)CX +
                (col - cols / 2) *
                    (cTypeShowSize + 10 +
                     sb), //(float)col/(float)cols*((float)cTypeShowSize+10.0f+(float)sb),
            (float)(100 + row * (cTypeShowSize + 10 + sb) + scroll)};

        DrawCircle(center.x, center.y, 10, RED);

        Rectangle rec = {center.x - (cTypeShowSize + 10) / 2,
                         center.y - (cTypeShowSize + 10) / 2,
                         (float)cTypeShowSize + 10, (float)cTypeShowSize + 10};

        DrawRectangleRec(
            rec,
            //{(float)CX-(cTypeShowSize/2+5),(float)H-(cTypeShowSize+25),(float)cTypeShowSize+10,(float)cTypeShowSize+10},
            cType == type ? WHITE : BLACK);

        DrawTexturePro(
            *tex, {0.0f, 0.0f, (float)tex->width, (float)tex->height},
            {center.x - (cTypeShowSize) / 2, center.y - (cTypeShowSize) / 2,
             (float)cTypeShowSize, (float)cTypeShowSize},
            {0.0f, 0.0f}, 0.0f, WHITE);

        if (CheckCollisionPointRec(GetMousePosition(), rec)) {
          DrawTextC(blockData[type].name,
                    {center.x, center.y - cTypeShowSize * 0.8f}, fontSize,
                    WHITE);
          if (CheckCollisionPointRec(GetMousePosition(), rec) &&
              IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            cType = type;
          }
        }
      }
    }

    EndDrawing();

    if (IsKeyPressed(KEY_E))
      run = false;

    if (WindowShouldClose()) {
      run = false;
      running = false;
    }
  }

  if (!IsCursorHidden())
    HideCursor();
}
