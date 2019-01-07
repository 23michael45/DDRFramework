Python 调试说明 

-----------------------------------------------------------------------------------------------------------

调试客户端：
	1 打开服务器  DDR_LocalServer.exe
	2 Copy   Share/script/Python 目录到某一路径  并配置Global配置表中PythonPath路径到此路径
	3 打开Python路径下ProtoData.py   编写客户端期望收到的 python 函数 例如 
		def rspMove():
			rspCmdMove = PB.reqCmdMove()
			rspCmdMove.type = eSuccess
			return rspCmdMove;
	  或改写
		def msg()
			...
			
	4 服务器程序命令行里，输入
		py:rspMove:192.168.1.183      #向183的客户端发送reqMove
		py:rspMove					  #向所有正在连接的客户端发送rspMove
		py							  #向所有正在连接的客户端发送msg函数里返回的msg
		
	

-----------------------------------------------------------------------------------------------------------



调试LSM服务:
	1 打开服务器  DDR_LocalServer.exe 及客户端模拟程序 DDR_LocalClient
	2 Copy   Share/script/Python 目录到某一路径  并配置Global配置表中PythonPath路径到此路径
	3 打开Python路径下ProtoData.py   编写LSM期望收到的 python 函数 例如 
		def rspMove():
			rspCmdMove = PB.reqCmdMove()
			rspCmdMove.type = eSuccess
			return rspCmdMove;
	  或改写
		def msg()
			...
			
	4 客户端程序命令行里，输入
		py:reqMove      			  #向服务端发送reqMove
		py							  #向服务端发送msg函数里返回的msg
		