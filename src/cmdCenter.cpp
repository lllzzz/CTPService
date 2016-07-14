#include "global.h"
#include <signal.h>

int kLineSrvID;
int tradeLogicSrvID;
int tradeStrategySrvID;
int tradeSrvID;

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

void stopKLine()
{
    // stop kLineSrv
    QClient kLineClt(kLineSrvID, sizeof(MSG_TO_KLINE));
    MSG_TO_KLINE msgkl = {0};
    msgkl.msgType = MSG_SHUTDOWN;
    kLineClt.send((void *)&msgkl);
}

void stopTradeLogic()
{
    // stop tradeLogicSrv
    QClient tradeLogicClt(tradeLogicSrvID, sizeof(MSG_TO_TRADE_LOGIC));
    MSG_TO_TRADE_LOGIC msgtl = {0};
    msgtl.msgType = MSG_SHUTDOWN;
    tradeLogicClt.send((void *)&msgtl);
}

void stopTradeStrategy()
{
    // stop tradeStrategySrv
    QClient tradeStrategyClt(tradeStrategySrvID, sizeof(MSG_TO_TRADE_STRATEGY));
    MSG_TO_TRADE_STRATEGY msgts = {0};
    msgts.msgType = MSG_SHUTDOWN;
    tradeStrategyClt.send((void *)&msgts);
}

void stopTrade()
{
    // stop tradeSrv
    QClient tradeClt(tradeSrvID, sizeof(MSG_TO_TRADE));
    MSG_TO_TRADE msgt = {0};
    msgt.msgType = MSG_SHUTDOWN;
    tradeClt.send((void *)&msgt);
}

int main(int argc, char const *argv[])
{

    if (argc == 1) {
        cout << "请输入命令代码：" << endl;
        cout << "1:启动服务|2:停止" << endl;
        cout << "3:启动回测(K线输入)|4:停止回测" << endl;
        cout << "5:启动回测(Tick输入)|6:停止回测" << endl;
        exit(0);
    }

    parseIniFile("../etc/config.ini");
    kLineSrvID         = getOptionToInt("k_line_service_id");
    tradeLogicSrvID    = getOptionToInt("trade_logic_service_id");
    tradeStrategySrvID = getOptionToInt("trade_strategy_service_id");
    tradeSrvID         = getOptionToInt("trade_service_id");
    string rootPath    = getOptionToString("root_path");

    string phpCmd = "php " + rootPath + "bin/tools/initSys.php";
    int cmd = atoi(argv[1]);
    if (cmd == 1) {

        // php 相关模块启动，负责初始化redis，队列消费者请手动启动
        system(phpCmd.c_str());
        sleep(1);

        // 启动各服务模块
        system("./tradeSrv &");
        system("./tradeStrategySrv &");
        system("./tradeLogicSrv &");
        system("./kLineSrv &");
        sleep(1);

        // 启动数据源
        system("./marketSrv &");

    } else if (cmd == 2) {

        stopMarket();
        stopKLine();
        stopTradeLogic();
        stopTradeStrategy();
        stopTrade();

    } else if (cmd == 3) {

        // 需要手动启动tradeStrategyAfter 1 以及消费者
        system("./tradeStrategySrv &");
        system("./tradeLogicSrv &");

    } else if (cmd == 4) {

        stopTradeLogic();
        stopTradeStrategy();

    } else if (cmd == 5) {

        // system(.c_str());
        // sleep(1);
        // 需要手动启动tradeStrategyAfter 1 以及消费者
        system("./tradeStrategySrv &");
        system("./tradeLogicSrv &");
        system("./kLineSrv &");

    } else if (cmd == 6) {

        stopKLine();
        stopTradeLogic();
        stopTradeStrategy();
    }

    return 0;
}
