#include "TradeSrv.h"
#include "TraderSpi.h"

TradeSrv::TradeSrv()
{
    _logger = new Logger("trade");

    string env = C::get("env");
    _brokerID = C::get("trade_broker_id_" + env);
    _userID = C::get("trade_user_id_" + env);
    _password = C::get("trade_password_" + env);
    _tradeFront = C::get("trade_front_" + env);
    _flowPath = C::get("flow_path_m");


    int db = Lib::stoi(C::get("rds_db_" + env));
    string host = C::get("rds_host_" + env);
    _rds = new Redis(host, 6379, db);
    _rdsLocal = new Redis("127.0.0.1", 6379, 1);

    _channelRsp = C::get("channel_trade_rsp");

    _reqID = 1;

}

TradeSrv::~TradeSrv()
{
    if (_tradeApi) {
        _tradeApi->RegisterSpi(NULL);
        _tradeApi->Release();
        _tradeApi = NULL;
    }
    if (_traderSpi) {
        delete _traderSpi;
    }
    _logger->info("TradeSrv[~]");
}

void TradeSrv::init()
{
    // 初始化交易接口
    _tradeApi = CThostFtdcTraderApi::CreateFtdcTraderApi(Lib::stoc(_flowPath));
    _traderSpi = new TraderSpi(this); // 初始化回调实例
    _tradeApi->RegisterSpi(_traderSpi);
    _tradeApi->SubscribePrivateTopic(THOST_TERT_QUICK);
    _tradeApi->SubscribePublicTopic(THOST_TERT_QUICK);

    _tradeApi->RegisterFront(Lib::stoc(_tradeFront));
    _tradeApi->Init();
}

void TradeSrv::confirm()
{
    CThostFtdcSettlementInfoConfirmField req = {0};
    strcpy(req.BrokerID, Lib::stoc(_brokerID));
    strcpy(req.InvestorID, Lib::stoc(_userID));
    string date = Lib::getDate("%Y%m%d");
    strcpy(req.ConfirmDate, date.c_str());
    string time = Lib::getDate("%H:%M:%S");
    strcpy(req.ConfirmTime, time.c_str());

    int res = _tradeApi->ReqSettlementInfoConfirm(&req, _reqID);
    _logger->request("TradeSrv[confirm]", _reqID++, res);
}

void TradeSrv::login()
{
    CThostFtdcReqUserLoginField req = {0};

    strcpy(req.BrokerID, Lib::stoc(_brokerID));
    strcpy(req.UserID, Lib::stoc(_userID));
    strcpy(req.Password, Lib::stoc(_password));

    int res = _tradeApi->ReqUserLogin(&req, _reqID);
    _logger->request("TradeSrv[login]", _reqID++, res);
}

void TradeSrv::onLogin(CThostFtdcRspUserLoginField * const rsp)
{
    if (!rsp) return;
    _frontID = rsp->FrontID;
    _sessionID = rsp->SessionID;
    _maxOrderRef = atoi(rsp->MaxOrderRef);

    _logger->push("FrontID", Lib::itos(rsp->FrontID));
    _logger->push("SessionID", Lib::itos(rsp->SessionID));
    _logger->push("MaxOrderRef", string(rsp->MaxOrderRef));
    _logger->info("TradeSrv[onLogin]");
}

void TradeSrv::onQryCommRate(CThostFtdcInstrumentOrderCommRateField * const rsp)
{
    if (!rsp) return;
    string iid = string(rsp->InstrumentID);
    double order = rsp->OrderCommByVolume;
    double cancel = rsp->OrderActionCommByVolume;

    _logger->push("iID", iid);
    _logger->push("Order", Lib::dtos(order));
    _logger->push("Cancel", Lib::dtos(cancel));
    _logger->info("TradeSrv[onQryCommRate]");

    Json::FastWriter writer;
    Json::Value data;

    data["type"] = "rate";
    data["iid"] = iid;
    data["order"] = order;
    data["cancel"] = cancel;

    std::string jsonStr = writer.write(data);
    _rdsLocal->push("Q_TRADE", jsonStr);
}

