#ifndef GlobalManagerBase_h__
#define GlobalManagerBase_h__
#include "src/Utility//LocalizationLoader.h"
#include "src/Utility/XmlLoader.h"
#include "src/Network/TcpClientBase.h"
#include "src/Network/UdpSocketBase.h"

#include <proto/RemoteCmd.pb.h>
namespace DDRFramework
{


	class GlobalManagerBase
	{
	public:
		GlobalManagerBase();
		~GlobalManagerBase();

		std::string GetRobotID();

		LocalizationLoader& GetLocalizationConfig();
		XmlLoader& GetGlobalConfig();

	protected:

		LocalizationLoader m_LocalizationConfig;
		XmlLoader m_GlobalConfig;
	};

	class GlobalManagerClientBase : public GlobalManagerBase
	{
	public:
		GlobalManagerClientBase();
		virtual ~GlobalManagerClientBase();

		virtual void Init();
		
		virtual bool StartUdp();
		virtual void StopUdp();

        std::shared_ptr<TcpClientBase> GetTcpClient();
        std::shared_ptr<TcpClientBase> GetAITcpClient();
		std::shared_ptr<UdpSocketBase> GetUdpClient();

		virtual void TcpConnect(std::string ip, std::string port);

		virtual void OnUdpDisconnect(UdpSocketBase& container);

	protected:

        std::shared_ptr<TcpClientBase> m_spTcpClient,m_aiTcpClient;
		std::shared_ptr<UdpSocketBase> m_spUdpClient;
	};
}
#endif // GlobalManagerBase_h__
