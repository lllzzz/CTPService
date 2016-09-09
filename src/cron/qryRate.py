#!/usr/bin/env python
# -*- encoding:utf-8 -*-

import sys
sys.path.append('../src/libs')
import warnings
warnings.filterwarnings('ignore')

import demjson as JSON

from Service import Service
from Rds import Rds
from Config import Config as C
from DB import DB
import re
import time

appKey = -518 #系统自己的appKey
sender = Rds.getSender()
sendCh = C.get('channel_trade')
db = DB(DB.TYPE_TRADE)
iids = re.split('/', C.get('iids'))


def send():
    if len(iids) == 0: 
        srv.stop()
        return
    iid = iids.pop()
    sendData = {
        'action': 'qryRate',
        'appKey': appKey,
        'iid': iid,
    }
    print sendData
    sender.publish(sendCh, JSON.encode(sendData))

def process(channel, data):
    print data
    if data['err'] > 0: return
    data = data['data']
    sql = '''
        INSERT INTO `rate` (`iid`, `open_by_money`, `open_by_vol`, `close_by_money`,
        `close_by_vol`, `close_today_by_money`, `close_today_by_vol`) VALUES ('%s',
        '%s', '%s', '%s', '%s', '%s', '%s')''' % (data['iid'], data['openByMoney'],
        data['openByVol'], data['closeByMoney'], data['closeByVol'], data['closeTodayByMoney'],
        data['closeTodayByVol'])
    db.insert(sql)
    time.sleep(1)
    send()

listenCh = C.get('channel_trade_rsp') + str(appKey)
srv = Service([listenCh], process)
send()
srv.run()
