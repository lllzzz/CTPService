#ifndef MARK_SPI_H
#define MARK_SPI_H

#include "../global.h"
#include "../../include/ThostFtdcMdApi.h"
#include "../../include/ThostFtdcTraderApi.h"
#include "../libs/Redis.h"
#include <map>

using namespace std;

typedef struct trade_hm
{
    int hour;
    int min;

} TRADE_HM;

class MarketSpi : public CThostFtdcMdSpi
{
private:

    CThostFtdcMdApi * _mdApi;
    Redis * _rds;

    string _brokerID;
    string _userID;
    string _password;
    std::vector<string>  _instrumnetIDs;

    string _channel;

    string _logPath;

    void _saveMarketData(CThostFtdcDepthMarketDataField *);

public:

    MarketSpi(CThostFtdcMdApi *, string, string, string, string, string, int, string);
    ~MarketSpi();

    void OnFrontConnected();
    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

    void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
};


#endif
