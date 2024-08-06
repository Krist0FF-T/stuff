#include <iostream>
#include <fstream>

#define igen  true
#define nem   false
#define str std::string
#define szam  int

int main() {
    std::ifstream f;
    f.open("f.save");

    if(f.fail()) {
        std::cout << "valami hiba történt.\n";
        return 1;
    }

    int ds = 512;
    bool data[ds];
    f.read(reinterpret_cast<char *>(data), sizeof(bool)*ds);

    for(int i=0; i<ds; i++) {
        std::cout << (int) data[i] << '\n';
    }

    return 0;
}