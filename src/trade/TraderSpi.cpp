#include "TraderSpi.h"
#include "../libs/Lib.h"

TraderSpi::TraderSpi(TradeSrv * service)
{
    _service = service;
    _logger = new Logger("trade");
}

TraderSpi::~TraderSpi()
{
    _service = NULL;
    _logger->info("TraderSpi[~]");
}

void TraderSpi::OnFrontConnected()
{
    _logger->info("TraderSpi[OnFrontConnected]");
    _service->login();
}


void TraderSpi::OnFrontDisconnected(int nReason)
{
    _logger->push("nReason", Lib::itos(nReason));
    _logger->info("TradeSpi[OnFrontDisconnected]");
}

void TraderSpi::OnHeartBeatWarning(int nTimeLapse)
{
    _logger->push("nTimeLapse", Lib::itos(nTimeLapse));
    _logger->info("TradeSpi[OnHeartBeatWarning]");
}

void TraderSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo && pRspInfo->ErrorID != 0) {
        _logger->error("TradeSpi[OnRspSettlementInfoConfirm]", pRspInfo, nRequestID, bIsLast);
    }

    _logger->push("ConfirmDate", string(pSettlementInfoConfirm->ConfirmDate));
    _logger->push("ConfirmTime", string(pSettlementInfoConfirm->ConfirmTime));
    _logger->info("TradeSpi[OnRspSettlementInfoConfirm]");
}

void TraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo && pRspInfo->ErrorID != 0) {
        _logger->error("TradeSpi[OnRspUserLogin]", pRspInfo, nRequestID, bIsLast);
    }
    _service->onLogin(pRspUserLogin);
    _service->confirm();

}

void TraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo && pRspInfo->ErrorID != 0) {
        _logger->error("TradeSpi[OnRspOrderInsert]", pRspInfo, nRequestID, bIsLast);
    }
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
    if (pRspInfo && pRspInfo->ErrorID != 0) {
        _logger->error("TradeSpi[OnRspQryInstrumentOrderCommRate]", pRspInfo, nRequestID, bIsLast);
    }
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
    if (pRspInfo && pRspInfo->ErrorID != 0) {
        _logger->error("TradeSpi[OnRspOrderAction]", pRspInfo, nRequestID, bIsLast);
    }
    _service->onCancelErr(pInputOrderAction, pRspInfo);
}

void TraderSpi::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder,
    CThostFtdcRspInfoField *pRspInfo)
{
    if (pRspInfo && pRspInfo->ErrorID != 0) {
        _logger->error("TradeSpi[OnErrRtnOrderInsert]", pRspInfo, 0, 1);
    }
}

void TraderSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pRspInfo && pRspInfo->ErrorID != 0) {
        _logger->error("TradeSpi[OnRspError]", pRspInfo, nRequestID, bIsLast);
    }
}

