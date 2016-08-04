#ifndef REDIS_H
#define REDIS_H

#include <hiredis/hiredis.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include "Config.h"
#include "Lib.h"

using namespace std;

typedef bool (*ACTIONCALLBACK)(string);

class Redis
{
public:
    redisContext *pRedisContext;
    redisReply *pRedisReply;

    ACTIONCALLBACK _callback;
    string _channel;
public:
    Redis(string host, int port, int db);
    ~Redis();
    string execCmd(string cmd, bool = false);
    string pop(string key);
    void set(string key, string data);
    void setnx(string key, string data);
    string incr(string key);
    string get(string key);

    void pub(string key, string data);

    void asService(ACTIONCALLBACK, string);
    void run();
};

#endif
