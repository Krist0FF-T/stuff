#ifndef THIRD_H
#define THIRD_H

#include <math.h>
#include <fstream>

#ifndef PI
    #define PI 3.141592653589793238462643383279502
#endif

#ifndef min
    #define min(a, b) (a < b ? a : b)
#endif

#ifndef max
    #define max(a, b) (a > b ? a : b)
#endif

#ifndef dCos
    #define dCos(x) (cosf(x*PI/180.0f))
#endif
#ifndef dSin
    #define dSin(x) (sinf(x*PI/180.0f))
#endif

#ifndef sign
    #define sign(x) (x == 0 ? 0 : (x < 0 ? -1 : 1))
#endif

#define isIn(x, k1, k2) (x > k1 && x < k2)

float toRad(float deg) {return deg*PI/180;}

float fFloor(float x) {
    return (float) ((int)x);
}

float fCeil(float x) {
    return (float) ((int)x+1);
}

bool chance(int m) {
    return rand()%m == 0;
}

bool chanceP(float percent) {
    return (rand() % ((int) round(1.0f/(percent/100.0f)))) == 0;
}

int choiceInt(int count, int *arr) {
    return arr[rand() % count];
}

template<typename type>
type vecRandChoice(std::vector<type> vec) {
    return vec[rand() % vec.size()];
}

template<typename type>
type abs(type x) {
    if(x > ((type) 0)) return x;
    return -x;
}

inline bool fileExists(const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}

std::vector<std::string> splitStr(std::string str, char sc) {
    std::vector<std::string> strVec = {};
    std::string nStr = "";

    for(char c : str) {
        if(c == sc) {
            if(nStr.size() == 0)
                continue;
            
            strVec.push_back(nStr);
            nStr = "";
        } else {
            nStr.push_back(c);
        }
    }
    strVec.push_back(nStr);
    return strVec;
}//splitStr

#endif