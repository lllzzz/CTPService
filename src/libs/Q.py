#!/usr/bin/env python
# -*- encoding:utf-8 -*-

import os
import sys
sys.path.append('../src/libs/')
from Config import Config as C
import demjson as JSON
from redis import Redis
import re
import time

class Q():
    """消化队列"""
    def __init__(self, key, obj):
        self.key = key
        self.obj = obj
        self.rds = Redis(host = '127.0.0.1', port = 6379, db = 1)

    def run(self):
        while True:
            data = self.rds.rpop(self.key)
            if data:
                dataObj = JSON.decode(data)
                self.obj.processQ(dataObj)
            else:
                time.sleep(1)
