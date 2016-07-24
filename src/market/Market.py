#!/usr/bin/env python
# -*- encoding:utf-8 -*-
import os
import sys
sys.path.append('../src/libs/')
import warnings
warnings.filterwarnings('ignore')

from Config import Config as C
from DB import DB

class Market():
    """Market相关持久化方法，以及脚本调度"""
    def __init__(self):
        self._db = DB(DB.TYPE_TICK)
        pass

    def start(self):
        iids = C.get('iids').split('/')
        for iid in iids:
            self.__initDB(iid)
        os.system('./marketSrv &')
        pass

    def stop(self):
        pidPath = C.get('pid_path')
        pid = open(pidPath).read()
        os.system('kill -30 %s' % (pid))


    def save(self):
        pass

    def __initDB(self, iid):
        sql = '''
            CREATE TABLE IF NOT EXISTS `tick_%s` (
              `id` int(11) NOT NULL AUTO_INCREMENT,
              `type` varchar(50) NOT NULL DEFAULT '',
              `iid` varchar(50) NOT NULL DEFAULT '',
              `time` datetime NOT NULL COMMENT '服务端返回时间',
              `msec` int(11) NOT NULL DEFAULT '0',
              `price` decimal(10,2) NOT NULL DEFAULT '0.00',
              `volume` int(11) NOT NULL DEFAULT '0',
              `bid_price1` decimal(10,2) NOT NULL DEFAULT '0.00',
              `bid_volume1` int(11) NOT NULL DEFAULT '0',
              `ask_price1` decimal(10,2) NOT NULL DEFAULT '0.00',
              `ask_volume1` int(11) NOT NULL DEFAULT '0',
              `mtime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
              PRIMARY KEY (`id`)
            ) ENGINE=InnoDB CHARSET=utf8;''' % (iid)
        self._db.insert(sql)
