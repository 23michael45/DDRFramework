/*!
 * File: LocalizationLoader.h
 * Date: 2019/04/29 16:01
 *
 * Author: michael
 * Contact: michael2345@live.cn
 *
 * Description:xml localization config file loader
 *
*/
#ifndef LocalizationLoader_h__
#define LocalizationLoader_h__


#include <map>
#include <string>
#include <vector>
#include "src/Utility/XmlLoader.h"

namespace DDRFramework
{
	class  LocalizationLoader : public XmlLoader
	{

		enum ELANGUAGE
		{
			Cn = 0,
			En = 1,

		};

	public:
		LocalizationLoader();

		void SetLanguage(ELANGUAGE lang);
		std::string GetString(std::string Key);

	private:
		ELANGUAGE m_CurrentLanguage;
		std::map<ELANGUAGE,std::string> m_LanguageStringMap;

	};
}
#endif // LocalizationLoader_h__