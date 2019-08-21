#ifndef __DDR_CLOUD_SERVICE_LIB_ROBOT_SIDE_CLIENT_H_INCLUDED__
#define __DDR_CLOUD_SERVICE_LIB_ROBOT_SIDE_CLIENT_H_INCLUDED__

#include <atomic>
#include "src/Network/TcpClientBase.h"
using namespace DDRFramework;
#include "RDDispatcher.h"

namespace DDRCloudService {
	
class RDClientTcpSession : public TcpClientSessionBase
{
public:
	RDClientTcpSession(asio::io_context& context);
	~RDClientTcpSession() {}
	SHARED_FROM_BASE(RDClientTcpSession)
};

class RDTcpClient : public TcpClientBase
{
public:
	RDTcpClient(std::shared_ptr<DownloadManagerBase> pDMgt);
	bool IsValid() const;
	bool IsDownloadingFinished() const;
	bool IsStopped() const;
	long long GetSecondsSinceLastRcv() const;

protected:
	std::shared_ptr<TcpClientSessionBase> BindSerializerDispatcher() override;
	void OnDisconnect(std::shared_ptr<TcpSocketContainer> spContainer) override;
	void OnConnected(std::shared_ptr<TcpSocketContainer> spContainer) override;
	void OnConnectTimeout(std::shared_ptr<TcpSocketContainer> spContainer) override;
	void OnConnectFailed(std::shared_ptr<TcpSocketContainer> spContainer) override;

private:
	std::shared_ptr<DownloadManagerBase> m_pDMgt;
	bool m_bForceUpdate;
	bool m_bStopped;
	std::mutex m_loc;
	int m_nConnFailCnt;
	std::atomic<long long> m_lastRcvTic;
};

}

#endif // __DDR_CLOUD_SERVICE_LIB_ROBOT_SIDE_CLIENT_H_INCLUDED__
