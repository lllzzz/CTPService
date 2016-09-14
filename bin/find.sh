#!/bin/bash
#
#
ps aux | grep marketSrv | grep -v grep | grep "$1"
ps aux | grep tradeSrv | grep -v grep | grep "$1"

ps aux | grep queue.py | grep ctp | grep -v grep | grep "$1"
