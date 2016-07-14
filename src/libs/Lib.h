#ifndef LIB_H
#define LIB_H

#include "../../include/ThostFtdcUserApiStruct.h"
#include <string>
// #include <time.h>
#include <sys/time.h>
#include <cstring>
#include <fstream>
#include <iostream>
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

    static void sysErrLog(string logPath, string logName, CThostFtdcRspInfoField *info, int id, int isLast);
    static void sysReqLog(string logPath, string logName, int code);

    static void initInfoLogHandle(string logPath, ofstream & infoHandle, string iID = "");
    static void initMarketLogHandle(string logPath, ofstream & handle);

    static vector<string> split(const string& s, const string& delim);

    static double max(double arr[], int cnt);
    static double min(double arr[], int cnt);
    static double mean(double arr[], int cnt);

    Lib();
    ~Lib();

};

#endif