void TradeSrv::trade(double price, int total, bool isBuy, bool isOpen, int orderID, string instrumnetID, string type)
{
    if (_isOrderDealed(orderID)) return;
    usleep(1500);
    _initOrder(orderID, instrumnetID);

    TThostFtdcOffsetFlagEnType flag = THOST_FTDC_OFEN_Open;
    if (!isOpen) {
        flag = THOST_FTDC_OFEN_CloseToday;
    }

    TThostFtdcContingentConditionType condition = THOST_FTDC_CC_Immediately;
    TThostFtdcTimeConditionType timeCondition = THOST_FTDC_TC_GFD;
    TThostFtdcVolumeConditionType volumeCondition = THOST_FTDC_VC_AV;
    TThostFtdcOrderPriceTypeType priceType = THOST_FTDC_OPT_LimitPrice;

    if (type == "FOK") {
        timeCondition = THOST_FTDC_TC_IOC;
        volumeCondition = THOST_FTDC_VC_CV;
    }

    if (type == "FAK") {
        timeCondition = THOST_FTDC_TC_IOC;
        volumeCondition = THOST_FTDC_VC_AV;
    }

    if (type == "IOC") {
        if (isBuy) {
            string upper = _rdsLocal->get("UPPERLIMITPRICE_" + instrumnetID);
            price = Lib::stod(upper);
        } else {
            string lower = _rdsLocal->get("LOWERLIMITPRICE_" + instrumnetID);
            price = Lib::stod(lower);
        }
    }

    _logger->push("type", type);
    _logger->push("iid", instrumnetID);
    _logger->push("orderID", Lib::itos(orderID));
    _logger->push("orderRef", Lib::itos(_maxOrderRef));
    _logger->push("price", Lib::dtos(price));
    _logger->info("TradeSrv[trade]");

    CThostFtdcInputOrderField order = _createOrder(instrumnetID, isBuy, total, price, flag,
            THOST_FTDC_HFEN_Speculation, THOST_FTDC_OPT_LimitPrice, timeCondition, volumeCondition, condition);


    while (true) {
        int res = _tradeApi->ReqOrderInsert(&order, _maxOrderRef);
        _logger->request("TradeSrv[login]", _maxOrderRef, res);
        if (!res) {
            _orderIDDealed[orderID] = 1;
            break;
        }
    }

    Json::FastWriter writer;
    Json::Value data;

    string time = Lib::getDate("%Y/%m/%d-%H:%M:%S", true);
    data["type"] = "order";
    data["iid"] = instrumnetID;
    data["orderID"] = orderID;
    data["frontID"] = _frontID;
    data["sessionID"] = _sessionID;
    data["orderRef"] = _maxOrderRef;
    data["price"] = price;
    data["isBuy"] = (int)isBuy;
    data["isOpen"] = (int)isOpen;
    data["time"] = time;

    std::string jsonStr = writer.write(data);
    _rdsLocal->push("Q_TRADE", jsonStr);
}

void TradeSrv::onTraded(CThostFtdcTradeField * const rsp)
{

    if (!rsp) {
        return;
    }

    int orderRef = atoi(rsp->OrderRef);
    if (orderRef <= 0) {
        return;
    }

    int orderID = _getOrderIDByRef(orderRef);
    if (orderID <= 0) {
        return;
    }

    // 普通单
    CThostFtdcOrderField orderInfo = _getOrderInfoByRef(orderRef);
    if (strcmp(orderInfo.ExchangeID, rsp->ExchangeID) != 0 ||
        strcmp(orderInfo.OrderSysID, rsp->OrderSysID) != 0)
    { // 不是我的订单，我就不处理了
        return;
    }

    // log
    _logger->push("iid", string(rsp->InstrumentID));
    _logger->push("orderID", Lib::itos(orderID));
    _logger->push("orderRef", string(rsp->OrderRef));
    _logger->push("price", Lib::dtos(rsp->Price));
    _logger->push("tradeID", string(rsp->TradeID));
    _logger->push("orderSysID", string(rsp->OrderSysID));
    _logger->push("orderLocalID", string(rsp->OrderLocalID));
    _logger->push("tradeDate", string(rsp->TradeDate));
    _logger->push("tradeTime", string(rsp->TradeTime));
    _logger->push("TradingDay", string(rsp->TradingDay));
    _logger->info("TradeSrv[onTraded]");

    _clearOrderByRef(orderRef);

    Json::FastWriter writer;
    Json::Value data;

    string time = Lib::getDate("%Y/%m/%d-%H:%M:%S", true);
    data["type"] = "traded";
    data["iid"] = rsp->InstrumentID;
    data["orderID"] = orderID;
    data["realPrice"] = rsp->Price;

    std::string jsonStr = writer.write(data);
    _rds->pub(_channelRsp, jsonStr); // 回传消息

    data["orderRef"] = rsp->OrderRef;
    data["frontID"] = _frontID;
    data["sessionID"] = _sessionID;
    data["tradeDate"] = rsp->TradeDate;
    data["tradeTime"] = rsp->TradeTime;
    data["localTime"] = time;

    std::string qStr = writer.write(data);
    _rdsLocal->push("Q_TRADE", qStr); // 记录本地数据
}

