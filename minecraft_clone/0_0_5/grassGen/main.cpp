#include <../include/raylib.h>

typedef unsigned char uc;

int main() {
    Image inImg = LoadImage("in.png");
    Image outImg = GenImageColor(inImg.width, inImg.height, BLACK);

    int a = 255;

    Color c = 
        {135,205,73};
        //{72, 181, 24, 255};

    float gr;
    Color nC, oC;

    for(int x=0; x<inImg.width; x++) {
    for(int y=0; y<inImg.height; y++) {
        oC = GetImageColor(inImg, x, y);
        gr = oC.r/255.0f;
        a = oC.a;
        nC = (Color) {(uc)(int)(c.r*gr), (uc)(int)(c.g*gr), (uc)(int)(c.b*gr), a};
        ImageDrawPixel(&outImg, x, y, nC);
    }}

    ExportImage(outImg, "out.png");

    UnloadImage(inImg);
    UnloadImage(outImg);
}