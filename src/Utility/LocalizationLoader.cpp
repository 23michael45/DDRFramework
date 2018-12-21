#include "LocalizationLoader.h"

namespace DDRFramework
{
	LocalizationLoader::LocalizationLoader() :XmlLoader("Config/Localization.xml")
	{
		m_LanguageStringMap.insert(std::make_pair(ELANGUAGE::Cn, "Cn"));
		m_LanguageStringMap.insert(std::make_pair(ELANGUAGE::En, "En"));

		m_CurrentLanguage = ELANGUAGE::Cn;
	}


	void LocalizationLoader::SetLanguage(ELANGUAGE lang)
	{
		m_CurrentLanguage = lang;

	}

	std::string LocalizationLoader::GetString(std::string Key)
	{
		int index = m_RowKeyIndexMap[m_DefaultSheetName]->at(Key);
		return GetValue(m_DefaultSheetName, index, m_LanguageStringMap[m_CurrentLanguage]);
	}
}