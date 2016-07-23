#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include "../../include/iniReader/iniReader.h"

using namespace std;

class C
{
public:
    C();
    ~C();
    
    static string get(string);
};


#endif