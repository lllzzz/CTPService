#!/usr/bin/env python
# -*- encoding:utf-8 -*-
#

print 'Check module:'

def checkImport(moduleName):
    try:
        __import__(moduleName)
    except:
        print '%s ... Failed' % moduleName
    else:
        print '%s ... OK' % moduleName

modules = ['demjson', 'MySQLdb', 'redis']

for m in modules:
    checkImport(m)

