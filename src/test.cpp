#include "global.h"

int main(int argc, char const *argv[])
{
    // Json::Value root;
    Json::FastWriter writer;
    Json::Value person;

    person["name"] = "hello world";
    person["age"] = 100;
    // root.append(person);

    std::string json_file = writer.write(person);

    cout << json_file << endl;

    Json::Reader *pJsonParser = new Json::Reader(Json::Features::strictMode());
    Json::Value tempVal;
    if(!pJsonParser->parse(json_file, tempVal))
    {
        cout << "parse error" << endl;
        return -1;
    }
    string name = tempVal["name"].asString();
    cout << name << endl;
    return 0;
}
