/*

void genMap1() {
    unsigned int oldSeed = GetRandomValue(1,1000);
    //SetRandomSeed(10);

    Image img1 = GenImageCellular(chunkSize, chunkSize, chunkSize);
    Image img2 = GenImageCellular(chunkSize, chunkSize, (int)(chunkSize*(3.0f/4.0f)));
    Image img3 = GenImageCellular(chunkSize, chunkSize, chunkSize/2);

    //Image gradient = GenImageGradientRadial(chunkSize, chunkSize, 0.5f, WHITE, BLACK);

    float mult = 50;
    int add = maxChunks *chunkSize -mult -200;

    int heightMap[chunkSize][chunkSize];

    for(int x=0; x<chunkSize; x++) {
        for(int y=0; y<chunkSize; y++) {
            unsigned char g = (GetImageColor(img1, x,y).r + GetImageColor(img2, x,y).r + GetImageColor(img3, x,y).r)/3;//*
                        //(GetImageColor(gradient, x,y).r/255.0f);
            int height = 1;
            if(!flat) height = min((int) round(g / 255.0f * mult + add), chunkSize*maxChunks-10);;
            heightMap[x][y] = height;
        }
    }

    UnloadImage(img1);
    UnloadImage(img2);
    UnloadImage(img3);

    int type;
    int height = 1;

    for(int x=0; x<chunkSize; x++) {
    for(int y=0; y<chunkSize*maxChunks; y++) {
    for(int z=0; z<chunkSize; z++) {

        if(!flat) height = heightMap[x][z];

        type = B_AIR;
        
        if(y == height)
            type = B_GREEN;

        else if(y < height)
            type = chance(10) ? B_STONE : B_DIRT;

        setAt(x,y,z, type);

    }}}

    // spawn player

    int cx = GetRandomValue(0, chunkSize-1),
        cz = GetRandomValue(0, chunkSize-1),
        cy = heightMap[cx][cz]+2;
    
    camera.position = (Vector3) {(float)cx, (float)cy, (float)cz};

    if(trees) {

        int treeMap[chunkSize][chunkSize];

        for(int x=0; x<chunkSize; x++) {
        for(int y=0; y<chunkSize; y++) {
            treeMap[x][y] = (abs(x-cx)<2 && abs(y-cz)<2) ? 1 : 0;
        }}

        int sb = 5;

        bool tree = false;
        int emptyC = 0;

        for(int x=sb/2; x<chunkSize-sb/2; x++) {
        for(int z=sb/2; z<chunkSize-sb/2; z++) {
            tree = false;
            if(chance(20)) {
                emptyC = 0;

                for(int j=-sb; j<sb+1; j++) {
                for(int i=-sb; i<sb+1; i++) {
                    int ax = x+j;
                    int az = z+i;
                    if(ax < 0 || ax > chunkSize-1 || az < 0 || az > chunkSize-1 || treeMap[ax][az] == 0)
                        emptyC++;

                    else
                        break;
                }}

                if(emptyC == pow(sb*2+1, 2)) {
                    tree = true;
                    if(!flat) height = heightMap[x][z];
                    genTree(x,height+1,z);
                }

            }

            treeMap[x][z] = tree;
            //printf("%i", tree);
        }}
    
    }

    SetRandomSeed(oldSeed);
}

void genMap2() {

    int biomeMap[maxChunks];

    int biome = 0;

    for(int i=0; i<maxChunks; i++) {
        if(chance(2)) biome = GetRandomValue(0,4) == 0 ? BIOME_DESERT : BIOME_FOREST;
        biomeMap[i] = biome;
    }

    int imgC = 3;
    Image imgs[imgC*2];
    int tileSize = chunkSize;

    for(int i=0; i<2; i++) {
        for(int j=0; j<imgC; j++) {
            imgs[i*imgC+j] = GenImageCellular(chunkSize, chunkSize*maxChunks, tileSize*(((imgC+1)-j)/(float)(imgC+1)));
        }
    }

    bool xMap[chunkSize][chunkSize*maxChunks];
    bool zMap[chunkSize][chunkSize*maxChunks];

    int g, sum, k = 120;

    for(int x=0; x<chunkSize; x++) {
        for(int y=0; y<chunkSize*maxChunks; y++) {
            if(!flat) {
                // X =================
                sum = 0;
                for(int i=0; i<imgC; i++) {
                    sum += GetImageColor(imgs[i], x,y).r;
                }
                g = sum/imgC;
                xMap[x][y] = ((int) g < k);

                // Z ==================
                sum = 0;
                for(int i=0; i<imgC; i++) {
                    sum += GetImageColor(imgs[i+imgC], x,y).r;
                }
                g = sum/imgC;
                zMap[x][y] = ((int) g < k);
            }

            else {
                xMap[x][y] = y == 0;
                zMap[x][y] = y == 0;
            }

        }
    }

    for(int i=0; i<6; i++)
        UnloadImage(imgs[i]);

    int type;

    //int types[3] = {B_GREEN, B_STONE, B_DIRT};

    for(int x=0; x<chunkSize; x++) {
    for(int y=0; y<chunkSize*maxChunks; y++) {
        int biome = biomeMap[chunkOf(y)];
    for(int z=0; z<chunkSize; z++) {
        type = B_AIR;

        if((xMap[x][y] && zMap[z][y])) {
            // 10% chance to stone, 90 to dirt
            type = (chanceP(10.0f)) ? ((biome == BIOME_FOREST) ? B_DIRT : B_SAND) : B_STONE;
            // if there is no block above
            if(!((y == 0 || y == maxChunks*chunkSize-1) || (xMap[x][y+1]) && zMap[z][y+1])) {
                type = (biome == BIOME_FOREST) ? B_GREEN : B_SAND;
            }

        }

        setAt(x,y,z, type);
    }}}

    // trees

    if(!trees) return;

    std::vector<Vector3> treeVec;
    int treeSb = 6;

    for(int x=0; x<chunkSize; x++) {
    for(int y=0; y<chunkSize*maxChunks; y++) {
        int biome = biomeMap[chunkOf(y)];
    for(int z=0; z<chunkSize; z++) {
        if(!(x > treeSb/2 && x < chunkSize-treeSb/2 && z > treeSb/2 && z < chunkSize-treeSb/2)) continue;

        // is grass, empty above and 1% chance
        if(isFlippable(getAt(x,y,z)) && getAt(x,y+1,z) == B_AIR && chanceP(1)) {

            bool tree = true;
            for(int i=0; i<treeVec.size()&&tree; i++) {
                if(abs(treeVec[i].y-(y+1)) < treeSb &&
                   abs(treeVec[i].x-x) < treeSb &&
                   abs(treeVec[i].z-z) < treeSb)
                        tree = false;
            }

            if(tree) {
                treeVec.push_back((Vector3){(float)x, (float)y+1, (float)z});
                if(biome == BIOME_FOREST) {
                    genTree(x,y+1,z);
                } else {

                    int maxHeight = GetRandomValue(2, 6);
                    for(int i=0; i<maxHeight; i++) {
                        if(!isEmpty(x,y+2+i,z)) break;
                        setAt(x,y+1+i,z, B_LEAF);

                    }
                }
            }
        }
    }}}
}

*/