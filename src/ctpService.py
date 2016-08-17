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
    trade.start()
    market.start()
elif cmd == 'stop':
    market.stop()
    trade.stop()
elif cmd == 'status':
    os.system('./find.sh')
elif cmd == 'startQ':
    os.system('./startQ.sh')
elif cmd == 'stopQ':
    os.system('./stopQ.sh')
