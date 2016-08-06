#!/usr/bin/env python
# -*- encoding:utf-8 -*-
import os
import sys

from redis import Redis
import demjson as JSON
import time

NORMAL = 0
FAK = 1
IOC = 2
FOK = 3

appKey = 100
host = '127.0.0.1'
db = 7
rds = Redis(host = host, db = db, port = 6379)

orderID = 1;

# 订阅交易回馈
client = rds.pubsub()
client.subscribe('trade_rsp_' + str(appKey))

# 发送一笔订单
data = {
    'action': 'trade',
    'appKey': appKey,
    'orderID': orderID,
    'iid': 'ni1609',
    'type': IOC,
    'price': 500,
    'total': 1,
    'isBuy': 1,
    'isOpen': 1,
}
rds.publish('trade', JSON.encode(data))

# 监听反馈
for msg in client.listen():
    if msg['type'] == 'message':
        print msg['data']
        data = JSON.decode(msg['data'])
        print data['err']
        print data['msg']
        break;
