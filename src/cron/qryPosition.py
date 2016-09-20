#!/usr/bin/env python
# -*- encoding:utf-8 -*-

import sys
sys.path.append('../src/libs')
import warnings
warnings.filterwarnings('ignore')
import threading
import demjson as JSON

from Service import Service
from Rds import Rds
from Config import Config as C
from DB import DB
import re
import time

appKey = -519 #系统自己的appKey
sender = Rds.getSender()
rds = Rds.getRds()
sendCh = C.get('channel_trade')
db = DB(DB.TYPE_TRADE)
iids = re.split('/', C.get('iids'))
num = len(iids)

pos = {}
cnt = 0

isSending = False
def checkSend():
    if isSending == True:
        srv.stop()

def send():
    iid = iids.pop(0)
    global cnt, num, isSending
    cnt += 1
    if cnt > num:
        cnt = 1
        time.sleep(60)

    sendData = {
        'action': 'qryPosition',
        'appKey': appKey,
        'iid': iid,
    }
    pos[iid] = {'buy': 0, 'sell': 0}
    iids.append(iid)
    sender.publish(sendCh, JSON.encode(sendData))
    isSending = True
    timer = threading.Timer(5, checkSend)
    timer.start()

def process(channel, data):
    global isSending
    isSending = False
    if data['err'] > 0: return
    if data['err'] == -1: # 网络不通
        srv.stop()
        return

    data = data['data']
    if data['direction'] == 'buy':
        pos[data['iid']]['buy'] += data['pos']
    elif data['direction'] == 'sell':
        pos[data['iid']]['sell'] += data['pos']

    if data['isLast']:
        print pos
        key = 'POSITION_%s' % (data['iid'])
        rds.set(key, JSON.encode(pos[data['iid']]))
        time.sleep(1)
        send()

listenCh = C.get('channel_trade_rsp') + str(appKey)
srv = Service([listenCh], process)
send()
srv.run()
