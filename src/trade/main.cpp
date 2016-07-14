#include "TradeSrv.h"

TradeSrv * service;

bool action(long int, const void *);

int main(int argc, char const *argv[])
{
    // 初始化参数
    parseIniFile("../etc/config.ini");
    string flowPath = getOptionToString("flow_path_t");
    string logPath  = getOptionToString("log_path");
    string bid      = getOptionToString("trade_broker_id");
    string userID   = getOptionToString("trade_user_id");
    string password = getOptionToString("trade_password");
    string tURL     = getOptionToString("trade_front");

    string instrumnetIDs = getOptionToString("instrumnet_id");

    int tradeSrvID         = getOptionToInt("trade_service_id");
    int tradeStrategySrvID = getOptionToInt("trade_strategy_service_id");

    int isDev = getOptionToInt("is_dev");
    int db;
    if (isDev) {
        db = getOptionToInt("rds_db_dev");
    } else {
        db = getOptionToInt("rds_db_online");
    }

    service = new TradeSrv(bid, userID, password, tURL, instrumnetIDs,
        flowPath, logPath, tradeStrategySrvID, db);
    service->init();

    // 服务化
    QService Qsrv(tradeSrvID, sizeof(MSG_TO_TRADE));
    Qsrv.setAction(action);
    cout << "TradeSrv start success!" << endl;
    Qsrv.run();
    cout << "TradeSrv stop success!" << endl;

    return 0;
}

bool action(long int msgType, const void * data)
{
    // cout << "MSG|" << msgType;
    if (msgType == MSG_SHUTDOWN) {
        if (service) delete service;
        // cout << endl;
        return false;
    }

    MSG_TO_TRADE msg = *((MSG_TO_TRADE*)data);
    // cout << "|PRICE|" << msg.price << "|ISBUY|" << msg.isBuy << "|ISOPEN|" << msg.isOpen << "|TOTAL|" << msg.total << "|KINDEX|" << msg.orderID << endl;
    if (msgType == MSG_ORDER) {
        service->trade(msg.price, msg.total, msg.isBuy, msg.isOpen, msg.orderID, string(msg.instrumnetID), msg.type);
    }
    if (msgType == MSG_ORDER_CANCEL) {
        service->cancel(msg.orderID);
    }
    return true;
}

