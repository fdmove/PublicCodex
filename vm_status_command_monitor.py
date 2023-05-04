#!/usr/bin/python3
# -*- coding: utf-8 -*-
# v1.0 2023-4-28
# by Dasmz
# 针对VM虚拟机卡死，进行三次判定，如果判定已断，则强制重启对应的VM
# NAME: /root/sh/vm_status_command_monitor.py

import subprocess
import socket
import time
import datetime


# 使用字典，定义静态的一些VM虚机的信息，
# VM的IP地址
# VM的SSH端口
# VM登录方式SSH，SSH登录超时时间5秒
# PVE内的VMID参数
vm100 = {
   "vIP" : '10.25.50.1',
   "vTryPort" : 22,
   "vtimeout" : 5,
   "vVMID": 100,
}

vm101 = {
   "vIP" : '10.25.50.60',
   "vTryPort" : 22,
   "vtimeout" : 5,
   "vVMID": 101,
}

vm103 = {
   "vIP" : '10.25.50.240',
   "vTryPort" : 22,
   "vtimeout" : 5,
   "vVMID": 103,
}

vm105 = {
   "vIP" : '10.25.50.85',
   "vTryPort" : 22,
   "vtimeout" : 5,
   "vVMID": 105,
}

vm106 = {
   "vIP" : '10.25.50.86',
   "vTryPort" : 22,
   "vtimeout" : 5,
   "vVMID": 106,
}

vm107 = {
   "vIP" : '10.25.50.87',
   "vTryPort" : 22,
   "vtimeout" : 5,
   "vVMID": 107,
}

vm108 = {
   "vIP" : '10.25.50.88',
   "vTryPort" : 22,
   "vtimeout" : 5,
   "vVMID": 108,
}

vm179 = {
   "vIP" : '10.25.50.179',
   "vTryPort" : 22,
   "vtimeout" : 5,
   "vVMID": 179,
}


#   function-35f9dCf6DdffABfD 返回时间字符串
def getNow():
    return datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")




#   function-34FcFCB5f593208d
#   功能 使用subprocess执行一个shell脚本
#   参数是一个列表 vCMDList
#                  ['ls', '-lrt']
#                  ['qm', 'stop', '106']
#                  ['qm', 'start', '106']
#  ["/usr/bin/ssh", "-i", "/root/.ssh/key_id_rsa_2048.key",  "root@10.25.50.88", "'echo OK'"]
def getShellResult(vCMDList):
    try:
        # 执行ls命令并获取输出结果
        result = subprocess.run(vCMDList, stdout=subprocess.PIPE)
        # 打印输出结果
        if result.stdout.decode('utf-8').strip() == 'OK':
            return 'Y'
        else:
            return 'N'
    except Exception as e:
        print(e)
        print("ERROR: function-34FcFCB5f593208d")
        return "N"


#   function-eff95f0AfB2b3AbF
#   离线连续判定 3次，则原则上判定主机离线
def gogogo(vVMDict):
    try:
        cnt = 0
        # 构建探测命令的列表
        vRunCMD =["/usr/bin/ssh", "-i", "/root/.ssh/key_id_rsa_2048.key"]  
        vRunCMD.append("root@%s" % vVMDict.get("vIP")) 
        vRunCMD.append('/usr/bin/echo OK')
        print(vRunCMD)
        # 构建启动停止命令
        vRunStopList  = ['/usr/sbin/qm', 'stop']
        vRunStartList = ['/usr/sbin/qm', 'start']
        vRunStopList.append(str(vVMDict.get("vVMID")))
        vRunStartList.append(str(vVMDict.get("vVMID")))
        print(vVMDict)
        print(vRunStopList,vRunStartList)
        # 执行三次判定
        for i in range(3):
            print("\n + 第%d次判定执行过程 开始 %s" % (i+1, getNow()))
            vOutput = getShellResult(vCMDList = vRunCMD)
            if vOutput == 'N':
                print(" + 可能存在主机离线 第%d次 %s" % (cnt+1, getNow()))
                cnt = cnt + 1
            time.sleep(20)
        print("\n + 统计 离线次数: %d %s" % (cnt,getNow()))
        if cnt == 3:
            print(" + 执行 关闭 VM的操作 %s" % getNow())
            vRunstopOutput  = getShellResult(vCMDList = vRunStopList)
            time.sleep(36)
            print(" + 执行 启动 VM的操作 %s\n\n\n" % getNow())
            vRunstartOutput = getShellResult(vCMDList = vRunStartList)
        else:
            print(" + 主机正常 无需其他操作 %s\n\n\n" % getNow())
    except Exception as e:
        print(e)
        print("ERROR: function-eff95f0AfB2b3AbF")


if __name__ == '__main__':
    # vmList = [vm100,vm101,vm103,vm105,vm106,vm107,vm108] 
    vmList = [vm101,vm103,vm105,vm106,vm107,vm108] 
    for v in vmList:
       gogogo(vVMDict = v)

