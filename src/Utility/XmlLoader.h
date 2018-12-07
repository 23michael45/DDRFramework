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
		int GetElementCount(std::string sheet);
		int GetElementCount();
		int GetKeyCount();
		std::string GetKey(int count);
		int GetKeyCount(std::string sheet);
		std::string GetKey(std::string sheeet, int count);



		typedef std::map<std::string, std::string> KVMap;
		typedef std::vector<std::shared_ptr<KVMap>> KVMapVector;
		typedef std::map<std::string, std::shared_ptr<KVMapVector>> SheetMap;
		typedef std::vector<std::string> KeyVector;


	protected:
		std::string mDefaultSheetName;

		std::map<std::string, std::shared_ptr<KeyVector>> mKeyMap;
		SheetMap mSheetMap;
	private:

	};
}