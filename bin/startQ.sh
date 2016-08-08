#!/bin/bash

nohup python ../src/market/queue.py &
nohup python ../src/trade/queue.py &
