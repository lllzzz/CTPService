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

    STATUS_UNKNOW = 0
    STATUS_ALL_TRADED = 1
    STATUS_ALL_CANCELED = 2

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
        timestamp = time.mktime(time.strptime(cTime, '%Y/%m/%d-%H:%M:%S'))
        return time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(timestamp))

    def __buildDBTime(self, cDate, cTime):
        timestamp = time.mktime(time.strptime(cDate + ' ' + cTime, '%Y%m%d %H:%M:%S'))
        return time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(timestamp))


    def processQ(self, data):

        type = data['type']

        if type == 'trade':
            t, u = data['time'].split('.')
            sql = '''
                INSERT INTO `order` (`appKey`, `iid`, `order_id`, `front_id`, `session_id`, `order_ref`,
                `price`, `total`, `is_buy`, `is_open`, `local_start_time`, `local_start_usec`) VALUES ('%s',
                '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')''' % (data['appKey'], data['iid'],
                data['orderID'], data['frontID'], data['sessionID'], data['orderRef'], data['price'], data['total'],
                data['isBuy'], data['isOpen'], self.__cTime2DBTime(t), u);
            self._db.insert(sql)

        elif type == 'traded':
            t, u = data['localTime'].split('.')
            sql = '''
                UPDATE `order` SET `srv_end_time` = '%s', `local_end_time` = '%s', `local_end_usec` = '%s', `real_price` = '%s', `real_total` = %s,
                `status` = %s WHERE  `order_id` = '%s' AND `front_id` = '%s' AND `session_id` = '%s' ''' % (self.__buildDBTime(data['tradeDate'],
                data['tradeTime']), self.__cTime2DBTime(t), u, data['realPrice'], 1, self.STATUS_ALL_TRADED,  data['orderID'], data['frontID'],
                data['sessionID'])
            self._db.update(sql)
            pass

        elif type == 'order':
            t, u = data['localTime'].split('.')
            sql = '''
                UPDATE `order` SET `srv_first_time` = '%s', `local_first_time` = '%s', `local_first_usec` = '%s'
                WHERE  `order_id` = '%s' AND `front_id` = '%s' AND `session_id` = '%s' AND `srv_first_time` = 0 ''' % (self.__buildDBTime(data['insertDate'],
                data['insertTime']), self.__cTime2DBTime(t), u, data['orderID'], data['frontID'], data['sessionID'])
            self._db.update(sql)

        elif type == 'canceled':
            t, u = data['localTime'].split('.')
            sql = '''
                UPDATE `order` SET `srv_end_time` = '%s', `local_end_time` = '%s', `local_end_usec` = '%s', `cancel_tick_price` = '%s',
                `status` = %s WHERE  `order_id` = '%s' AND `front_id` = '%s' AND `session_id` = '%s' ''' % (self.__buildDBTime(data['insertDate'],
                data['insertTime']), self.__cTime2DBTime(t), u, data['currentTick'], self.STATUS_ALL_CANCELED,  data['orderID'], data['frontID'],
                data['sessionID'])
            self._db.update(sql)


    def __initDB(self):
        sql = '''
            CREATE TABLE IF NOT EXISTS `order` (
                `id` int(11) NOT NULL AUTO_INCREMENT,
                `appKey` varchar(50) NOT NULL DEFAULT '',
                `iid` varchar(50) NOT NULL DEFAULT '',
                `order_id` int(11) NOT NULL DEFAULT '0',
                `front_id` int(11) NOT NULL DEFAULT '0',
                `session_id` int(11) NOT NULL DEFAULT '0',
                `order_ref` int(11) NOT NULL DEFAULT '0',
                `price` decimal(10,2) NOT NULL DEFAULT '0.00',
                `total` int(11) NOT NULL DEFAULT 0,
                `real_total` int(11) NOT NULL DEFAULT 0,
                `cancel_tick_price` decimal(10,2) NOT NULL,
                `real_price` decimal(10,2) NOT NULL,
                `is_buy` int(1) NOT NULL DEFAULT '0',
                `is_open` int(11) NOT NULL DEFAULT '0',
                `srv_first_time` datetime NOT NULL COMMENT '服务器返回的insert时间',
                `srv_end_time` datetime NOT NULL COMMENT '服务器返回的最后时间',
                `local_start_time` datetime NOT NULL COMMENT '发出交易指令时间',
                `local_start_usec` int(11) NOT NULL DEFAULT '0',
                `local_first_time` datetime NOT NULL COMMENT '首次得到交易回馈时间',
                `local_first_usec` int(11) NOT NULL DEFAULT '0',
                `local_end_time` datetime NOT NULL COMMENT '首次得到交易回馈时间',
                `local_end_usec` int(11) NOT NULL DEFAULT '0',
                `status` int(11) NOT NULL DEFAULT '0',
                `mtime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
                PRIMARY KEY (`id`),
                KEY `idx_front_session_ref` (`front_id`,`session_id`,`order_ref`)
            ) ENGINE=InnoDB AUTO_INCREMENT=9458 DEFAULT CHARSET=utf8;'''
        self._db.insert(sql)
