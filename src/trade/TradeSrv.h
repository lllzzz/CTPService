#ifndef TRADE_SRV_H
#define TRADE_SRV_H

#include "../../include/ThostFtdcTraderApi.h"
#include "../strategy/global.h"
#include "../global.h"
#include "../libs/Redis.h"
#include <map>


class TraderSpi;

class TradeSrv
{
private:

    string _logPath;
    Redis * _store;

    string _brokerID;
    string _userID;
    string _password;
    string _tradeFront;
    string _flowPath;


    TThostFtdcFrontIDType _frontID;
    TThostFtdcSessionIDType _sessionID;
    int _maxOrderRef;

    CThostFtdcTraderApi * _tradeApi;
    TraderSpi * _traderSpi;
    QClient * _tradeStrategySrvClient;

    map<int, CThostFtdcOrderField> _orderRef2Info;// orderRef -> OrderInfo
    map<int, int> _orderRef2ID; // orderRef -> orderID
    map<int, int> _orderID2Ref; // orderID -> orderRef
    map<int, int> _orderIDDealed; // orderID -> 1
    map<int, int> _orderIDCanceled; // orderID -> 1
    map<string, int> _rate;

    void _showData();
    bool _isOrderDealed(int);
    bool _isOrderCanceled(int);

    void _initOrder(int, string); // 初始化Order记录
    void _clearOrderByRef(int); // 清理Order记录
    int _getOrderIDByRef(int);
    int _getOrderRefByID(int);
    void _updateOrderInfoByRef(int, CThostFtdcOrderField * const); // 更新OrderInfo记录
    CThostFtdcOrderField _getOrderInfoByRef(int);

    CThostFtdcInputOrderField _createOrder(string, bool, int, double,
        TThostFtdcOffsetFlagEnType, // 开平标志
        TThostFtdcHedgeFlagEnType = THOST_FTDC_HFEN_Speculation, // 投机套保标志
        TThostFtdcOrderPriceTypeType = THOST_FTDC_OPT_LimitPrice, // 报单价格条件
        TThostFtdcTimeConditionType = THOST_FTDC_TC_IOC, // 有效期类型
        TThostFtdcVolumeConditionType = THOST_FTDC_VC_CV, //成交量类型
        TThostFtdcContingentConditionType = THOST_FTDC_CC_Immediately// 触发条件
    );

public:

    TradeSrv(string, string, string, string, string, string, string, int, int);
    ~TradeSrv();

    void init();

    void confirm();
    void login();
    void onLogin(CThostFtdcRspUserLoginField * const);

    void trade(double, int, bool, bool, int, string, bool);
    void onTraded(CThostFtdcTradeField * const);
    void onOrderRtn(CThostFtdcOrderField * const);
    void onOrderErr(CThostFtdcInputOrderField * const, CThostFtdcRspInfoField * const);

    void cancel(int);
    void onCancel(CThostFtdcOrderField * const);
    void onCancelErr(CThostFtdcInputOrderActionField * const, CThostFtdcRspInfoField * const);

    void onQryCommRate(CThostFtdcInstrumentOrderCommRateField * const);
};

#endif