void TradeSrv::onOrderRtn(CThostFtdcOrderField * const rsp)
{
    if (!rsp) {
        return;
    }
    if (rsp->SessionID != _sessionID) {
        return;
    }
    int orderRef = atoi(rsp->OrderRef);
    if (orderRef <= 0) {
        return;
    }
    int orderID = _getOrderIDByRef(orderRef);
    if (orderID <= 0) {
        return;
    }

    // log
    _logger->push("iid", string(rsp->InstrumentID));
    _logger->push("orderID", Lib::itos(orderID));
    _logger->push("orderRef", string(rsp->OrderRef));
    _logger->push("frontID", Lib::itos(rsp->FrontID));
    _logger->push("sessionID", Lib::itos(rsp->SessionID));
    _logger->push("orderSysID", string(rsp->OrderSysID));
    _logger->push("orderStatus", Lib::itos(rsp->OrderStatus));
    _logger->info("TradeSrv[onOrderRtn]");

    _updateOrderInfoByRef(orderRef, rsp);

    Json::FastWriter writer;
    Json::Value data;

    string time = Lib::getDate("%Y/%m/%d-%H:%M:%S", true);
    data["type"] = "order";
    data["iid"] = rsp->InstrumentID;
    data["orderID"] = orderID;
    data["orderRef"] = rsp->OrderRef;
    data["frontID"] = _frontID;
    data["sessionID"] = _sessionID;
    data["insertDate"] = rsp->InsertDate;
    data["insertTime"] = rsp->InsertTime;
    data["localTime"] = time;
    data["orderStatus"] = rsp->OrderStatus;

    std::string qStr = writer.write(data);
    _rdsLocal->push("Q_TRADE", qStr); // 记录本地数据

}

void TradeSrv::cancel(int orderID)
{
    if (_isOrderCanceled(orderID)) return;

    usleep(1500);

    int orderRef = _getOrderRefByID(orderID);
    if (orderRef <= 0) {
        return;
    }

    CThostFtdcOrderField orderInfo = _getOrderInfoByRef(orderRef);

    // log
    _logger->push("iid", string(orderInfo.InstrumentID));
    _logger->push("orderID", Lib::itos(orderID));
    _logger->push("orderRef", Lib::itos(orderRef));
    _logger->push("frontID", Lib::itos(orderInfo.FrontID));
    _logger->push("sessionID", Lib::itos(orderInfo.SessionID));
    _logger->push("orderSysID", string(orderInfo.OrderSysID));
    _logger->info("TradeSrv[cancel]");

    CThostFtdcInputOrderActionField req = {0};

    ///投资者代码
    strncpy(req.InvestorID, orderInfo.InvestorID,sizeof(TThostFtdcInvestorIDType));
    ///报单引用
    strncpy(req.OrderRef, orderInfo.OrderRef,sizeof(TThostFtdcOrderRefType));
    ///前置编号
    req.FrontID = orderInfo.FrontID;
    ///会话编号
    req.SessionID = orderInfo.SessionID;
    ///合约代码
    strncpy(req.InstrumentID, orderInfo.InstrumentID, sizeof(TThostFtdcInstrumentIDType));
    ///操作标志
    req.ActionFlag = THOST_FTDC_AF_Delete;

    ///经纪公司代码
    if (strlen(orderInfo.BrokerID) > 0)
        strncpy(req.BrokerID, orderInfo.BrokerID,sizeof(TThostFtdcBrokerIDType));
    ///交易所代码
    if (strlen(orderInfo.ExchangeID) > 0)
        strncpy(req.ExchangeID, orderInfo.ExchangeID, sizeof(TThostFtdcExchangeIDType));
    ///报单编号
    if (strlen(orderInfo.OrderSysID) > 0)
        strncpy(req.OrderSysID, orderInfo.OrderSysID, sizeof(TThostFtdcOrderSysIDType));

    while (true) {
        int res = _tradeApi->ReqOrderAction(&req, Lib::stoi(orderInfo.OrderRef));
        _logger->request("TradeSrv[cancel]", Lib::stoi(orderInfo.OrderRef), res);
        if (!res) {
            _orderIDCanceled[orderID] = 1;
            break;
        }
    }
}

