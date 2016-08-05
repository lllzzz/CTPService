#ifndef TRADE_SRV_H
#define TRADE_SRV_H

#include "../../include/ThostFtdcTraderApi.h"
#include "../global.h"

#define CODE_SUCCESS 0
#define CODE_ERR_ORDER_EXIST 1001
#define CODE_ERR_ORDER_NOT_EXIST 1002
#define CODE_ERR_OVER_CLOSETODAY_POSITION 1003
#define CODE_ERR_INSUITABLE_ORDER_STATUS 1004

#define ORDER_TYPE_NORMAL 0
#define ORDER_TYPE_FAK 1
#define ORDER_TYPE_IOC 2
#define ORDER_TYPE_FOK 3

typedef struct OrderInfo
{
    int appKey;
    int orderID;
    int orderRef;
    string iid;
    TThostFtdcExchangeIDType eID;
    TThostFtdcOrderSysIDType oID;
    TThostFtdcInvestorIDType iID;
    TThostFtdcOrderRefType   oRef;

} OrderInfo;

class TradeSrv : public CThostFtdcTraderSpi
{
private:

    Logger * _logger;
    CThostFtdcTraderApi * _tApi;

    Redis * _rds; // 用于通知的redis实例
    Redis * _rdsLocal; // 本地redis实例

    string _brokerID;
    string _userID;
    string _password;
    string _tradeFront;
    string _flowPath;

    int _frontID;
    int _sessionID;
    int _maxOrderRef;
    int _reqID;

    // 通知结果的频道
    string _channelRsp;

    // 构造订单
    CThostFtdcInputOrderField _createOrder(string, bool, int, double,
        TThostFtdcOffsetFlagEnType, // 开平标志
        TThostFtdcHedgeFlagEnType = THOST_FTDC_HFEN_Speculation, // 投机套保标志
        TThostFtdcOrderPriceTypeType = THOST_FTDC_OPT_LimitPrice, // 报单价格条件
        TThostFtdcTimeConditionType = THOST_FTDC_TC_IOC, // 有效期类型
        TThostFtdcVolumeConditionType = THOST_FTDC_VC_CV, //成交量类型
        TThostFtdcContingentConditionType = THOST_FTDC_CC_Immediately// 触发条件
    );

    // 处理订单反馈
    void _onOrder(CThostFtdcOrderField *);
    void _onCancel(CThostFtdcOrderField *);

    // 客户端订单
    std::map<int, std::map<int, OrderInfo> > _orderInfoViaAO; // appKey => orderID => orderRef
    std::map<int, OrderInfo> _orderInfoViaR;
    void _initOrder(int, int, string);
    bool _isExistOrder(int, int);
    OrderInfo _getOrderByRef(int);
    void _updateOrder(int, CThostFtdcOrderField *);

    // 返回通知
    void _rspMsg(int, int, string, Json::Value* = NULL);

public:

    TradeSrv();
    ~TradeSrv();

    // 初始化回调接口
    void OnFrontConnected();
    void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    // 下单/撤单
    void trade(int, int, string, bool, bool, int, double, int); // ReqOrderInsert
    void cancel(int, int); // ReqOrderAction
    void OnRtnOrder(CThostFtdcOrderField *pOrder);
    void OnRtnTrade(CThostFtdcTradeField *pTrade);

    // // 手续费查询
    // void qryCommissionRate(); // ReqQryInstrumentCommissionRate
    // void OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};

    // // 持仓查询
    // void qryPosition(); // ReqQryInvestorPosition
    // void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};


    // 异常
    void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo);
    void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnFrontDisconnected(int nReason);
    void OnHeartBeatWarning(int nTimeLapse);

};

#endif
