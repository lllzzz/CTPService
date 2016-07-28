#ifndef TRADER_SPI_H
#define TRADER_SPI_H

#include "TradeSrv.h"
#include "../global.h"
#include <cstring>

using namespace std;

class TraderSpi : public CThostFtdcTraderSpi
{
private:

    TradeSrv * _service;
    Logger * _logger;

public:
    TraderSpi(TradeSrv *);
    ~TraderSpi();

    void OnFrontConnected();
    void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    ///错误应答
    void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    ///报单通知
    void OnRtnOrder(CThostFtdcOrderField *pOrder);
    ///成交通知
    void OnRtnTrade(CThostFtdcTradeField *pTrade);
    ///报单录入错误回报
    void OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo);
    // void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition,
    //     CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    void OnFrontDisconnected(int nReason);
    void OnHeartBeatWarning(int nTimeLapse);

    void OnRspQryInstrumentOrderCommRate(CThostFtdcInstrumentOrderCommRateField *pInstrumentOrderCommRate,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
};

#endif
