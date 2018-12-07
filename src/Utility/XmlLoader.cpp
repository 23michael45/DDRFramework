#include "XmlLoader.h"
#include "tinyxml.h"

namespace DDRFramework
{
	XmlLoader::XmlLoader(std::string fileName)
	{
		TiXmlDocument doc(fileName.c_str());
		bool loadOkay = doc.LoadFile();

		if (!loadOkay)
		{
			printf("Could not load test file %s. Error='%s'. Exiting.\n", fileName.c_str(), doc.ErrorDesc());
			exit(1);
		}

		TiXmlNode* rootNode = 0;
		TiXmlNode* sheeetNode = 0;
		TiXmlElement* rootElement = 0;
		TiXmlElement* sheetElement = 0;
		TiXmlElement* itemElement = 0;

		rootNode = doc.FirstChild("Root");
		rootElement = rootNode->ToElement();

		bool firstSheet = true;
		for (sheeetNode = rootElement->FirstChild(); sheeetNode != 0; sheeetNode = sheeetNode->NextSibling())
		{
			sheetElement = sheeetNode->ToElement();


			std::shared_ptr<KVMapVector> spVector = std::make_shared<KVMapVector>();
			mSheetMap[sheetElement->Value()] = spVector;
			if (firstSheet)
			{
				mDefaultSheetName = sheetElement->Value();
				firstSheet = false;
			}

			std::shared_ptr<KeyVector> spKeyVector = std::make_shared<KeyVector>();
			mKeyMap[sheetElement->Value()] = spKeyVector;

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
		std::string s = mSheetMap[sheet]->at(count)->at(key);
		return s;

	}
	std::string XmlLoader::GetValue(int count, std::string key)
	{

		std::string s = mSheetMap[mDefaultSheetName]->at(count)->at(key);
		return s;
	}
	int XmlLoader::GetElementCount(std::string sheet)
	{
		return  mSheetMap[sheet]->size();
	}
	int XmlLoader::GetElementCount()
	{
		return  mSheetMap[mDefaultSheetName]->size();
	}
	int XmlLoader::GetKeyCount()
	{
		return mKeyMap[mDefaultSheetName]->size();
	}
	std::string  XmlLoader::GetKey(int count)
	{
		return mKeyMap[mDefaultSheetName]->at(count);

	}

	int XmlLoader::GetKeyCount(std::string sheet)
	{
		return mKeyMap[sheet]->size();
	}
	std::string XmlLoader::GetKey(std::string sheeet, int count)
	{
		return mKeyMap[sheeet]->at(count);

	}
}