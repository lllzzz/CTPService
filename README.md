# ctpTrading

系统分为MarketService与TradeService两个部分。

ms部分负责tick的收集，分发与存储，是对ctp接口的封装，分发依赖redis的消息订阅，第三方模型通过订阅频道来获取tick数据，从而进行下单，存储部分通过异步队列，将tick保存在数据库中。

ts部分作为交易中心，负责交易指令的发送，与订单状态的保存，与ms相同，ts作为服务，通过redis监听固定频道来进行交易处理，落地采用异步队列形式。



###  MarketService说明：
bin目录./ctpService start启动服务，配置文件位于etc/config.ini。停止服务命令./ctpService stop。

消息队列为本地redis，db为1，这个是固定的，暂时不可配置，在队列中写入db，db可配置，启动命令为./ctpService queue start，停止服务为./ctpService queue stop
