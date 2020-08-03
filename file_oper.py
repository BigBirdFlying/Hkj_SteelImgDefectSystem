import os
import sys
import time
import logging
import datetime
import configparser
from time import strftime, gmtime
from xml.etree import ElementTree as ET


class Log:
    def __init__(self, log_dir_name):
        self.log_dir_name = log_dir_name
        if False == os.path.exists(self.log_dir_name):
            os.mkdir(self.log_dir_name)
        # self.log_file_name=((self.log_dir_name+"\%s.txt") % time.strftime("%Y%m%d"))

    def AddLog(self, info):
        self.log_file_name = ((self.log_dir_name + "\%s.txt") % time.strftime("%Y%m%d"))
        curtime = time.strftime("%H:%M:%S")
        loginfo = ("%s: %s" % (curtime, info))
        log = open(self.log_file_name, 'a')
        print(loginfo, file=log)


class Xml:
    def __init__(self, file_name):
        self.file_name = file_name
        self.per = ET.parse(self.file_name)

    def read_info(self, key_info):  # key: ef:'./缺陷类别/类别1',只能接收1层
        p = self.per.findall(key_info)
        for one_per in p:
            if len(one_per) == 0:
                return one_per.text
            else:
                key_value = {}
                for child in one_per.getchildren():
                    key_value[child.tag] = child.text
                return key_value
        return None
