#!/usr/bin/env python
# -*- encoding:utf-8 -*-
import MySQLdb
from Config import Config as C

class DB():

    TYPE_TICK = 0
    TYPE_TRADE = 1

    """数据库管理类"""
    def __init__(self, type):

        env = C.get('env')

        if env == 'online':
            host = C.get('mysql_host_online')
            user = C.get('mysql_user_online')
            passwd = C.get('mysql_passwd_online')
        else:
            host = C.get('mysql_host_dev')
            user = C.get('mysql_user_dev')
            passwd = C.get('mysql_passwd_dev')


        if type == DB.TYPE_TICK:
            name = C.get('mysql_name_tick_' + env)
        else:
            name = C.get('mysql_name_trade_' + env)

        self.db = MySQLdb.connect(
            host = host,
            port = 3306,
            user = user,
            passwd = passwd,
            db = name,
            charset='utf8')
        self.cursor = self.db.cursor()

    def getAll(self, sql):
        self.cursor.execute(sql)
        res = self.cursor.fetchall()
        return len(res), res

    def getOne(self, sql):
        self.cursor.execute(sql)
        res = self.cursor.fetchone()
        flg = False if not res or len(res) == 0 else True
        return flg, res

    def insert(self, sql):
        self.cursor.execute(sql)
        self.db.commit()

    def update(self, sql):
        self.cursor.execute(sql)
        self.db.commit()

