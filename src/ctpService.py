#!/usr/bin/env python
# -*- encoding:utf-8 -*-

import sys
sys.path.append('../src/market/')

from Market import Market

cmd = sys.argv[1]

market = Market()

if (cmd == 'start'):
    market.start()
elif (cmd == 'stop'):
    market.stop()