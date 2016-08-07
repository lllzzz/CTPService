#!/usr/bin/env python
# -*- encoding:utf-8 -*-
import os
import sys
sys.path.append('../src/libs/')

from Config import Config as C
from redis import Redis
import demjson as JSON
import time

IOC = 2

env = sys.argv[1]
appKey = sys.argv[2]
orderID = sys.argv[3]
host = C.get('rds_host_' + env)
db = C.get('rds_db_' + env)
rds = Redis(host = host, db = db, port = 6379)

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
    'isBuy': 1,
    'isOpen': 1,
}

rds.publish('trade', JSON.encode(data))

# 监听反馈
for msg in client.listen():
    if msg['type'] == 'message':
        print msg['data']
        break;
