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

#include <iconv.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

using namespace std;

class Lib
{
public:

    static string getDate(string format, bool needUsec = false);

    static char * stoc(string str);
    static string ctos(char c);
    static int stoi(string s);
    static double stod(string s);

    static string dtos(double dbl);
    static string itos(int num);

    static vector<string> split(const string& s, const string& delim);

    static int code_convert(string from_charset, string to_charset, char *inbuf, size_t inlen, char *outbuf, size_t outlen);
    static string g2u(string);


    Lib();
    ~Lib();

};

#endif
