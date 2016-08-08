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
iid = 'ni1609'
isBuy = 1
isOpen = 1


# 订阅交易回馈
client = rds.pubsub()
client.subscribe('trade_rsp_' + str(appKey))

# 发送一笔订单
data = {
    'action': 'trade',
    'appKey': appKey,
    'orderID': orderID,
    'iid': iid,
    'type': IOC,
    'price': 0,
    'total': 1,
    'isBuy': isBuy,
    'isOpen': isOpen,
}
rds.publish('trade', JSON.encode(data))

maxCnt = 101

# 监听反馈
for msg in client.listen():
    if msg['type'] == 'message':
        if (maxCnt == 0): break
        maxCnt -= 1
        print msg['data']
        data = JSON.decode(msg['data'])
        if (data['err']):
            print data['msg']
            break;
        else:
            isBuy = 1 if isBuy == 0 else 0
            isOpen = 1 if isOpen == 0 else 0
            orderID += 1
            data = data['data']
            print data['orderID']
            print data['realPrice']
            newData = {
                'action': 'trade',
                'appKey': appKey,
                'orderID': orderID,
                'iid': iid,
                'type': IOC,
                'price': 0,
                'total': 1,
                'isBuy': isBuy,
                'isOpen': isOpen,
            }
            rds.publish('trade', JSON.encode(newData))
