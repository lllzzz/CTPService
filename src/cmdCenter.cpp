#include "global.h"
#include <signal.h>

int getPid(string pidPath)
{
    ifstream pf(pidPath.c_str());
    char pc[20];
    pf.getline(pc, 20);
    pf.close();
    return atoi(pc);
}

void stopMarket()
{
    // 关闭市场数据源
    int pid = getPid(getOptionToString("pid_path"));
    kill(pid, 30);
}

void stopTrade()
{
    // stop tradeSrv
    // QClient tradeClt(tradeSrvID, sizeof(MSG_TO_TRADE));
    // MSG_TO_TRADE msgt = {0};
    // msgt.msgType = MSG_SHUTDOWN;
    // tradeClt.send((void *)&msgt);
}

int main(int argc, char const *argv[])
{

    if (argc == 1) {
        cout << "请输入命令代码：" << endl;
        cout << "1:启动服务|2:停止" << endl;
        exit(0);
    }

    parseIniFile("../etc/config.ini");

    int cmd = atoi(argv[1]);
    if (cmd == 1) {
        // 启动各服务模块
        // system("./tradeSrv &");

        // 启动数据源
        system("./marketSrv &");

    } else if (cmd == 2) {

        stopMarket();
        stopTrade();

    } 

    return 0;
}
