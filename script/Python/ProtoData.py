#coding=utf-8
import BaseCmd_pb2 as PB

#default msg,donot delete it 
def msg():
    reqCmdMove = PB.reqCmdMove()
    reqCmdMove.line_speed = 234
    reqCmdMove.angulau_speed = 567
    return reqCmdMove


def reqMove():
    reqCmdMove = PB.reqCmdMove()
    reqCmdMove.line_speed = 111
    reqCmdMove.angulau_speed = 222
    return reqCmdMove

def rspMove():
    rspCmdMove = PB.rspCmdMove()
    rspCmdMove.type = PB.eSuccess
    return rspCmdMove

def reqAudioTalk():
    reqAudioTalk = PB.reqAudioTalk()
    #reqAudioTalk.optype = PB.eStart
    return reqAudioTalk

def notifyAlarmInfo():
    notifyAlarm = PB.notifyAlarmInfo()
    notifyAlarm.type = PB.notifyAlarmInfo.eThermalAlarm
    notifyAlarm.alarmcontent = "Python Alarm"
    return notifyAlarm