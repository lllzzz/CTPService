#include "MarketSpi.h"

using namespace std;

MarketSpi::MarketSpi(CThostFtdcMdApi * mdApi)
{
    _mdApi = mdApi;
    _logger = new Logger("market");

    _userID = C::get("market_user_id_online");
    _password = C::get("market_password_online");
    _brokerID = C::get("market_broker_id_online");
    _iIDs = Lib::split(C::get("iids"), "/");
    _channel = C::get("channel_tick");

    string env = C::get("env");
    int db = Lib::stoi(C::get("rds_db_" + env));
    string host = C::get("rds_host_" + env);
    _rds = new Redis(host, 6379, db);
    _rdsLocal = new Redis("127.0.0.1", 6379, 1);
    _reqID = 1;
}

MarketSpi::~MarketSpi()
{
    _mdApi = NULL;
    delete _rds;
    delete _rdsLocal;
    delete _logger;
    _logger->info("MarketSpi[~]");
}

void MarketSpi::OnFrontConnected()
{

    CThostFtdcReqUserLoginField reqUserLogin;

    memset(&reqUserLogin, 0, sizeof(reqUserLogin));

    strcpy(reqUserLogin.BrokerID, _brokerID.c_str());
    strcpy(reqUserLogin.UserID, _userID.c_str());
    strcpy(reqUserLogin.Password, _password.c_str());

    // 发出登陆请求
    int res = _mdApi->ReqUserLogin(&reqUserLogin, _reqID++);
    _logger->request("MarketSrv[ReqUserLogin]", _reqID, res);
}


void MarketSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo && pRspInfo->ErrorID != 0) {
        _logger->error("MarketSrv[OnRspUserLogin]", pRspInfo, nRequestID, bIsLast);
    }

    _logger->push("SessionID", Lib::itos(pRspUserLogin->SessionID));
    _logger->push("TradingDay", string(pRspUserLogin->TradingDay));
    _logger->info("MarketSrv[LoginSuccess]");

    int cnt = _iIDs.size();
    char ** Instrumnet;
    Instrumnet = (char**)malloc((sizeof(char*)) * cnt);
    for (int i = 0; i < cnt; i++) {
        char * tmp = Lib::stoc(_iIDs[i]);
        Instrumnet[i] = tmp;
    }

    int res = _mdApi->SubscribeMarketData(Instrumnet, cnt);
    free(Instrumnet);
    _logger->request("MarketSrv[SubscribeMarketData]", _reqID, res);
}

void MarketSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo && pRspInfo->ErrorID != 0) {
        _logger->error("MarketSrv[OnRspSubMarketData]", pRspInfo, nRequestID, bIsLast);
    }
}

/**
 * 接收市场数据
 * 绘制K线图
 * 执行策略进行买卖
 * @param pDepthMarketData [description]
 */
void MarketSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
    _saveMarketData(pDepthMarketData);
}

void MarketSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    _logger->error("MarketSrv[OnRspError]", pRspInfo, nRequestID, bIsLast);
}

void MarketSpi::_saveMarketData(CThostFtdcDepthMarketDataField *data)
{
    if (!data) return;

    Json::FastWriter writer;
    Json::Value tick;

    string iid = string(data->InstrumentID);
    tick["iid"] = iid;
    tick["price"] = data->LastPrice;
    tick["vol"] = data->Volume;
    tick["time"] = string(data->ActionDay) + " " + string(data->UpdateTime);
    tick["msec"] = data->UpdateMillisec;
    tick["bid1"] = data->BidPrice1;
    tick["bidvol1"] = data->BidVolume1;
    tick["ask1"] = data->AskPrice1;
    tick["askvol1"] = data->AskVolume1;

    std::string jsonStr = writer.write(tick);
    _rds->pub(_channel + iid, jsonStr);
    _rdsLocal->push("Q_TICK", jsonStr);
}


