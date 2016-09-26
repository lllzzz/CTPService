#!/bin/bash

nohup python ../src/market/queue.py ctp > /tmp/mq.log &
nohup python ../src/trade/queue.py ctp > /tmp/tq.log &
