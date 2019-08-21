#include "UploaderClient.h"

namespace DDRCloudService {

extern int g_CurrentRouteVersion;
extern int g_MaxDisRetrials;
extern int MAX_SZ_ONE_DATA_SLICE;

RUClientTcpSession::RUClientTcpSession(asio::io_context& context) : TcpClientSessionBase(context)
{
	GetRecvBuf().prepare(MAX_SZ_ONE_DATA_SLICE);
}

RUTcpClient::RUTcpClient(const std::string &rID, const std::string &routeName,
	                     const std::string &serAddr, const std::string &serPort,
	                     bool bForceUpdate)
	: m_rID(rID), m_routeName(routeName),
	m_serAddr(serAddr), m_serPort(serPort), m_bForceUpdate(bForceUpdate),
	m_bStopped(false), m_nConnFailCnt(0)
{
	if (!try2load()) {
		m_pRSRM = nullptr;
	}
	m_lastRcvTic = std::chrono::duration_cast<std::chrono::seconds>
		(std::chrono::system_clock::now().time_since_epoch()).count();
}

bool RUTcpClient::IsRSRMWrong() const
{
	return (!m_pRSRM || m_pRSRM->GetStage() < 0);
}

bool RUTcpClient::IsUploadingFinished() const
{
	return (m_pRSRM && 2 == m_pRSRM->GetStage());
}

bool RUTcpClient::IsStopped() const
{
	return m_bStopped;
}

long long RUTcpClient::GetSecondsSinceLastRcv() const
{
	auto cur = std::chrono::duration_cast<std::chrono::seconds>
		(std::chrono::system_clock::now().time_since_epoch()).count();
	return (cur - m_lastRcvTic);
}

void RUTcpClient::Try2reconnect()
{
	std::lock_guard<std::mutex> lg(m_loc);
	if (++m_nConnFailCnt <= g_MaxDisRetrials && try2load()) { 
		Connect(m_serAddr, m_serPort);
	} else {
		m_bStopped = true;
		Stop();
	}
}

std::shared_ptr<TcpClientSessionBase> RUTcpClient::BindSerializerDispatcher()
{
	auto pTcpCltSess = std::make_shared<RUClientTcpSession>(m_IOContext);
	auto pSerializer = std::make_shared<MessageSerializer>();
	pTcpCltSess->LoadSerializer(pSerializer);
	auto pRUDisp = std::make_shared<RUDispatcher>(m_pRSRM, &m_lastRcvTic);
	auto pHeadRuleRouter = std::make_shared<BaseHeadRuleRouter>();
	pSerializer->Init();
	pSerializer->BindDispatcher(pRUDisp, pHeadRuleRouter);
	return pTcpCltSess;
}

void RUTcpClient::OnDisconnect(std::shared_ptr<TcpSocketContainer> spContainer)
{
	Try2reconnect();
}

void RUTcpClient::OnConnected(std::shared_ptr<TcpSocketContainer> spContainer)
{
	bool bSucc = false;
	if (!IsRSRMWrong()) {
		auto pMsg = m_pRSRM->GetNextMsg();
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

void RUTcpClient::OnConnectTimeout(std::shared_ptr<TcpSocketContainer> spContainer)
{
	Try2reconnect();
}

void RUTcpClient::OnConnectFailed(std::shared_ptr<TcpSocketContainer> spContainer)
{
	Try2reconnect();
}

bool RUTcpClient::try2load()
{
	m_pRSRM = std::make_shared<RobotSideRouteManager>(m_routeName.c_str());
	if (-1 == m_pRSRM->GetStage()) {
		std::string fn = m_routeName + "/__INFO__.ini";
		std::ofstream ofs(fn);
		ofs << g_CurrentRouteVersion << "\n"
			<< m_rID << "\n0\n0\n" << std::endl;
		if (!ofs) {
			return false;
		}
		ofs.close();
		m_pRSRM = std::make_shared<RobotSideRouteManager>(m_routeName.c_str());
		if (-1 == m_pRSRM->GetStage()) {
			return false;
		}
	} else if (m_bForceUpdate && 2 == m_pRSRM->GetStage()) {
		std::string fn = m_routeName + "/__INFO__.ini";
		int ver, tt, type;
		std::string rID;
		u64 uid;
		std::ifstream ifs(fn);
		if (!(ifs >> ver >> rID >> type >> tt >> uid) || 0 != type) {
			return false;
		}
		ifs.close();
		std::ofstream ofs(fn);
		ofs << ver << "\n" << rID << "\n0\n1\n" << uid << std::endl;
		if (!ofs) {
			return false;
		}
		ofs.close();
		m_pRSRM = std::make_shared<RobotSideRouteManager>(m_routeName.c_str());
		if (1 != m_pRSRM->GetStage()) {
			return false;
		}
	}
	return true;
}

}

