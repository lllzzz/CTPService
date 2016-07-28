#include "TradeSrv.h"

TradeSrv * service;

bool action(string);

int main(int argc, char const *argv[])
{
    service = new TradeSrv();
    service->init();

    // 服务化
    string env = C::get("env");
    string host = C::get("rds_host_" + env);
    int db = Lib::stoi(C::get("rds_db_" + env));
    Redis * srv = new Redis(host, 6379, db);
    string channel = C::get("channel_trade");
    srv->asService(action, channel);

    cout << "TradeSrv start success!" << endl;
    srv->run();
    cout << "TradeSrv stop success!" << endl;

    return 0;
}

bool action(string data)
{
    if (data == "stop") {
        if (service) delete service;
        return false;
    }

    Json::Reader reader;
    Json::Value root;
    reader.parse(data, root, false);

    string action = root["action"].asString();
    if (action == "trade") {
        double price = root["price"].asDouble();
        int total = root["total"].asInt();
        bool isBuy = root["isBuy"].asBool();
        bool isOpen = root["isOpen"].asBool();
        int orderID = root["orderID"].asInt();
        string iid = root["iid"].asString();
        string type = root["type"].asString();
        service->trade(price, total, isBuy, isOpen, orderID, iid, type);
    }

    if (action == "cancel") {
        int orderID = root["orderID"].asInt();
        service->cancel(orderID);
    }

    return true;
}

