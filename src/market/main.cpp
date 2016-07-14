#include "MarketSpi.h"
#include <signal.h>

using namespace std;

CThostFtdcMdApi * mApi;
string pidPath;

void shutdown(int sig)
{
    mApi->Release();
    remove(pidPath.c_str());
    cout << "MarketSrv stop success!" << endl;
}

int main(int argc, char const *argv[])
{
    // 初始化参数
    parseIniFile("../etc/config.ini");
    string env = getOptionToString("env");

    string flowPath = getOptionToString("flow_path_m");
    string logPath  = getOptionToString("log_path");

    string bid      = getOptionToString("market_broker_id_" + env);
    string userID   = getOptionToString("market_user_id_" + env);
    string password = getOptionToString("market_password_" + env);
    string mURL     = getOptionToString("market_front_" + env);

    string instrumnetIDs = getOptionToString("iids");
    string channel = getOptionToString("channel_tick");
    pidPath  = getOptionToString("pid_path");


    int db;
    if (env == "online") {
        db = getOptionToInt("rds_db_online");
    } else {
        db = getOptionToInt("rds_db_dev");
    }

    signal(30, shutdown);
    ofstream pid;
    pid.open(pidPath.c_str(), ios::out);
    pid << getpid();
    pid.close();

    // 初始化交易接口
    mApi = CThostFtdcMdApi::CreateFtdcMdApi(flowPath.c_str());
    MarketSpi mSpi(mApi, logPath, bid, userID, password, instrumnetIDs, db, channel); // 初始化回调实例
    mApi->RegisterSpi(&mSpi);
    mApi->RegisterFront(Lib::stoc(mURL));
    mApi->Init();
    cout << "MarketSrv start success!" << endl;
    mApi->Join();

    return 0;
}
