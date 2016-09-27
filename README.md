# CTPService

系统分为MarketService（MS）与TradeService（TS）两个部分。

MS部分负责行情数据（tick）的收集，分发与存储，是对ctp接口的封装。分发依赖redis的消息订阅，第三方模型通过订阅频道来获取tick数据，从而进行下单。存储部分通过异步队列，将tick保存在数据库中。

TS部分作为交易中心，负责交易指令的发送，与订单状态的保存，与MS相同，TS作为服务，通过redis监听固定频道来进行交易处理，落地采用异步队列形式。另外还提供一些查询命令。

###启动说明
启动命令均在bin目录下

***./ctpService start/stop default*** 该命令启动/停止MS与TS服务

***./ctpService startQ/stopQ default*** 该命令启动/停止消费队列，包括MS与TS

***./ctpService status default*** 查看服务情况

***./check.py*** 检查python依赖的包是否已经安装

***./cron/qryRate.py*** 查询合约手续费，该命令在crontab中有设定

另外crontab文件中的命令，可以直接copy到系统crontab中，从而定时启动任务

###配置说明
配置文件位于etc目录下。

env为环境配置，当前仅支持dev与online，及开发与线上环境。

####Redis
对于Redis，用于本地储存的队列以及一些系统内的数据共享，直接选择了db = 1，不可配置，对于与其他系统的交互，包括tick的传输与交易信号的传输，提供了分环境的配置项，建议不要与使用db = 1

频道配置channel_tick为tick信号的广播频道，配置格式为 ***tick_合约ID***，channel_trade为订阅的订单频道，其他系统通过该频道广播下单信息，格式 ***trade***，配置channel_trade_rsp为下单信息反馈的广播频道，第三方系统下单之后，可以通过订阅该频道接收下单的回馈，格式为***trade_rsp_appkey*** 其中appkey为第三方系统的标示。

####Mysql
根据环境分别配置即可。

####CTP相关配置
配置虚拟盘与实盘的相关登录信息，系统根据env进行自动获取

####iids
需要收集行情的合约配置，多个用 **/** 分割

####path
路径相关的配置


###交互格式
第三方系统通过订阅或者发布信息到指定频道，与本系统进行交互，交互格式均为Json格式，以下做出样例。

####MS tick数据格式

    {
        iid: 'hc1701', //合约ID
        price: 1892, // 最新价
        vol: 1002, // 成交量
        time: 20160809_10:20:54, // CTP服务端返回时间
        msec: 500, // 当前毫秒
        bid1: 1892,
        bidvol1: 1000,
        ask1: 1892,
        askvol1: 1000,
    }

####TS 监听格式
    // 下单与撤单
    {
        action: 'trade', // trade下单，cancel撤单
        appKey: 100, // 整型，第三方服务id
        orderID: 100, // 整型，第三方服务提供的下单ID，用于标示下单或撤单
        // 以下参数均为action为trade时需要
        iid: 'hc1701',
        type: 1, // 下单类型，0：普通单，1：FAK，2：IOC，3：FOK（慎用，有的交易所不支持）
        price: 1890, //double，下单价
        total: 1, // 下几手
        isBuy: 1, // 是否买 1/0
        isOpen: 1, // 是否开仓 1/0
    }
    // 查询合约手续费
    {
        action: 'qryRate',
        appKey: 100,
        iid: 'hc1701',
    }

####TS 返回格式

    {
        err: 0, // 非零失败
        msg: 'hahah', // 错误信息
        data: obj // 成功时返回信息
    }

##### 成功信息

    // 下单反馈
    {
        type: 'traded',
        iid: 'hc1701',
        orderID: 100,
        realPrice: 1000, // 实际成交金额
        successVol: 2, // 成功交易几手
    }

    // 撤单反馈
    {
        type: 'canceled',
        iid: 'hc1701',
        orderID: 100,
        price: 10000, // 当前最新价
        cancelVol: 3， // 撤单笔数
    }

    // 查询合约手续费
    {
        type: 'rate',
        iid: 'hc1701',
        openByMoney: 100,
        openByVol: 100,
        closeByMoney: 100,
        closeByVol: 100,
        closeTodayByMoney: 100,
        closeTodayByVol: 100,
    }

