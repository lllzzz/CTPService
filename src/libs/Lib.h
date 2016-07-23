#ifndef LIB_H
#define LIB_H

#include "../../include/ThostFtdcUserApiStruct.h"
#include <string>
#include <fstream>
#include <sys/time.h>
#include <cstring>
#include <vector>
#include <sstream>
#include <algorithm>

using namespace std;

class Lib
{
public:

    static string getDate(string format, bool needUsec = false);

    static char * stoc(string str); 
    static int stoi(string s);
    static double stod(string s);

    static string dtos(double dbl);
    static string itos(int num);

    static vector<string> split(const string& s, const string& delim);

    Lib();
    ~Lib();

};

#endif
