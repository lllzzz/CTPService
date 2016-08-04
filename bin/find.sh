#!/bin/bash
#
#
ps aux | grep marketSrv | grep -v grep
ps aux | grep tradeSrv | grep -v grep

ps aux | grep queue.py | grep -v grep
