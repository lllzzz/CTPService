#!/usr/bin/env python
# -*- encoding:utf-8 -*-
import os
import sys

from redis import Redis
import demjson as JSON
import time

NORMAL = 0

appKey = 100
host = '127.0.0.1'
db = 7
rds = Redis(host = host, db = db, port = 6379)

orderID = 1;

tradeType = sys.argv[1]
tradeType = locals()[tradeType]
price = sys.argv[2]

# 订阅交易回馈
client = rds.pubsub()
client.subscribe('trade_rsp_' + str(appKey))

# 发送一笔订单
data = {
    'action': 'trade',
    'appKey': appKey,
    'orderID': orderID,
    'iid': 'ni1609',
    'type': tradeType,
    'price': int(price),
    'total': 1,
    'isBuy': 1,
    'isOpen': 1,
}
rds.publish('trade', JSON.encode(data))

time.sleep(1)
# 撤单
data = {
    'action': 'cancel',
    'appKey': appKey,
    'orderID': orderID,
}
rds.publish('trade', JSON.encode(data))

# 监听反馈
for msg in client.listen():
    if msg['type'] == 'message':
        print msg['data']
        break;
