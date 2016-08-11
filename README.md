# ctpService

系统分为MarketService与TradeService两个部分。

ms部分负责tick的收集，分发与存储，是对ctp接口的封装，分发依赖redis的消息订阅，第三方模型通过订阅频道来获取tick数据，从而进行下单，存储部分通过异步队列，将tick保存在数据库中。

ts部分作为交易中心，负责交易指令的发送，与订单状态的保存，与ms相同，ts作为服务，通过redis监听固定频道来进行交易处理，落地采用异步队列形式。

###  服务说明：

首先需要 cd bin/ 下对代码进行编译，直接执行 ./build.sh 即可，此时会有markerSrv与tradeSrv两个可执行文件，然后可以执行如下命令启动服务。

./ctpService start启动服务，配置文件位于etc/config.ini。

停止服务命令./ctpService stop。

查看状态./ctpService status（其实就是ps）。

启动队列./ctpService q start。停止队列./ctpService q stop。

启动服务的时候会自动建表，消息队列需要单独启动。ctpService为src目录下ctpService.py的软连接，负责服务的调度。


###  工具：
tools目录下提供工具，./ioc_close.py为强平工具，使用方法./ioc_close env appKey orderID iid
