#include "PythonDebugTools.h"
#include <string>

#include <google/protobuf/message.h>
#include "../../Shared/src/Utility/CommonFunc.h"
#include "../../Shared/src/Utility/DDRMacro.h"
#include "../../Shared/src/Utility/Logger.h"


#include "../../Shared/thirdparty/cppfs/include/cppfs/FilePath.h"

namespace DDRFramework
{
	PythonDebugTools::PythonDebugTools(std::string path)
	{
		if (path.empty())
		{

			std::string exedir = DDRFramework::getexepath();
			cppfs::FilePath fpath(exedir);
			exedir = fpath.directoryPath() + "/Python";
			m_BasePath = exedir;
		}
		else
		{

			m_BasePath = path;
		}
	}

	PythonDebugTools::~PythonDebugTools()
	{
	}

	PyObject* PythonDebugTools::RunModuleFunc(std::string moduleName, std::string funcName, PyObject* params)
	{
		PyObject* pModuleName = PyUnicode_FromString(moduleName.c_str());
		PyObject* pModule = PyImport_Import(pModuleName);
		if (!pModule)
		{
			DebugLog("[ERROR] Python get module failed.");
			return nullptr;
		}
		PyObject* pyFunc = PyObject_GetAttrString(pModule, funcName.c_str());
		if (!pyFunc || !PyCallable_Check(pyFunc))  // 验证是否加载成功
		{
			DebugLog("[ERROR] Can't find funftion (%s)", funcName.c_str());
			return nullptr;
		}

		PyObject* pRetValue = PyObject_CallObject(pyFunc, params);
		return pRetValue;
	}

	void PythonDebugTools::RunPython(std::string& funcName, char** buffer, Py_ssize_t& len, std::string& type_name)
	{
		Py_Initialize();
		PyRun_SimpleString("import sys");

		//std::string setbasePath = GlobalManager::Instance()->GetGlobalConfig().GetValue("PythonPath");
		std::string setbasePath = "sys.path.append('" + m_BasePath + "')";

		DebugLog("Set Python Base Path %s",m_BasePath.c_str());

		PyRun_SimpleString(setbasePath.c_str());



		std::string exedir = DDRFramework::getexepath();
		cppfs::FilePath fpath(exedir);
		exedir = fpath.directoryPath();
		std::wstring wexedir = DDRFramework::StringToWString(exedir);
		Py_SetPythonHome((wchar_t*)wexedir.c_str());

		PyObject* pMsg = RunModuleFunc("ProtoData", funcName, nullptr);
		if (pMsg)
		{
			auto args = PyTuple_New(1);
			PyTuple_SetItem(args, 0, pMsg);
			PyObject* pRetValue = RunModuleFunc("PythonDebugger", "Serialize", args);

			if (pRetValue)
			{

				PyObject * pObjDataString = nullptr;
				const char* pObjTypeName = nullptr;

				PyArg_ParseTuple(pRetValue, "O|s", &pObjDataString, &pObjTypeName);

				Py_ssize_t size = PyBytes_AsStringAndSize(pObjDataString, buffer, &len);
				type_name = std::string(pObjTypeName);
				DebugLog("result: %i", pRetValue);
			}
			else
			{
				DebugLog("ProtoData Serialize Error");

			}

		}
		else
		{
			DebugLog("ProtoData Get Error");
		}

		Py_Finalize();
	}
	std::shared_ptr<google::protobuf::Message> PythonDebugTools::Run(std::string funcname)
	{
		char* pdata = nullptr;
		Py_ssize_t size;
		std::string type_name;
		RunPython(funcname, &pdata, size, type_name);

		if (!type_name.empty())
		{
			auto spMsg = std::shared_ptr<google::protobuf::Message>(NULL);
			const Descriptor* descriptor =
				DescriptorPool::generated_pool()->FindMessageTypeByName(type_name);
			if (descriptor) {
				const google::protobuf::Message* prototype =
					MessageFactory::generated_factory()->GetPrototype(descriptor);
				if (prototype) {
					spMsg = std::shared_ptr<google::protobuf::Message>(prototype->New());
				}
			}
			spMsg->ParseFromArray(pdata, size);
			return spMsg;

		}
		return nullptr;

	}

	void PythonDebugTools::RunPythonLegency(char** buffer, Py_ssize_t& len, std::string& type_name)
	{
		Py_Initialize();
		PyRun_SimpleString("print('Hello Python Debugger!'\n)");
		PyRun_SimpleString("import sys");
		//PyRun_SimpleString("sys.path.append('D:/DevelopProj/Dadao/DDRFramework/DDRLocalServer/x64/Debug/Python/')");



		PyRun_SimpleString("sys.path.append('D:/DevelopProj/Dadao/DDRFramework/DDRLocalServer/DDR_LocalClient/Python/')");
		PyRun_SimpleString("import ClientPythonDebugger as db");
		//PyRun_SimpleString("db.Move()");

		//std::string path = "D:/DevelopProj/Dadao/DDRFramework/DDRLocalServer/DDR_LocalClient/Python";
		//std::wstring path = L"D:/DevelopProj/Dadao/DDRFramework/DDRLocalServer/x64/Debug/Python/";
		//PySys_SetPath(path.c_str());


		std::string moduleName = "ClientPythonDebugger";
		PyObject* pModuleName = PyUnicode_FromString(moduleName.c_str());

		PyObject* pModule = PyImport_Import(pModuleName);
		//PyObject* pModule = PyImport_ImportModule(moduleName.c_str());
		if (!pModule) // 加载模块失败
		{
			DebugLog("[ERROR] Python get module failed.");
			Py_Finalize();
			return;
		}

		PyObject* pyFunc = PyObject_GetAttrString(pModule, "Serialize");
		if (!pyFunc || !PyCallable_Check(pyFunc))  // 验证是否加载成功
		{
			DebugLog("[ERROR] Can't find funftion (Move)");
			return;
		}
		/*
			PyObject * pyParams = PyTuple_New(2);
			PyTuple_SetItem(pyParams, 0, Py_BuildValue("s"));
			PyTuple_SetItem(pyParams, 1, Py_BuildValue("s"));
		*/
		PyObject* pRetValue = PyObject_CallObject(pyFunc, nullptr);



		// 获取参数
		if (pRetValue)  // 验证是否调用成功
		{
			//int size = Pytu(pRetValue);

			/*const char* pdatastring;
			const char* ptypename;*/


			PyObject * pObjDataString = nullptr;
			const char* pObjTypeName = nullptr;

			PyArg_ParseTuple(pRetValue, "O|s", &pObjDataString, &pObjTypeName);

			Py_ssize_t size = PyBytes_AsStringAndSize(pObjDataString, buffer, &len);
			type_name = std::string(pObjTypeName);
			//Py_UNICODE* pdata = PyUnicode_AS_UNICODE(pObjDataString);
			//Py_UNICODE* pdata = 
			//string sdatastring();
			//string stypename(PyUnicode_AsUTF8(pObjTypeName));

			//PyObject* pyDataString = PyList_GetItem(pRet, 0);
			//PyObject* pyType= PyList_GetItem(pRet, 1);

			//datastring = PyBytes_AS_STRING(pyDataString);
			//type_name = PyBytes_AS_STRING(pyType);
			//datastring = pdatastring;
			//type_name = ptypename;
			DebugLog("result: %i", pRetValue);
		}

		Py_Finalize();
	}


}