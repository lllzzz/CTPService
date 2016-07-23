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
    signal(30, shutdown);
    ofstream pid;
    pidPath = C::get("pid_path");
    pid.open(pidPath.c_str(), ios::out);
    pid << getpid();
    pid.close();

    // 初始化交易接口
    string flowPath = C::get("flow_path_m");
    string mURL = C::get("market_front_online");
    mApi = CThostFtdcMdApi::CreateFtdcMdApi(flowPath.c_str());
    MarketSpi mSpi(mApi); // 初始化回调实例
    mApi->RegisterSpi(&mSpi);
    mApi->RegisterFront(Lib::stoc(mURL));
    mApi->Init();
    cout << "MarketSrv start success!" << endl;
    mApi->Join();

    return 0;
}
