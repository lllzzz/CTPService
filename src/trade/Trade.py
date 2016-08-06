#!/usr/bin/env python
# -*- encoding:utf-8 -*-
import os
import sys
sys.path.append('../src/libs/')
import warnings
warnings.filterwarnings('ignore')
import re

from Config import Config as C
from DB import DB
from redis import Redis
import demjson as JSON
import time

class Trade():
    """Market相关持久化方法，以及脚本调度"""
    def __init__(self):
        self._db = DB(DB.TYPE_TRADE)
        pass

    def start(self):
        self.__initDB()
        os.system('./tradeSrv &')
        pass

    def stop(self):
        env = C.get('env');
        host = C.get('rds_host_' + env)
        db = C.get('rds_db_' + env)
        rds = Redis(host = host, port = 6379, db = db)
        tradeChannel = C.get('channel_trade')
        rds.publish(tradeChannel, 'stop');


    def __cTime2DBTime(self, cTime):
        timestamp = time.mktime(time.strptime(cTime, '%Y%m%d %H:%M:%S'))
        return time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(timestamp))


    def processQ(self, data):
        sql = '''
            INSERT INTO `tick_%s` (`iid`, `time`, `msec`, `price`, `volume`,
            `bid_price1`, `bid_volume1`, `ask_price1`, `ask_volume1`) VALUES ('%s',
            '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')''' % (data['iid'],
              data['iid'], self.__cTime2DBTime(data['time']), data['msec'], data['price'],
              data['vol'], data['bid1'], data['bidvol1'], data['ask1'],
              data['askvol1'])
        self._db.insert(sql)


    def __initDB(self):
        sql = '''
          CREATE TABLE IF NOT EXISTS `order` (
              `id` int(11) NOT NULL AUTO_INCREMENT,
              `appKey` varchar(50) NOT NULL DEFAULT '',
              `instrumnet_id` varchar(50) NOT NULL DEFAULT '',
              `order_id` int(11) NOT NULL DEFAULT '0',
              `front_id` int(11) NOT NULL DEFAULT '0',
              `session_id` int(11) NOT NULL DEFAULT '0',
              `order_ref` int(11) NOT NULL DEFAULT '0',
              `price` decimal(10,2) NOT NULL DEFAULT '0.00',
              `cancel_tick_price` decimal(10,2) NOT NULL,
              `real_price` decimal(10,2) NOT NULL,
              `is_buy` int(1) NOT NULL DEFAULT '0',
              `is_open` int(11) NOT NULL DEFAULT '0',
              `srv_insert_time` datetime NOT NULL COMMENT '服务器返回的insert时间',
              `srv_traded_time` datetime NOT NULL COMMENT '服务器返回的成交',
              `start_time` datetime NOT NULL COMMENT '发出交易指令时间',
              `start_usec` int(11) NOT NULL DEFAULT '0',
              `first_time` datetime NOT NULL COMMENT '首次得到交易回馈时间',
              `first_usec` int(11) NOT NULL DEFAULT '0',
              `end_time` datetime NOT NULL COMMENT '首次得到交易回馈时间',
              `end_usec` int(11) NOT NULL DEFAULT '0',
              `status` int(11) NOT NULL DEFAULT '0',
              `mtime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
              PRIMARY KEY (`id`),
              KEY `idx_front_session_ref` (`front_id`,`session_id`,`order_ref`)
            ) ENGINE=InnoDB AUTO_INCREMENT=9458 DEFAULT CHARSET=utf8;'''
        self._db.insert(sql)
