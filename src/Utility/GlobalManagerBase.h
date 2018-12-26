#ifndef GlobalManagerBase_h__
#define GlobalManagerBase_h__
#include "LocalizationLoader.h"
#include "XmlLoader.h"

namespace DDRFramework
{


	class GlobalManagerBase
	{
	public:
		GlobalManagerBase();
		~GlobalManagerBase();

		std::string GetRobotID();

		LocalizationLoader& GetLocalizationConfig();

	protected:

		LocalizationLoader m_LocalizationConfig;
		XmlLoader m_GlobalConfig;
	};
}
#endif // GlobalManagerBase_h__
