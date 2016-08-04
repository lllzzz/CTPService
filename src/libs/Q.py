#!/usr/bin/env python
# -*- encoding:utf-8 -*-

import os
import sys
sys.path.append('../src/libs/')
from Config import Config as C
import demjson as JSON
import re
import time

class Q():
    """消化队列"""
    def __init__(self, key, obj):
        self.key = key
        self.obj = obj
        # self.qPath = '../../logs/'
        self.qPath = C.get('q_path')

    def __getFile(self):

        fNames = os.listdir(self.qPath)
        pFiles = []
        for file in fNames:
            if re.search(self.key + '_.*\.push', file):
                pFiles.append(file)

        if len(pFiles) == 0: return False
        pFiles.sort()
        return self.qPath + pFiles.pop(0)

    def __getData(self):

        fName = self.__getFile()
        if not fName: return []
        fp = open(fName, 'r')
        data = fp.readlines()
        fp.close()
        os.remove(fName)
        return data


    def run(self):
        while True:
            datas = self.__getData()
            if len(datas):
                for line in datas:
                    data = JSON.decode(line)
                    self.obj.processQ(data)
                pass
            else:
                time.sleep(1)
