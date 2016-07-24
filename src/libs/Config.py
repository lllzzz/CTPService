#!/usr/bin/env python
# -*- encoding:utf-8 -*-

class Config():
    path = '../etc/config.ini'
    """读取ini文件"""
    def __init__(self):
        pass

    @staticmethod
    def get(key):
        configStr = open(Config.path).read()
        configArr = configStr.split('\n');
        kv = {}
        for line in configArr:
            if (not line): continue
            k, v = line.split('=')
            kv[k.strip()] = v.strip()
        return kv[key]
