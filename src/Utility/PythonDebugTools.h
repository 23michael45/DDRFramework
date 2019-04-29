/*!
 * File: PythonDebugTools.h
 * Date: 2019/04/29 16:01
 *
 * Author: michael
 * Contact: michael2345@live.cn
 *
 * Description:Python Debug Tools
 *
*/
#ifndef PythonDebugTools_h__
#define PythonDebugTools_h__

#include <Python.h>
#include <string>

#include "src/Utility/CommonFunc.h"
#include "src/Utility/DDRMacro.h"
#include "src/Utility/Logger.h"
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
