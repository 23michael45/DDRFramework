#ifndef PythonDebugTools_h__
#define PythonDebugTools_h__

#include <Python.h>
#include <string>

#include "../../Shared/src/Utility/CommonFunc.h"
#include "../../Shared/src/Utility/DDRMacro.h"
#include "../../Shared/src/Utility/Logger.h"
using namespace google::protobuf;

namespace DDRFramework
{
	class PythonDebugTools
	{
	public:
		PythonDebugTools(std::string path);
		~PythonDebugTools();


		std::shared_ptr<google::protobuf::Message> Run(std::string funcname);

		PyObject* RunModuleFunc(std::string moduleName, std::string funcName, PyObject* params);

		void RunPython(std::string& funcName, char** buffer, Py_ssize_t& len, std::string& type_name);
		void RunPythonLegency(char** buffer, Py_ssize_t& len, std::string& type_name);


	private:
		std::string m_BasePath;
	};

}
#endif // PythonDebugTools_h__