void TradeSrv::onCancel(CThostFtdcOrderField * const rsp)
{

    if (!rsp) {
        return;
    }

    if (rsp->SessionID != _sessionID) {
        return;
    }

    int orderRef = atoi(rsp->OrderRef);
    if (orderRef <= 0) {
        return;
    }
    int orderID = _getOrderIDByRef(orderRef);
    if (orderID <= 0) {
        return;
    }

    // log
    _logger->push("iid", string(rsp->InstrumentID));
    _logger->push("orderID", Lib::itos(orderID));
    _logger->push("orderRef", string(rsp->OrderRef));
    _logger->push("frontID", Lib::itos(rsp->FrontID));
    _logger->push("sessionID", Lib::itos(rsp->SessionID));
    _logger->push("orderSysID", string(rsp->OrderSysID));
    _logger->push("orderStatus", Lib::itos(rsp->OrderStatus));
    _logger->push("limitPrice", Lib::dtos(rsp->LimitPrice));
    _logger->info("TradeSrv[onCancel]");

    _clearOrderByRef(orderRef);

    Json::FastWriter writer;
    Json::Value data;

    string time = Lib::getDate("%Y/%m/%d-%H:%M:%S", true);
    data["type"] = "canceled";
    data["iid"] = rsp->InstrumentID;
    data["orderID"] = orderID;
    data["price"] = rsp->LimitPrice;

    std::string jsonStr = writer.write(data);
    _rds->pub(_channelRsp, jsonStr); // 回传消息

    data["orderRef"] = rsp->OrderRef;
    data["frontID"] = _frontID;
    data["sessionID"] = _sessionID;
    data["insertDate"] = rsp->InsertDate;
    data["insertTime"] = rsp->InsertTime;
    data["localTime"] = time;
    data["orderStatus"] = rsp->OrderStatus;
    data["currentTick"] = _rdsLocal->get("CURRENT_TICK_" + string(rsp->InstrumentID));

    std::string qStr = writer.write(data);
    _rdsLocal->push("Q_TRADE", qStr); // 记录本地数据
}

void TradeSrv::onCancelErr(CThostFtdcInputOrderActionField * const rsp, CThostFtdcRspInfoField * const errInfo)
{

    if (!rsp) {
        return;
    }
    int orderRef = atoi(rsp->OrderRef);
    if (orderRef <= 0) {
        return;
    }
    int orderID = _getOrderIDByRef(orderRef);
    if (orderID <= 0) {
        return;
    }

    // log
    _logger->push("iid", string(rsp->InstrumentID));
    _logger->push("orderID", Lib::itos(orderID));
    _logger->push("errNo", Lib::itos(errInfo->ErrorID));
    _logger->push("orderActionRef", Lib::itos(rsp->OrderActionRef));
    _logger->push("orderRef", string(rsp->OrderRef));
    _logger->push("frontID", Lib::itos(rsp->FrontID));
    _logger->push("sessionID", Lib::itos(rsp->SessionID));
    _logger->push("orderSysID", string(rsp->OrderSysID));
    _logger->info("TradeSrv[onCancelErr]");

    Json::FastWriter writer;
    Json::Value data;

    data["type"] = "cancelErr";
    data["iid"] = rsp->InstrumentID;
    data["orderID"] = orderID;
    data["errNo"] = errInfo->ErrorID;

    std::string jsonStr = writer.write(data);
    _rds->pub(_channelRsp, jsonStr); // 回传消息

    _clearOrderByRef(orderRef);
}

