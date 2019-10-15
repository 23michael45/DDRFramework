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

	DDRFramework::XmlLoader& GlobalManagerBase::GetGlobalConfig()
	{
		return m_GlobalConfig;
	}

	GlobalManagerClientBase::GlobalManagerClientBase()
	{

	}

	GlobalManagerClientBase::~GlobalManagerClientBase()
	{
		if (m_spUdpClient)
		{
			m_spUdpClient.reset();
		}

		if (m_spTcpClient)
		{
			m_spTcpClient.reset();
		}
	}
	void GlobalManagerClientBase::Init()
	{
		if (!m_spUdpClient)
		{
			m_spUdpClient = std::make_shared<UdpSocketBase>();
			m_spUdpClient->BindOnDisconnect(std::bind(&GlobalManagerClientBase::OnUdpDisconnect, this, std::placeholders::_1));

			StartUdp();
		}
		if (!m_spTcpClient)
		{
			m_spTcpClient = std::make_shared<TcpClientBase>();
			m_spTcpClient->Start();
		}

		

	}

	bool GlobalManagerClientBase::StartUdp()
	{
		if (m_spUdpClient)
		{
			m_spUdpClient->Start();

			auto spDispatcher = std::make_shared<BaseUdpMessageDispatcher>();
			spDispatcher->AddProcessor<bcLSAddr, BaseProcessor>();
			m_spUdpClient->GetSerializer()->BindDispatcher(spDispatcher);
			m_spUdpClient->StartReceive(m_GlobalConfig.GetValue<int>("UdpPort"));
		}
		return true;
	}

	void GlobalManagerClientBase::StopUdp()
	{
		if (m_spUdpClient && m_spUdpClient->IsWorking())
		{
			m_spUdpClient->StopReceive();
			m_spUdpClient->Stop();
			//m_spUdpClient.reset();donot reset here cause Stop is async ,it will release when OnDisconnect is called

		}
	}

	std::shared_ptr<TcpClientBase> GlobalManagerClientBase::GetTcpClient()
	{
        return m_spTcpClient;
    }

	std::shared_ptr<UdpSocketBase> GlobalManagerClientBase::GetUdpClient()
	{
		return m_spUdpClient;
	}

	void GlobalManagerClientBase::TcpConnect(std::string ip, std::string port)
	{
		GetTcpClient()->Connect(ip, port);
	}

	void GlobalManagerClientBase::OnUdpDisconnect(UdpSocketBase& container)
	{
		if (m_spUdpClient)
		{
			m_spUdpClient->Stop();
		}
	}
}
