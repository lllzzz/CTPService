#!/usr/bin/env python
# -*- encoding:utf-8 -*-

import sys
sys.path.append('../src/market/')
import os
import time

from Market import Market

if len(sys.argv) == 1:
    print '''启动/停止系统：./ctpService start/stop
启动/停止队列：./ctpService q start/stop
查看状态：./ctpService status'''
    sys.exit()

cmd = sys.argv[1]

market = Market()

if cmd == 'start':
    market.start()
elif cmd == 'stop':
    market.stop()
elif cmd == 'status':
    os.system('./find.sh')
elif cmd == 'q':
    subcmd = sys.argv[2]
    if (subcmd == 'start'):
        os.system('./startQ.sh')
    elif (subcmd == 'stop'):
        os.system('./stopQ.sh')
