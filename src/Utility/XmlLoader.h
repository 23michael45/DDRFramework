#pragma once
#include <map>
#include <string>
#include <vector>

namespace DDRFramework
{
	class  XmlLoader
	{
	public:
		XmlLoader(std::string fileName);
		~XmlLoader();

		std::string GetValue(std::string sheet, int count, std::string key);
		std::string GetValue(int count, std::string key);
		std::string GetValue(std::string key);


		int GetElementCount(std::string sheet);
		int GetElementCount();
		int GetKeyCount();
		std::string GetKey(int count);
		int GetKeyCount(std::string sheet);
		std::string GetKey(std::string sheet, int count);


		void SetValue(std::string sheet, int count, std::string key, std::string value);
		void SetValue(int count, std::string key, std::string value);
		void SetValue(std::string key, std::string value);
		void DoSave(std::string filename); 
		void DoSave();


		typedef std::map<std::string, std::string> KVMap;
		typedef std::vector<std::shared_ptr<KVMap>> KVMapVector;
		typedef std::map<std::string, std::shared_ptr<KVMapVector>> SheetMap;
		typedef std::vector<std::string> KeyVector;


	protected:
		std::string m_DefaultSheetName;

		std::map<std::string, std::shared_ptr<KeyVector>> m_KeyMap;
		SheetMap m_SheetMap;
		std::string m_FileName;
	private:

	};
}