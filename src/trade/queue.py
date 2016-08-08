#!/usr/bin/env python
# -*- encoding:utf-8 -*-

import sys
sys.path.append('../src/trade/')
sys.path.append('../src/libs/')

from Trade import Trade
from Q import Q

trade = Trade()
queue = Q('Q_TRADE', trade)
queue.run()