void TradeSrv::onOrderErr(CThostFtdcInputOrderField * const rsp, CThostFtdcRspInfoField * const errInfo)
{

    if (!rsp) {
        return;
    }
    int orderRef = atoi(rsp->OrderRef);
    if (orderRef <= 0) {
        return;
    }
    int orderID = _getOrderIDByRef(orderRef);
    if (orderID <= 0) {
        return;
    }

    // log
    _logger->push("iid", string(rsp->InstrumentID));
    _logger->push("orderID", Lib::itos(orderID));
    _logger->push("errNo", Lib::itos(errInfo->ErrorID));
    _logger->push("orderRef", string(rsp->OrderRef));
    _logger->push("frontID", Lib::itos(_frontID));
    _logger->push("sessionID", Lib::itos(_sessionID));
    _logger->info("TradeSrv[onOrderErr]");

    Json::FastWriter writer;
    Json::Value data;

    data["type"] = "orderErr";
    data["iid"] = rsp->InstrumentID;
    data["orderID"] = orderID;
    data["errNo"] = errInfo->ErrorID;

    std::string jsonStr = writer.write(data);
    _rds->pub(_channelRsp, jsonStr); // 回传消息

    _clearOrderByRef(orderRef);
}

void TradeSrv::_initOrder(int orderID, string iID)
{
    _logger->push("iid", iID);
    _logger->push("orderID", Lib::itos(orderID));

    // 查询手续费
    std::map<string, int>::iterator i = _rate.find(iID);
    if (i == _rate.end()) {
        _logger->push("isFirst", "True");
        _rate[iID] = 1;
        CThostFtdcQryInstrumentOrderCommRateField req = {0};

        strcpy(req.BrokerID, Lib::stoc(_brokerID));
        strcpy(req.InvestorID, Lib::stoc(_userID));
        strcpy(req.InstrumentID, Lib::stoc(iID));

        int res = _tradeApi->ReqQryInstrumentOrderCommRate(&req, _reqID);
        _logger->request("TradeSrv[rate]", _reqID++, res);
    }


    _maxOrderRef++;
    _logger->push("orderRef", Lib::itos(_maxOrderRef));
    _logger->info("TradeSrv[initOrder]");

    _orderRef2ID[_maxOrderRef] = orderID;
    _orderID2Ref[orderID] = _maxOrderRef;

    CThostFtdcOrderField data = {0};
    data.FrontID = _frontID;
    data.SessionID = _sessionID;
    sprintf(data.OrderRef, "%d", _maxOrderRef);
    strcpy(data.InstrumentID, iID.c_str());
    strcpy(data.InvestorID, _userID.c_str());

    _orderRef2Info[_maxOrderRef] = data;
}

void TradeSrv::_clearOrderByRef(int orderRef)
{
    int orderID = _getOrderIDByRef(orderRef);

    std::map<int, int>::iterator i2i;
    i2i = _orderRef2ID.find(orderRef);
    if (i2i != _orderRef2ID.end()) _orderRef2ID.erase(i2i);
    i2i = _orderID2Ref.find(orderID);
    if (i2i != _orderID2Ref.end()) _orderID2Ref.erase(i2i);

    std::map<int, CThostFtdcOrderField>::iterator i2O;
    i2O = _orderRef2Info.find(orderRef);
    if (i2O != _orderRef2Info.end()) _orderRef2Info.erase(i2O);

}

int TradeSrv::_getOrderIDByRef(int orderRef)
{
    std::map<int, int>::iterator i = _orderRef2ID.find(orderRef);
    if (i != _orderRef2ID.end()) return i->second;
    return 0;
}

