#include "XmlLoader.h"
#include "tinyxml.h"

namespace DDRFramework
{
	XmlLoader::XmlLoader(std::string fileName) :m_FileName(fileName)
	{
		TiXmlDocument doc(m_FileName.c_str());
		bool loadOkay = doc.LoadFile();

		if (!loadOkay)
		{
			printf("Could not load test file %s. Error='%s'. Exiting.\n", fileName.c_str(), doc.ErrorDesc());
			exit(1);
		}

		TiXmlNode* rootNode = 0;
		TiXmlNode* sheetNode = 0;
		TiXmlElement* rootElement = 0;
		TiXmlElement* sheetElement = 0;
		TiXmlElement* itemElement = 0;

		rootNode = doc.FirstChild("Root");
		rootElement = rootNode->ToElement();

		bool firstSheet = true;
		for (sheetNode = rootElement->FirstChild(); sheetNode != 0; sheetNode = sheetNode->NextSibling())
		{
			sheetElement = sheetNode->ToElement();


			std::shared_ptr<KVMapVector> spVector = std::make_shared<KVMapVector>();
			m_SheetMap[sheetElement->Value()] = spVector;
			if (firstSheet)
			{
				m_DefaultSheetName = sheetElement->Value();
				firstSheet = false;
			}

			std::shared_ptr<KeyVector> spKeyVector = std::make_shared<KeyVector>();
			m_KeyMap[sheetElement->Value()] = spKeyVector;

			TiXmlElement* itemElement = 0;
			TiXmlNode* itemNode = 0;

			bool firstElement = true;
			for (itemNode = sheetElement->FirstChild(); itemNode != 0; itemNode = itemNode->NextSibling())
			{
				itemElement = itemNode->ToElement();


				std::shared_ptr<KVMap> spMap = std::make_shared<KVMap>();
				TiXmlAttribute* attr;
				for (attr = itemElement->FirstAttribute(); attr != 0; attr = attr->Next())
				{
					spMap->insert(std::pair<std::string, std::string>(attr->Name(), attr->Value()));

					if (firstElement)
					{
						spKeyVector->push_back(attr->Name());
					}

				}
				firstElement = false;


				spVector->push_back(spMap);
			}

		}
	}

	XmlLoader::~XmlLoader()
	{
	}

	std::string XmlLoader::GetValue(std::string sheet, int count, std::string key)
	{
		std::string s = m_SheetMap[sheet]->at(count)->at(key);
		return s;

	}
	std::string XmlLoader::GetValue(int count, std::string key)
	{

		std::string s = m_SheetMap[m_DefaultSheetName]->at(count)->at(key);
		return s;
	}
	std::string XmlLoader::GetValue(std::string key)
	{
		std::string s = m_SheetMap[m_DefaultSheetName]->at(0)->at(key);
		return s;

	}
	int XmlLoader::GetElementCount(std::string sheet)
	{
		return  m_SheetMap[sheet]->size();
	}
	int XmlLoader::GetElementCount()
	{
		return  GetElementCount(m_DefaultSheetName);
	}

	std::string  XmlLoader::GetKey(int count)
	{
		return GetKey(m_DefaultSheetName,count);

	}
	std::string XmlLoader::GetKey(std::string sheet, int count)
	{
		return m_KeyMap[sheet]->at(count);
	}

	int XmlLoader::GetKeyCount()
	{
		return GetKeyCount(m_DefaultSheetName);
	}
	int XmlLoader::GetKeyCount(std::string sheet)
	{
		return m_KeyMap[sheet]->size();
	}



	void XmlLoader::SetValue(std::string sheet, int count, std::string key, std::string value)
	{
		m_SheetMap[sheet]->at(count)->at(key) = value;
	}
	void XmlLoader::SetValue(int count, std::string key, std::string value)
	{
		SetValue(m_DefaultSheetName, count, key, value);

	}
	void XmlLoader::SetValue(std::string key, std::string value)
	{
		SetValue(0, key, value);

	}
	void XmlLoader::DoSave()
	{
		if (!m_FileName.empty())
		{
			DoSave(m_FileName);

		}
	}
	void XmlLoader::DoSave(std::string filename)
	{
		TiXmlDocument doc;

		TiXmlNode* rootNode = new TiXmlElement("Root");
		TiXmlNode* sheetNode = 0;
		TiXmlElement* rootElement = 0;
		TiXmlElement* sheetElement = 0;
		TiXmlElement* itemElement = 0;

		rootElement = rootNode->ToElement();

		bool firstSheet = true;
		for (auto sheetMapPair : m_SheetMap)
		{
			sheetNode = new TiXmlElement(sheetMapPair.first.c_str());

			auto elementVec = sheetMapPair.second;
			for (auto elementVecIter : *elementVec)
			{
				itemElement = new TiXmlElement("Element");

				for (auto attrMapPair : *elementVecIter)
				{
					std::string key = attrMapPair.first;
					std::string value = attrMapPair.second;
					itemElement->SetAttribute(key.c_str(), value.c_str());
				}

				sheetNode->LinkEndChild(itemElement);
			}
			rootElement->LinkEndChild(sheetNode);
		}
		doc.LinkEndChild(rootNode);
		doc.SaveFile(filename.c_str());
	}
}