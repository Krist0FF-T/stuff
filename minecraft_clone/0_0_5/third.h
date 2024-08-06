#ifndef THIRD_H
#define THIRD_H

#include <math.h>
#include <raylib.h>
#include <rlgl.h>

#include <fstream>

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

inline bool exists_test0 (const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
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


// Note: I didn't write this one.
static TextureCubemap GenTextureCubemap(Shader shader, Texture2D panorama, int size, int format) {
    TextureCubemap cubemap = { 0 };

    rlDisableBackfaceCulling();     // Disable backface culling to render inside the cube

    // STEP 1: Setup framebuffer
    //------------------------------------------------------------------------------------------
    unsigned int rbo = rlLoadTextureDepth(size, size, true);
    cubemap.id = rlLoadTextureCubemap(0, size, format);

    unsigned int fbo = rlLoadFramebuffer(size, size);
    rlFramebufferAttach(fbo, rbo, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);
    rlFramebufferAttach(fbo, cubemap.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_CUBEMAP_POSITIVE_X, 0);

    // Check if framebuffer is complete with attachments (valid)
    if (rlFramebufferComplete(fbo)) TraceLog(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", fbo);
    //------------------------------------------------------------------------------------------

    // STEP 2: Draw to framebuffer
    //------------------------------------------------------------------------------------------
    // NOTE: Shader is used to convert HDR equirectangular environment map to cubemap equivalent (6 faces)
    rlEnableShader(shader.id);

    // Define projection matrix and send it to shader
    Matrix matFboProjection = MatrixPerspective(90.0*DEG2RAD, 1.0, RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);
    rlSetUniformMatrix(shader.locs[SHADER_LOC_MATRIX_PROJECTION], matFboProjection);

    // Define view matrix for every side of the cubemap
    Matrix fboViews[6] = {
        MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){  1.0f,  0.0f,  0.0f }, (Vector3){ 0.0f, -1.0f,  0.0f }),
        MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){ -1.0f,  0.0f,  0.0f }, (Vector3){ 0.0f, -1.0f,  0.0f }),
        MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){  0.0f,  1.0f,  0.0f }, (Vector3){ 0.0f,  0.0f,  1.0f }),
        MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){  0.0f, -1.0f,  0.0f }, (Vector3){ 0.0f,  0.0f, -1.0f }),
        MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){  0.0f,  0.0f,  1.0f }, (Vector3){ 0.0f, -1.0f,  0.0f }),
        MatrixLookAt((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector3){  0.0f,  0.0f, -1.0f }, (Vector3){ 0.0f, -1.0f,  0.0f })
    };

    rlViewport(0, 0, size, size);   // Set viewport to current fbo dimensions
    
    // Activate and enable texture for drawing to cubemap faces
    rlActiveTextureSlot(0);
    rlEnableTexture(panorama.id);

    for (int i = 0; i < 6; i++)
    {
        // Set the view matrix for the current cube face
        rlSetUniformMatrix(shader.locs[SHADER_LOC_MATRIX_VIEW], fboViews[i]);
        
        // Select the current cubemap face attachment for the fbo
        // WARNING: This function by default enables->attach->disables fbo!!!
        rlFramebufferAttach(fbo, cubemap.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_CUBEMAP_POSITIVE_X + i, 0);
        rlEnableFramebuffer(fbo);

        // Load and draw a cube, it uses the current enabled texture
        rlClearScreenBuffers();
        rlLoadDrawCube();

        // ALTERNATIVE: Try to use internal batch system to draw the cube instead of rlLoadDrawCube
        // for some reason this method does not work, maybe due to cube triangles definition? normals pointing out?
        // TODO: Investigate this issue...
        //rlSetTexture(panorama.id); // WARNING: It must be called after enabling current framebuffer if using internal batch system!
        //rlClearScreenBuffers();
        //DrawCubeV(Vector3Zero(), Vector3One(), WHITE);
        //rlDrawRenderBatchActive();
    }
    //------------------------------------------------------------------------------------------

    // STEP 3: Unload framebuffer and reset state
    //------------------------------------------------------------------------------------------
    rlDisableShader();          // Unbind shader
    rlDisableTexture();         // Unbind texture
    rlDisableFramebuffer();     // Unbind framebuffer
    rlUnloadFramebuffer(fbo);   // Unload framebuffer (and automatically attached depth texture/renderbuffer)

    // Reset viewport dimensions to default
    rlViewport(0, 0, rlGetFramebufferWidth(), rlGetFramebufferHeight());
    rlEnableBackfaceCulling();
    //------------------------------------------------------------------------------------------

    cubemap.width = size;
    cubemap.height = size;
    cubemap.mipmaps = 1;
    cubemap.format = format;

    return cubemap;
}

Model createSkybox() {
    
    Mesh cube = GenMeshCube(1.0f, 1.0f, 1.0f);
    Model skybox = LoadModelFromMesh(cube);

    // Load skybox shader and set required locations
    // NOTE: Some locations are automatically set at shader loading
    skybox.materials[0].shader = LoadShader("shaders/skybox.vs", "shaders/skybox.fs");

    int mmc[1] = {MATERIAL_MAP_CUBEMAP}, int1_0[1]{0};
    SetShaderValue(skybox.materials[0].shader, GetShaderLocation(skybox.materials[0].shader, "environmentMap"), mmc, SHADER_UNIFORM_INT);
    SetShaderValue(skybox.materials[0].shader, GetShaderLocation(skybox.materials[0].shader, "doGamma"), int1_0, SHADER_UNIFORM_INT);
    SetShaderValue(skybox.materials[0].shader, GetShaderLocation(skybox.materials[0].shader, "vflipped"), int1_0, SHADER_UNIFORM_INT);

    // Load cubemap shader and setup required shader locations
    Shader shdrCubemap = LoadShader("shaders/cubemap.vs", "shaders/cubemap.fs");

    SetShaderValue(shdrCubemap, GetShaderLocation(shdrCubemap, "equirectangularMap"), int1_0, SHADER_UNIFORM_INT);

    Image panoramaImg = LoadImage("img/skybox.png");
    ImageFlipVertical(&panoramaImg);
    Texture2D panorama = LoadTextureFromImage(panoramaImg);
    UnloadImage(panoramaImg);

    // Generate cubemap from panorama texture
    skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = GenTextureCubemap(shdrCubemap, panorama, 1024, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    UnloadTexture(panorama);

    UnloadShader(shdrCubemap);

    return skybox;
}


#endif