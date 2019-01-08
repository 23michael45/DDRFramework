#coding=utf-8
import BaseCmd_pb2 as PB
import ProtoData as PD

def Serialize(msg):
    print('Python Serialize Called')
    if msg == None:
        return (b'','Serialize error')
    else:
        dataarray = msg.SerializeToString()
        return (dataarray,msg.DESCRIPTOR.full_name)
