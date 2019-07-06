#ifndef __DDR_CLOUD_SERVICE_LIB_ROBOT_SIDE_CLIENT_H_INCLUDED__
#define __DDR_CLOUD_SERVICE_LIB_ROBOT_SIDE_CLIENT_H_INCLUDED__

#include <atomic>
#include "src/Network/TcpClientBase.h"
using namespace DDRFramework;
#include "RUDispatcher.h"

namespace DDRCloudService {
	
class RUClientTcpSession : public TcpClientSessionBase
{
public:
	RUClientTcpSession(asio::io_context& context);
	~RUClientTcpSession() {}
	SHARED_FROM_BASE(RUClientTcpSession)
};

class RUTcpClient : public TcpClientBase
{
public:
	RUTcpClient(const std::string &routeName, const std::string &rID,
		        const std::string &serAddr, const std::string &serPort,
		        bool bForceUpdate);
	bool IsRSRMWrong() const;
	bool IsUploadingFinished() const;
	bool IsStopped() const;
	__int64 GetSecondsSinceLastRcv() const;
	void Try2reconnect();

protected:
	std::shared_ptr<TcpClientSessionBase> BindSerializerDispatcher() override;
	void OnDisconnect(std::shared_ptr<TcpSocketContainer> spContainer) override;
	void OnConnected(std::shared_ptr<TcpSocketContainer> spContainer) override;
	void OnConnectTimeout(std::shared_ptr<TcpSocketContainer> spContainer) override;
	void OnConnectFailed(std::shared_ptr<TcpSocketContainer> spContainer) override;

private:
	bool try2load();
	std::shared_ptr<RobotSideRouteManager> m_pRSRM;
	std::string m_routeName, m_rID, m_serAddr, m_serPort;
	bool m_bForceUpdate;
	bool m_bStopped;
	std::mutex m_loc;
	int m_nConnFailCnt;
	std::atomic<__int64> m_lastRcvTic;
};

}

#endif // __DDR_CLOUD_SERVICE_LIB_ROBOT_SIDE_CLIENT_H_INCLUDED__
