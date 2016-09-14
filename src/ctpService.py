#!/usr/bin/env python
# -*- encoding:utf-8 -*-

import sys
sys.path.append('../src/market/')
sys.path.append('../src/trade/')
import os
import time

from Market import Market
from Trade import Trade

if len(sys.argv) == 1:
    print '''启动/停止系统：./ctpService start/stop
启动/停止队列：./ctpService startQ/stopQ
查看状态：./ctpService status'''
    sys.exit()

cmd = sys.argv[1]

market = Market()
trade = Trade()

if cmd == 'start':
    appKey = sys.argv[2]
    trade.start(appKey)
    market.start(appKey)
elif cmd == 'stop':
    market.stop()
    trade.stop()
elif cmd == 'status':
    appKey = sys.argv[2] if len(sys.argv) == 3 else ''
    os.system('./find.sh %s' % appKey)
elif cmd == 'startQ':
    os.system('./startQ.sh')
elif cmd == 'stopQ':
    os.system('./stopQ.sh')
