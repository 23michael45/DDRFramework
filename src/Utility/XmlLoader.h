#ifndef XmlLoader_h__
#define XmlLoader_h__


#include <map>
#include <string>
#include <sstream>
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
		template <typename T>
		T GetValue(int count, std::string key)
		{
			std::istringstream  iss(GetValue(count,key));
			iss.exceptions(std::ios::failbit);
			T t;
			iss >> t;
			return t;

		}

		std::string GetValue(std::string sheet, std::string key);
		std::string GetValue(std::string key);
		template <typename T> 
		T GetValue(std::string key)
		{
			std::istringstream  iss(GetValue(key));
			iss.exceptions(std::ios::failbit);
			T t;
			iss >> t;
			return t;
		}

		std::string GetRCValue(std::string sheet, std::string rowkey, std::string colkey);
		std::string GetRCValue(std::string rowkey, std::string colkey);
		template <typename T>
		T GetRCValue(std::string key,std::string colkey)
		{
			std::istringstream  iss(GetRCValue(key, colkey));
			iss.exceptions(std::ios::failbit);
			T t;
			iss >> t;
			return t;
		}


		int GetElementCount(std::string sheet);
		int GetElementCount();
		int ColumnGetKeyCount();
		std::string GetColumnKey(int count);
		int ColumnGetKeyCount(std::string sheet);
		std::string GetColumnKey(std::string sheet, int count);


		int RowGetKeyCount();
		std::string GetRowKey(int count);
		int RowGetKeyCount(std::string sheet);
		std::string GetRowKey(std::string sheet, int count);

		int RowGetKeyIndex(std::string key)
		{
			return RowGetKeyIndex(m_DefaultSheetName, key);
		}
		int RowGetKeyIndex(std::string sheet, std::string key)
		{
			if (m_RowKeyIndexMap.find(sheet) != m_RowKeyIndexMap.end())
			{
				return m_RowKeyIndexMap[sheet]->at(key);
			}
			return -1;
		}


		void SetValue(std::string sheet, int count, std::string key, std::string value);
		void SetValue(int count, std::string key, std::string value);
		void SetValue(std::string key, std::string value);

		template <typename T>
		void SetValue(std::string key, T& value)
		{
			SetValue(key, std::string::to_string(value));
		}

		void DoSave(std::string filename); 
		void DoSave();


		typedef std::map<std::string, std::string> KVMap;
		typedef std::vector<std::shared_ptr<KVMap>> KVMapVector;
		typedef std::map<std::string, std::shared_ptr<KVMapVector>> SheetMap;
		typedef std::vector<std::string> KeyVector;

		typedef std::map<std::string, int> KeyIndexMap;
		typedef std::map<int, std::string> IndexKeyMap;
	protected:
		std::string m_DefaultSheetName;

		std::map<std::string, std::shared_ptr<KeyVector>> m_ColumnKeyMap;


		std::map<std::string, std::shared_ptr<KeyIndexMap>> m_RowKeyIndexMap;
		std::map<std::string, std::shared_ptr<IndexKeyMap>> m_RowIndexKeyMap;
		

		SheetMap m_SheetMap;
		std::string m_FileName;
	private:

	};
}
#endif // XmlLoader_h__