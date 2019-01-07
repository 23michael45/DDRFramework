import PythonDebugger
import ProtoData

def main():

    msg = ProtoData.msg()
    ret = PythonDebugger.Serialize(msg)
    print(ret)
    
    
    msg = ProtoData.reqMove()
    ret = PythonDebugger.Serialize(msg)
    print(ret)

    
    msg = ProtoData.rspMove()
    ret = PythonDebugger.Serialize(msg)
    print(ret)

    
    msg = ProtoData.notifyAlarmInfo()
    ret = PythonDebugger.Serialize(msg)
    print(ret)
if __name__ == "__main__":
    main()