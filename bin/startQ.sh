#!/bin/bash

nohup python ../src/market/queue.py ctp &
nohup python ../src/trade/queue.py ctp &
