#include "global.h"

int main(int argc, char const *argv[])
{
    // // Json::Value root;
    // Json::FastWriter writer;
    // Json::Value person;

    // person["name"] = "hello world";
    // person["age"] = 100;
    // // root.append(person);

    // std::string json_file = writer.write(person);

    // cout << json_file << endl;

    // Json::Reader *pJsonParser = new Json::Reader(Json::Features::strictMode());
    // Json::Value tempVal;
    // if(!pJsonParser->parse(json_file, tempVal))
    // {
    //     cout << "parse error" << endl;
    //     return -1;
    // }
    // string name = tempVal["name"].asString();
    // cout << name << endl;


    Redis *rds = new Redis("127.0.0.1", 6379, 8);
    redisContext *context = rds->pRedisContext;
    redisReply *reply = (redisReply*)redisCommand(context,"SUBSCRIBE foo");
    freeReplyObject(reply);
    while(true) {
        int code = redisGetReply(context, (void**)&reply);
        cout << code << endl;
        cout << reply->type << endl;
        cout << reply->elements << endl;
        int i = 0;
        for (i = 0; i < reply->elements; i++) {
            cout << reply->element[i]->str << endl;
        }
        if (REDIS_OK != code) break;
        // consume message

        freeReplyObject(reply);
    }
    context = NULL;
    return 0;
}
