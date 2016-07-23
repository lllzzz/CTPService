#include "Config.h"

C::C(){}
C::~C(){}

string C::get(string key)
{
    string path = "../etc/config.ini";
    parseIniFile(path);
    string val = getOptionToString(key);
    return val;
}