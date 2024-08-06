#include <iostream>
#include <fstream>

int main() {
    std::ofstream f;
    f.open("f.save");

    if(f.fail()) {
        std::cout << "valami hiba történt.\n";
        return 1;
    }

    int ds = 512;
    bool data[ds];

    for(int i; i<ds; i++) {
        data[i] = i%3 == 0;
    }

    f.write(reinterpret_cast<const char *>(data), sizeof(bool)*ds);

    f.close();

    return 0;
}