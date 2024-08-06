#include <iostream>
#include <fstream>

//#include <raylib.h>

static const int
    chunkSize = 32,
    maxChunks = 100;

static __int8 blocks[maxChunks][chunkSize][chunkSize][chunkSize];

static int16_t ppos[3];
static int mode;

int loadWorld(std::string fName) {
	std::ifstream inputData;
	inputData.open("worlds/" + fName + ".save");
	if (inputData) {
        int16_t pos[3];
        inputData.read(reinterpret_cast<char *>(pos), sizeof(int16_t) * 3);
        ppos[0] = (float) ((int)pos[0]*0.1f);
        ppos[1] = (float) ((int)pos[1]*0.1f);
        ppos[2] = (float) ((int)pos[2]*0.1f);

        for(int chunk=0; chunk<maxChunks; chunk++) {

            for(int x=0; x<chunkSize; x++) {
            for(int y=0; y<chunkSize; y++) {
                inputData.read(reinterpret_cast<char *>(blocks[chunk][x][y]), sizeof(__int8) * chunkSize);
            }}
        }
		
		inputData.close();
		return 0;
	}
	else
		return -1;
}

int saveWorld(std::string fName) {
	std::ofstream outputData;
	outputData.open("worlds/" + fName + ".save");
	if (outputData) {
        int16_t pos[3] {
            (int16_t) (ppos[0]*10.0f),
            (int16_t) (ppos[1]*10.0f),
            (int16_t) (ppos[2]*10.0f),
        };
        
        outputData.write(reinterpret_cast<const char *>(pos), sizeof(int16_t)*3);

        for(int i=0; i<maxChunks; i++) {

            for(int x=0; x<chunkSize; x++) {
            for(int y=0; y<chunkSize; y++) {
                outputData.write(reinterpret_cast<const char *>(blocks[i][x][y]), sizeof(__int8) * chunkSize);
            }}
        }

		outputData.close();
		return 0;
	}
	else
		return -1;
}

void convert() {
    for(int i=0; i<maxChunks; i++) {
        for(int x=0; x<chunkSize; x++) {
        for(int cy=0; cy<chunkSize; cy++) {
        for(int z=0; z<chunkSize; z++) {
            blocks[i][x][cy][z] = -1;
        }}}
    }
}

int main() {

    std::string inFn, outFn;

    std::cout << "input world name: ";
    std::cin >> inFn;

    std::cout << "output world name: ";
    std::cin >> outFn;

    loadWorld(inFn);
    convert();
    saveWorld(outFn);

    system("pause");

    return 0;
}