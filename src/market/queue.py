#!/usr/bin/env python
# -*- encoding:utf-8 -*-

import sys
sys.path.append('../src/market/')
sys.path.append('../src/libs/')

from Market import Market
from Q import Q

market = Market()
queue = Q('Q_TICK', market)
queue.run()
