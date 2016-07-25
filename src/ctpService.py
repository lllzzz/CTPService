#!/usr/bin/env python
# -*- encoding:utf-8 -*-

import sys
sys.path.append('../src/market/')
import os
import time

from Market import Market

cmd = sys.argv[1]

market = Market()

if (cmd == 'start'):
    market.start()
elif (cmd == 'stop'):
    market.stop()
elif (cmd == 'queue'):
    subcmd = sys.argv[2]
    if (subcmd == 'start'):
        os.system('./startQ.sh')
    elif (subcmd == 'stop'):
        os.system('./stopQ.sh')
