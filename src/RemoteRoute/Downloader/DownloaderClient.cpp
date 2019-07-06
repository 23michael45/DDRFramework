#include "DownloaderClient.h"

namespace DDRCloudService {

extern int g_CurrentRouteVersion;
extern int g_MaxDisRetrials;
extern int MAX_SZ_ONE_DATA_SLICE;

RDClientTcpSession::RDClientTcpSession(asio::io_context& context) : TcpClientSessionBase(context)
{
	GetRecvBuf().prepare(MAX_SZ_ONE_DATA_SLICE);
}

RDTcpClient::RDTcpClient(std::shared_ptr<DownloadManagerBase> pDMgt)
	: m_pDMgt(pDMgt), m_bStopped(false), m_nConnFailCnt(0)
{
	m_lastRcvTic = std::chrono::duration_cast<std::chrono::seconds>
		(std::chrono::system_clock::now().time_since_epoch()).count();
}

bool RDTcpClient::IsValid() const
{
	return (m_pDMgt && m_pDMgt->GetStage() >= 0);
}

bool RDTcpClient::IsDownloadingFinished() const
{
	return (m_pDMgt && 2 == m_pDMgt->GetStage());
}

bool RDTcpClient::IsStopped() const
{
	return m_bStopped;
}

__int64 RDTcpClient::GetSecondsSinceLastRcv() const
{
	auto cur = std::chrono::duration_cast<std::chrono::seconds>
		(std::chrono::system_clock::now().time_since_epoch()).count();
	return (cur - m_lastRcvTic);
}

std::shared_ptr<TcpClientSessionBase> RDTcpClient::BindSerializerDispatcher()
{
	auto pTcpCltSess = std::make_shared<RDClientTcpSession>(m_IOContext);
	auto pSerializer = std::make_shared<MessageSerializer>();
	pTcpCltSess->LoadSerializer(pSerializer);
	auto pRUDisp = std::make_shared<RDDispatcher>(m_pDMgt, &m_lastRcvTic);
	auto pHeadRuleRouter = std::make_shared<BaseHeadRuleRouter>();
	pSerializer->Init();
	pSerializer->BindDispatcher(pRUDisp, pHeadRuleRouter);
	return pTcpCltSess;
}

void RDTcpClient::OnDisconnect(std::shared_ptr<TcpSocketContainer> spContainer)
{
	m_bStopped = true;
	Stop();
}

void RDTcpClient::OnConnected(std::shared_ptr<TcpSocketContainer> spContainer)
{
	bool bSucc = false;
	if (IsValid()) {
		auto pMsg = m_pDMgt->GetNextMsg();
		if (pMsg) {
			Send(pMsg);
			bSucc = true;
			m_lastRcvTic = std::chrono::duration_cast<std::chrono::seconds>
				(std::chrono::system_clock::now().time_since_epoch()).count();
		}
	}
	if (!bSucc) {
		Disconnect();
	}
}

void RDTcpClient::OnConnectTimeout(std::shared_ptr<TcpSocketContainer> spContainer)
{
	m_bStopped = true;
	Stop();
}

void RDTcpClient::OnConnectFailed(std::shared_ptr<TcpSocketContainer> spContainer)
{
	m_bStopped = true;
	Stop();
}

}

