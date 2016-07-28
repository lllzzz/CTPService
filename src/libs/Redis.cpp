#include "Redis.h"

Redis::Redis(string host, int port, int db)
{
    struct timeval timeout = {2, 0};    //2s的超时时间
    //redisContext是Redis操作对象
    pRedisContext = (redisContext*)redisConnectWithTimeout(host.c_str(), port, timeout);
    if ( (NULL == pRedisContext) || (pRedisContext->err) )
    {
        if (pRedisContext)
        {
            std::cout << "connect error:" << pRedisContext->errstr << std::endl;
        }
        else
        {
            std::cout << "connect error: can't allocate redis context." << std::endl;
        }
        exit(-1);
    }
    char _db[2];
    int l = sprintf(_db, "%d", db);
    string select  = "select " + string(_db);
    string res = execCmd(select);
}

Redis::~Redis()
{
    delete pRedisContext;
    delete pRedisReply;
}

void Redis::pub(string key, string data)
{
    string cmd = "public " + key + " " + data;
    execCmd(cmd);
}

void Redis::push(string key, string data)
{
    string cmd = "lpush " + key + " " + data;
    execCmd(cmd);
}

string Redis::pop(string key)
{
    string cmd = "rpop " + key;
    return execCmd(cmd);
}

void Redis::set(string key, string data)
{
    string cmd = "set " + key + " " + data;
    execCmd(cmd);
}

void Redis::setnx(string key, string data)
{
    string cmd = "setnx " + key + " " + data;
    execCmd(cmd);
}

string Redis::incr(string key)
{
    string cmd = "incr " + key;
    return execCmd(cmd, true);
}

string Redis::get(string key)
{
    string cmd = "get " + key;
    return execCmd(cmd);
}

void Redis::asService(ACTIONCALLBACK callback, string channel)
{
    _callback = callback;
    _channel = channel;
}

void Redis::run()
{
    pRedisReply = (redisReply*)redisCommand(pRedisContext, Lib::stoc("SUBSCRIBE " + _channel));
    freeReplyObject(pRedisReply);
    while(true) {
        int code = redisGetReply(pRedisContext, (void**)&pRedisReply);
        if (pRedisReply->elements >= 3) {
            string data = string(pRedisReply->element[2]->str);
            if(!_callback(data)) break;
        }
        freeReplyObject(pRedisReply);
        if (REDIS_OK != code) break;
    }
}


string Redis::execCmd(string cmd, bool returnInt)
{
    //redisReply是Redis命令回复对象 redis返回的信息保存在redisReply对象中
    pRedisReply = (redisReply*)redisCommand(pRedisContext, cmd.c_str());  //执行INFO命令
    string res = "";
    if (returnInt) {
        char s[10];
        sprintf(s, "%d", pRedisReply->integer);
        res = string(s);
    } else if (pRedisReply->len > 0)
        res = pRedisReply->str;
    //当多条Redis命令使用同一个redisReply对象时
    //每一次执行完Redis命令后需要清空redisReply 以免对下一次的Redis操作造成影响
    freeReplyObject(pRedisReply);
    return res;
}