int TradeSrv::_getOrderRefByID(int orderID)
{
    std::map<int, int>::iterator i = _orderID2Ref.find(orderID);
    if (i != _orderID2Ref.end()) return i->second;
    return 0;
}


void TradeSrv::_updateOrderInfoByRef(int orderRef, CThostFtdcOrderField * const info)
{
    _orderRef2Info[orderRef] = *info;
}

CThostFtdcOrderField TradeSrv::_getOrderInfoByRef(int orderRef)
{
    std::map<int, CThostFtdcOrderField>::iterator i = _orderRef2Info.find(orderRef);
    if (i != _orderRef2Info.end()) return i->second;
    CThostFtdcOrderField info = {0};
    return info;
}

CThostFtdcInputOrderField TradeSrv::_createOrder(string instrumnetID, bool isBuy, int total, double price,
    // double stopPrice,
    TThostFtdcOffsetFlagEnType offsetFlag, // 开平标志
    TThostFtdcHedgeFlagEnType hedgeFlag, // 投机套保标志
    TThostFtdcOrderPriceTypeType priceType, // 报单价格条件
    TThostFtdcTimeConditionType timeCondition, // 有效期类型
    TThostFtdcVolumeConditionType volumeCondition, //成交量类型
    TThostFtdcContingentConditionType contingentCondition// 触发条件
    )
{
    CThostFtdcInputOrderField order = {0};

    strcpy(order.BrokerID, _brokerID.c_str()); ///经纪公司代码
    strcpy(order.InvestorID, _userID.c_str()); ///投资者代码
    strcpy(order.InstrumentID, instrumnetID.c_str()); ///合约代码
    strcpy(order.UserID, _userID.c_str()); ///用户代码
    // strcpy(order.ExchangeID, "SHFE"); ///交易所代码

    order.MinVolume = 1;///最小成交量
    order.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;///强平原因
    order.IsAutoSuspend = 0;///自动挂起标志
    order.UserForceClose = 0;///用户强评标志
    order.IsSwapOrder = 0;///互换单标志

    order.Direction = isBuy ? THOST_FTDC_D_Buy : THOST_FTDC_D_Sell; ///买卖方向
    order.VolumeTotalOriginal = total;///数量
    order.LimitPrice = price;///价格
    order.StopPrice = 0;///止损价
    if (contingentCondition != THOST_FTDC_CC_Immediately) {
        order.StopPrice = price;///止损价
    }

    ///组合开平标志
    //THOST_FTDC_OFEN_Open 开仓
    //THOST_FTDC_OFEN_Close 平仓
    //THOST_FTDC_OFEN_ForceClose 强平
    //THOST_FTDC_OFEN_CloseToday 平今
    //THOST_FTDC_OFEN_CloseYesterday 平昨
    //THOST_FTDC_OFEN_ForceOff 强减
    //THOST_FTDC_OFEN_LocalForceClose 本地强平
    order.CombOffsetFlag[0] = offsetFlag;
    if (THOST_FTDC_OFEN_ForceClose == offsetFlag) {
        order.ForceCloseReason = THOST_FTDC_FCC_Other; // 其他
        order.UserForceClose = 1;
    }

    ///组合投机套保标志
    // THOST_FTDC_HFEN_Speculation 投机
    // THOST_FTDC_HFEN_Arbitrage 套利
    // THOST_FTDC_HFEN_Hedge 套保
    order.CombHedgeFlag[0] = hedgeFlag;

    ///报单价格条件
    // THOST_FTDC_OPT_AnyPrice 任意价
    // THOST_FTDC_OPT_LimitPrice 限价
    // THOST_FTDC_OPT_BestPrice 最优价
    // THOST_FTDC_OPT_LastPrice 最新价
    // THOST_FTDC_OPT_LastPricePlusOneTicks 最新价浮动上浮1个ticks
    // THOST_FTDC_OPT_LastPricePlusTwoTicks 最新价浮动上浮2个ticks
    // THOST_FTDC_OPT_LastPricePlusThreeTicks 最新价浮动上浮3个ticks
    // THOST_FTDC_OPT_AskPrice1 卖一价
    // THOST_FTDC_OPT_AskPrice1PlusOneTicks 卖一价浮动上浮1个ticks
    // THOST_FTDC_OPT_AskPrice1PlusTwoTicks 卖一价浮动上浮2个ticks
    // THOST_FTDC_OPT_AskPrice1PlusThreeTicks 卖一价浮动上浮3个ticks
    // THOST_FTDC_OPT_BidPrice1 买一价
    // THOST_FTDC_OPT_BidPrice1PlusOneTicks 买一价浮动上浮1个ticks
    // THOST_FTDC_OPT_BidPrice1PlusTwoTicks 买一价浮动上浮2个ticks
    // THOST_FTDC_OPT_BidPrice1PlusThreeTicks 买一价浮动上浮3个ticks
    // THOST_FTDC_OPT_FiveLevelPrice 五档价
    order.OrderPriceType = priceType;

    ///有效期类型
    // THOST_FTDC_TC_IOC 立即完成，否则撤销
    // THOST_FTDC_TC_GFS 本节有效
    // THOST_FTDC_TC_GFD 当日有效
    // THOST_FTDC_TC_GTD 指定日期前有效
    // THOST_FTDC_TC_GTC 撤销前有效
    // THOST_FTDC_TC_GFA 集合竞价有效
    order.TimeCondition = timeCondition;

    ///成交量类型
    // THOST_FTDC_VC_AV 任何数量
    // THOST_FTDC_VC_MV 最小数量
    // THOST_FTDC_VC_CV 全部数量
    order.VolumeCondition = volumeCondition;

    ///触发条件
    // THOST_FTDC_CC_Immediately 立即
    // THOST_FTDC_CC_Touch 止损
    // THOST_FTDC_CC_TouchProfit 止赢
    // THOST_FTDC_CC_ParkedOrder 预埋单
    // THOST_FTDC_CC_LastPriceGreaterThanStopPrice 最新价大于条件价
    // THOST_FTDC_CC_LastPriceGreaterEqualStopPrice 最新价大于等于条件价
    // THOST_FTDC_CC_LastPriceLesserThanStopPrice 最新价小于条件价
    // THOST_FTDC_CC_LastPriceLesserEqualStopPrice 最新价小于等于条件价
    // THOST_FTDC_CC_AskPriceGreaterThanStopPrice 卖一价大于条件价
    // THOST_FTDC_CC_AskPriceGreaterEqualStopPrice 卖一价大于等于条件价
    // THOST_FTDC_CC_AskPriceLesserThanStopPrice 卖一价小于条件价
    // THOST_FTDC_CC_AskPriceLesserEqualStopPrice 卖一价小于等于条件价
    // THOST_FTDC_CC_BidPriceGreaterThanStopPrice 买一价大于条件价
    // THOST_FTDC_CC_BidPriceGreaterEqualStopPrice 买一价大于等于条件价
    // THOST_FTDC_CC_BidPriceLesserThanStopPrice 买一价小于条件价
    // THOST_FTDC_CC_BidPriceLesserEqualStopPrice 买一价小于等于条件价
    order.ContingentCondition = contingentCondition;

    ///报单引用
    sprintf(order.OrderRef, "%d", _maxOrderRef);

    ///请求编号
    // _reqID++;
    order.RequestID = _maxOrderRef;

    // order.GTDDate = ;///GTD日期
    // order.BusinessUnit = ;///业务单元
    // order.InvestUnitID = ;///投资单元代码
    // order.AccountID = ;///资金账号
    // order.CurrencyID = ;///币种代码
    // order.ClientID = ;///交易编码
    // order.IPAddress = ;///IP地址
    // order.MacAddress = ;///Mac地址

    return order;
}

bool TradeSrv::_isOrderDealed(int orderID)
{
    std::map<int, int>::iterator i = _orderIDDealed.find(orderID);
    if (i != _orderIDDealed.end()) return true;
    return false;
}

bool TradeSrv::_isOrderCanceled(int orderID)
{
    std::map<int, int>::iterator i = _orderIDCanceled.find(orderID);
    if (i != _orderIDCanceled.end()) return true;
    return false;
}
