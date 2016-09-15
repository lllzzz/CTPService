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

    LOG_TYPE_ACT_TRADE = 1
    LOG_TYPE_RSP_ORDER = 2
    LOG_TYPE_RSP_TRADED = 3
    LOG_TYPE_RSP_CANCELED = 4

    def __init__(self):
        self._db = DB(DB.TYPE_TRADE)
        pass

    def start(self, appKey):
        self.__initDB()
        os.popen('./tradeSrv %s &' % (appKey))
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
                INSERT INTO `order_log` (`appKey`, `iid`, `order_id`, `front_id`, `session_id`, `order_ref`,
                `price`, `total`, `is_buy`, `is_open`, `srv_time`, `local_time`, `local_usec`, `type`) VALUES ('%s',
                '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')''' % (data['appKey'],
                data['iid'], data['orderID'], data['frontID'], data['sessionID'], data['orderRef'],
                data['price'], data['total'], data['isBuy'], data['isOpen'], '0', self.__cTime2DBTime(t), u, self.LOG_TYPE_ACT_TRADE);
            self._db.insert(sql)

        elif type == 'traded':
            t, u = data['localTime'].split('.')
            sql = '''
                INSERT INTO `order_log` (`appKey`, `iid`, `order_id`, `front_id`, `session_id`, `order_ref`,
                `price`, `total`, `is_buy`, `is_open`, `srv_time`, `local_time`, `local_usec`, `type`) VALUES ('%s',
                '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')''' % (data['appKey'],
                data['iid'], data['orderID'], data['frontID'], data['sessionID'], data['orderRef'],
                data['realPrice'], data['successVol'], '-1', '-1', self.__buildDBTime(data['tradeDate'],
                data['tradeTime']), self.__cTime2DBTime(t), u, self.LOG_TYPE_RSP_TRADED);
            self._db.update(sql)

        elif type == 'order':
            t, u = data['localTime'].split('.')
            sql = '''
                INSERT INTO `order_log` (`appKey`, `iid`, `order_id`, `front_id`, `session_id`, `order_ref`,
                `price`, `total`, `is_buy`, `is_open`, `srv_time`, `local_time`, `local_usec`, `type`) VALUES ('%s',
                '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')''' % (data['appKey'],
                data['iid'], data['orderID'], data['frontID'], data['sessionID'], data['orderRef'],
                '-1', data['todoVol'], '-1', '-1', self.__buildDBTime(data['insertDate'], data['insertTime']),
                self.__cTime2DBTime(t), u, self.LOG_TYPE_RSP_ORDER);

            self._db.update(sql)

        elif type == 'canceled':
            t, u = data['localTime'].split('.')
            sql = '''
                INSERT INTO `order_log` (`appKey`, `iid`, `order_id`, `front_id`, `session_id`, `order_ref`,
                `price`, `total`, `is_buy`, `is_open`, `srv_time`, `local_time`, `local_usec`, `type`) VALUES ('%s',
                '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')''' % (data['appKey'],
                data['iid'], data['orderID'], data['frontID'], data['sessionID'], data['orderRef'],
                data['currentTick'], data['cancelVol'], '-1', '-1', self.__buildDBTime(data['insertDate'], data['insertTime']),
                self.__cTime2DBTime(t), u, self.LOG_TYPE_RSP_CANCELED);

            self._db.update(sql)


    def __initDB(self):

        sql = '''
            CREATE TABLE IF NOT EXISTS `order_log` (
                `id` int(11) NOT NULL AUTO_INCREMENT,
                `appKey` varchar(50) NOT NULL DEFAULT '',
                `iid` varchar(50) NOT NULL DEFAULT '',
                `order_id` int(11) NOT NULL DEFAULT '0',
                `front_id` int(11) NOT NULL DEFAULT '0',
                `session_id` int(11) NOT NULL DEFAULT '0',
                `order_ref` int(11) NOT NULL DEFAULT '0',
                `price` decimal(10,2) NOT NULL DEFAULT '0.00',
                `total` int(11) NOT NULL DEFAULT 0,
                `is_buy` int(1) NOT NULL DEFAULT '-1',
                `is_open` int(11) NOT NULL DEFAULT '-1',
                `srv_time` datetime NOT NULL COMMENT '服务器返回时间',
                `local_time` datetime NOT NULL COMMENT '发出交易指令时间',
                `local_usec` int(11) NOT NULL DEFAULT '0',
                `type` int(11) NOT NULL DEFAULT '0',
                `mtime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
                PRIMARY KEY (`id`),
                KEY `idx_front_session_ref` (`front_id`,`session_id`,`order_ref`)
            ) ENGINE=InnoDB CHARSET=utf8;'''
        self._db.insert(sql)

        sql = '''
            CREATE TABLE IF NOT EXISTS `rate` (
                `id` int(11) NOT NULL AUTO_INCREMENT,
                `iid` varchar(50) NOT NULL DEFAULT '',
                `open_by_money` decimal(10,2) NOT NULL DEFAULT '0.00',
                `open_by_vol` decimal(10,2) NOT NULL DEFAULT '0.00',
                `close_by_money` decimal(10,2) NOT NULL DEFAULT '0.00',
                `close_by_vol` decimal(10,2) NOT NULL DEFAULT '0.00',
                `close_today_by_money` decimal(10,2) NOT NULL DEFAULT '0.00',
                `close_today_by_vol` decimal(10,2) NOT NULL DEFAULT '0.00',
                `mtime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
                PRIMARY KEY (`id`)
            ) ENGINE=InnoDB CHARSET=utf8;'''
        self._db.insert(sql)
