#include "GlobalManagerBase.h"
namespace DDRFramework
{
	GlobalManagerBase::GlobalManagerBase():m_GlobalConfig("Config/Global.xml")
	{

	}
	GlobalManagerBase::~GlobalManagerBase()
	{

	}

	std::string GlobalManagerBase::GetRobotID()
	{
		return m_GlobalConfig.GetValue("RobotID");
	}

	DDRFramework::LocalizationLoader& GlobalManagerBase::GetLocalizationConfig()
	{
		return m_LocalizationConfig;
	}

}