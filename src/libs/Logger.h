#ifndef LOGGER_H
#define LOGGER_H

#include "../../include/ThostFtdcUserApiStruct.h"
#include "Config.h"
#include "Lib.h"
#include <string>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>

using namespace std;

#define TYPE_INFO 0
#define TYPE_ERR  1
#define TYPE_REQ  2

class Logger
{
private:
    std::map<string, string> _info;
    string _path;
    string _id;
    void _write(int, string);
    void _clear();

public:
    Logger(string);
    ~Logger();
    
    void push(string, string);

    void info(string);
    void error(string, CThostFtdcRspInfoField *, int, int);
    void request(string, int, int);
};

#endif
