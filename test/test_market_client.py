#!/usr/bin/env python
# -*- encoding:utf-8 -*-
import os
import sys

from redis import Redis
import demjson as JSON
import time


host = '127.0.0.1'
db = 7
rds = Redis(host = host, port = 6379, db = db)

# 订阅行情
client = rds.pubsub()
client.subscribe(['tick_ni1609', 'tick_hc1610'])

# 监听行情
for msg in client.listen():
    if msg['type'] == 'message':
        data = msg['data']
        print data
