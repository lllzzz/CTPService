#include "TraderSpi.h"
#include "../libs/Lib.h"

TraderSpi::TraderSpi(TradeSrv * service, string logPath)
{
    _service = service;
    _logPath = logPath;
    _sessionID = 0;
}

TraderSpi::~TraderSpi()
{
    _service = NULL;
    cout << "~TraderSpi" << endl;
}

void TraderSpi::OnFrontConnected()
{
    _service->login();
}


void TraderSpi::OnFrontDisconnected(int nReason)
{
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeSpi[OnFrontDisconnected]";
    info << "|" << nReason;
    info << endl;
    info.close();
}

void TraderSpi::OnHeartBeatWarning(int nTimeLapse)
{
    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeSpi[OnHeartBeatWarning]";
    info << "|" << nTimeLapse;
    info << endl;
    info.close();
}

void TraderSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Lib::sysErrLog(_logPath, "TradeSpi[OnRspSettlementInfoConfirm]", pRspInfo, nRequestID, bIsLast);

    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeSpi[OnRspSettlementInfoConfirm]";
    if (pSettlementInfoConfirm) {
        info << "|ConfirmDate|" << pSettlementInfoConfirm->ConfirmDate;
        info << "|ConfirmTime|" << pSettlementInfoConfirm->ConfirmTime;
    }
    info << endl;
    info.close();
}

void TraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Lib::sysErrLog(_logPath, "TradeSpi[onLogin]", pRspInfo, nRequestID, bIsLast);
    _service->onLogin(pRspUserLogin);
    _service->confirm();
    _sessionID = pRspUserLogin->SessionID;

    ofstream info;
    Lib::initInfoLogHandle(_logPath, info);
    info << "TradeSpi[onLogin]";
    if (pRspUserLogin) {
        info << "|SessionID|" << pRspUserLogin->SessionID;
        info << "|FrontID|" << pRspUserLogin->FrontID;
        info << "|TradingDay|" << pRspUserLogin->TradingDay;
        info << "|MaxOrderRef|" << pRspUserLogin->MaxOrderRef;
    }
    info << endl;
    info.close();
}

void TraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Lib::sysErrLog(_logPath, "TradeSpi[onOrderInsert]", pRspInfo, nRequestID, bIsLast);
    _service->onOrderErr(pInputOrder, pRspInfo);
}

void TraderSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
    if (!pOrder) return;
    if (pOrder->OrderStatus != THOST_FTDC_OST_Canceled) {
        _service->onOrderRtn(pOrder);
    } else {
        _service->onCancel(pOrder);
    }
}

void TraderSpi::OnRspQryInstrumentOrderCommRate(CThostFtdcInstrumentOrderCommRateField *pInstrumentOrderCommRate,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Lib::sysErrLog(_logPath, "TradeSpi[OnRspQryInstrumentOrderCommRate]", pRspInfo, nRequestID, bIsLast);
    if (!pInstrumentOrderCommRate) return;
    _service->onQryCommRate(pInstrumentOrderCommRate);
}

void TraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
    _service->onTraded(pTrade);
}

void TraderSpi::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction,
        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Lib::sysErrLog(_logPath, "TradeSpi[onOrderAction]", pRspInfo, nRequestID, bIsLast);
    _service->onCancelErr(pInputOrderAction, pRspInfo);
}

void TraderSpi::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder,
    CThostFtdcRspInfoField *pRspInfo)
{
    Lib::sysErrLog(_logPath, "TradeSpi[ErrRtnOrderInsert]", pRspInfo, 0, 1);
}

void TraderSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    Lib::sysErrLog(_logPath, "TradeSpi[RspError]", pRspInfo, nRequestID, bIsLast);
}

