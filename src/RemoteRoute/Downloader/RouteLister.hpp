#ifndef __DDR_CLOUD_SERVICE_LIB_CLIENT_SIDE_ROUTE_LISTER_H_INCLUDED__
#define __DDR_CLOUD_SERVICE_LIB_CLIENT_SIDE_ROUTE_LISTER_H_INCLUDED__

#include <algorithm>
#include <vector>
#include <memory>
#include <atomic>
#include <iostream>
#include "Thirdparty/zlib/zlib.h"
#include "src/Network/BaseMessageDispatcher.h"
#include "src/Network/TcpClientBase.h"
#include "src/Utility/Timer.hpp"
#include "proto/RemoteRoute.pb.h"
#include "RouteInfo.h"

namespace DDRCloudService {

class ListerDispatcher : public DDRFramework::BaseMessageDispatcher
{
public:
	ListerDispatcher(std::vector<routeInfo> *pInfoArray, std::atomic<bool> *pbRIObtained,
		             DDRFramework::Timer *pTimer, DDRFramework::timer_id tid)
		: m_pInfoArray(pInfoArray), m_pbObtained(pbRIObtained), m_pTimer(pTimer), m_tid(tid)
	{
		pInfoArray->resize(0);
	}
	void Dispatch(std::shared_ptr<DDRFramework::BaseSocketContainer> spParentSocketContainer,
		          std::shared_ptr<DDRCommProto::CommonHeader> spHeader,
		          std::shared_ptr<google::protobuf::Message> spMsg) override
	{
		if ("RemoteRouteProto.rspListRoutes" == spMsg->GetTypeName()) {
			_analyze((RemoteRouteProto::rspListRoutes*)spMsg.get());
		}
		spParentSocketContainer->GetTcp()->Stop();
	}
private:
	std::atomic<bool> *m_pbObtained;
	std::vector<routeInfo> *m_pInfoArray;
	DDRFramework::Timer *m_pTimer;
	DDRFramework::timer_id m_tid;
	void _analyze(RemoteRouteProto::rspListRoutes *pRsp)
	{
		if (!m_pInfoArray || RemoteRouteProto::eOkay != pRsp->ret() ||
			pRsp->routerecords_sz() <= 0) {
			return;
		}
		m_pInfoArray->resize(0);

		RemoteRouteProto::MultipleRouteInfo mri;
		if (RemoteRouteProto::eNoZip == pRsp->ziptype()) {
			if (pRsp->routerecords_sz() != pRsp->routerecords().length() ||
				!mri.ParseFromArray(pRsp->routerecords().c_str(),
					                pRsp->routerecords().length())) {
				return;
			}
		} else if (RemoteRouteProto::eZLib == pRsp->ziptype()) {
			uLongf oriSz = (uLongf)pRsp->routerecords_sz();
			std::vector<char> buf(oriSz);
			if (Z_OK != uncompress((Bytef*)&buf[0], &oriSz,
					               (const Bytef*)pRsp->routerecords().c_str(),
					               (uLong)pRsp->routerecords().length()) ||
				oriSz != (uLongf)pRsp->routerecords_sz()) {
				return;
			}
			if (!mri.ParseFromArray(&buf[0], oriSz)) {
				return;
			}
		}
		if (mri.routes_size() != mri.downloadid_size()) {
			return;
		}

		m_pTimer->remove(m_tid);
		
		// mri okay
		m_pInfoArray->resize(mri.routes_size());
		for (int i = 0; i < mri.routes_size(); ++i) {
			(*m_pInfoArray)[i].version = mri.routes()[i].version();
			(*m_pInfoArray)[i].robotID = mri.routes()[i].robotid();
			(*m_pInfoArray)[i].routeName = mri.routes()[i].routename();
			(*m_pInfoArray)[i].crTime = mri.routes()[i].datetime();
			(*m_pInfoArray)[i].description = mri.routes()[i].description();
			(*m_pInfoArray)[i].downloadID = mri.downloadid()[i];
		}
		std::sort(m_pInfoArray->begin(), m_pInfoArray->end(),
			[](const routeInfo &ri1, const routeInfo &ri2) {return ri1.crTime < ri2.crTime; });
		*m_pbObtained = true;
	}
};

using namespace DDRFramework;
class RouteLister : public TcpClientBase
{
public:
	RouteLister(const char *pServerIP, const char *pServerPort, std::vector<routeInfo> *pInfoArray)
		: m_serIP(pServerIP), m_serPort(pServerPort), m_stage(0), m_pInfoArray(pInfoArray), m_bRIObtained(false)
	{
	}
	~RouteLister()
	{
		Stop();
	}

	void Start()
	{
		if (0 == m_stage) {
			m_nFailCnt = 0;
			m_stage = 1;
			try2reconnect();
		}
	}

	// 0 - initialized; 1 - connecting; 2 - successfully finished; -1 - time-out or too many failures
	int GetCurStage() const { return m_stage; }

protected:
	std::shared_ptr<TcpClientSessionBase> BindSerializerDispatcher() override
	{
		auto pTcpCltSess = std::make_shared<TcpClientSessionBase>(m_IOContext);
		auto pSerializer = std::make_shared<MessageSerializer>();
		pTcpCltSess->LoadSerializer(pSerializer);
		auto pLDisp = std::make_shared<ListerDispatcher>(m_pInfoArray, &m_bRIObtained, &m_timer, m_tid);
		auto pHeadRuleRouter = std::make_shared<BaseHeadRuleRouter>();
		pSerializer->Init();
		pSerializer->BindDispatcher(pLDisp, pHeadRuleRouter);
		return pTcpCltSess;
	}
	void OnDisconnect(std::shared_ptr<TcpSocketContainer> spContainer) override
	{
		try2reconnect();
	}
	void OnConnected(std::shared_ptr<TcpSocketContainer> spContainer) override
	{
		std::shared_ptr<google::protobuf::Message> pMsg = std::make_shared<RemoteRouteProto::reqListRoutes>();
		Send(pMsg);
	}
	void OnConnectTimeout(std::shared_ptr<TcpSocketContainer> spContainer) override
	{
		try2reconnect();
	}
	void OnConnectFailed(std::shared_ptr<TcpSocketContainer> spContainer) override
	{
		try2reconnect();
	}

private:
	int m_stage;
	std::string m_serIP, m_serPort;
	std::atomic<bool> m_bRIObtained;
	std::vector<routeInfo> *m_pInfoArray;
	DDRFramework::Timer m_timer;
	DDRFramework::timer_id m_tid;
	int m_nFailCnt;

	void try2reconnect()
	{
		if (m_bRIObtained) {
			m_stage = 2;
			return;
		}
		if (++m_nFailCnt > 5) {
			Stop();
			m_stage = -1;
		} else {
			TcpClientBase::Start(1);
			TcpClientBase::Connect(m_serIP, m_serPort);
			m_tid = m_timer.add(std::chrono::seconds(8), std::bind(&RouteLister::rsp2timeout, this));
		}
	}

	void rsp2timeout()
	{
		if (!m_bRIObtained) {
			m_timer.remove(m_tid);
			Stop();
		}
	}
};

}

#endif // __DDR_CLOUD_SERVICE_LIB_CLIENT_SIDE_ROUTE_LISTER_H_INCLUDED__

