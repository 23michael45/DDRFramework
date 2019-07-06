#include "LocalTcpServer.h"
#include "src/Network/TcpSocketContainer.h"
#include "src/Network/MessageSerializer.h"
#include "src/Network/BaseMessageDispatcher.h"
#include "LocalServerDispatcher.h"
#include "LocalServerHeadRuleRouter.h"
#include "src/Utility/XmlLoader.h"


LocalServerTcpSession::LocalServerTcpSession(asio::io_context& context) :TcpSessionBase::TcpSessionBase(context)
{
}

LocalServerTcpSession::~LocalServerTcpSession()
{
	LevelLog(DDRFramework::Log::Level::INFO,"LocalServerTcpSession Destroy");
}

void LocalServerTcpSession::AssignLoginInfo(reqLogin info)
{
	m_reqLoginInfo.CopyFrom(info);

	GlobalManager::Instance()->GetTcpServer()->AddSessionType(info.type(), info.username(), shared_from_base());
}

DDRCommProto::reqLogin& LocalServerTcpSession::GetLoginInfo()
{
	return m_reqLoginInfo;
}





bool LocalServerTcpSession::HasLogin()
{
	if (m_reqLoginInfo.username().empty() && m_reqLoginInfo.userpwd().empty())
	{
		return false;
	}
	return true;
}

void LocalServerTcpSession::OnStart()
{
	TcpSessionBase::OnStart();

	auto spClientBehavior = std::make_shared<BaseClientBehavior>();
	BindBehavior(spClientBehavior);

}


LocalTcpServer::LocalTcpServer(int port):TcpServerBase(port)
{
}


LocalTcpServer::~LocalTcpServer()
{
}


std::shared_ptr<TcpSessionBase> LocalTcpServer::BindSerializerDispatcher()
{
	BIND_IOCONTEXT_SERIALIZER_DISPATCHER(m_IOContext, LocalServerTcpSession, MessageSerializer, LocalServerDispatcher, BaseHeadRuleRouter)
		return spLocalServerTcpSession;
}


std::shared_ptr<TcpSessionBase> LocalTcpServer::GetTcpSessionBySocket(tcp::socket* pSocket)
{
	if (m_SessionMap.find(pSocket) != m_SessionMap.end())
	{
		return m_SessionMap[pSocket];
	}
	return nullptr;
}

std::map<tcp::socket*, std::shared_ptr<TcpSessionBase>>& LocalTcpServer::GetTcpSocketContainerMap()
{
	return m_SessionMap;
}

void LocalTcpServer::OnSessionDisconnect(std::shared_ptr<TcpSocketContainer> spContainer)
{
	TcpServerBase::OnSessionDisconnect(spContainer);
	auto spClientSession = dynamic_pointer_cast<LocalServerTcpSession>(spContainer);
	
	if (spClientSession)
	{
		auto info = spClientSession->GetLoginInfo();
		RemoveSessionType(info.type(), info.username());

		spClientSession->DoFinishStop();
	}



}

void LocalTcpServer::DelayAddSessionType(eCltType type, std::string sname, std::shared_ptr<LocalServerTcpSession> sp)
{
	AddSessionType(type, sname, sp);
}
void LocalTcpServer::AddSessionType(eCltType type, std::string sname, std::shared_ptr<LocalServerTcpSession> sp)
{
	std::lock_guard<std::mutex> lock(mTypeSessionMapMutex);
	if (m_TypeSessionMap.find(type) == m_TypeSessionMap.end())
	{
		auto spMap = std::make_shared<std::map<std::string, std::shared_ptr<LocalServerTcpSession>>>();
		m_TypeSessionMap.insert(make_pair(type, spMap));
	}
	auto& spMap = m_TypeSessionMap[type];
	if (spMap->find(sname) == spMap->end())
	{
		spMap->insert(make_pair(sname, sp));
	}
	else
	{
		auto spold = spMap->at(sname);
		spold->Stop();//stop will remove it from map , insert new session after stop finish


		function<void()> bf;
		bf = std::bind(&LocalTcpServer::DelayAddSessionType,shared_from_base(), type, sname, sp);
		spold->BindDelayAddFunc(bf);

		LevelLog(DDRFramework::Log::Level::WARNING,"Add LocalServerTcpSession of type:%i name:%s already Exist", type, sname.c_str());
	}
}

void LocalTcpServer::RemoveSessionType(eCltType type, std::string sname)
{
	std::lock_guard<std::mutex> lock(mTypeSessionMapMutex);
	if (m_TypeSessionMap.find(type) != m_TypeSessionMap.end())
	{
		auto& spMap = m_TypeSessionMap[type];
		if (spMap->find(sname) != spMap->end())
		{
			spMap->erase(sname);
		}
		else
		{

			LevelLog(DDRFramework::Log::Level::ERR,"Remove LocalServerTcpSession of type:%i name:%s not Exist", type, sname.c_str());
		}
	}
	else
	{
		LevelLog(DDRFramework::Log::Level::ERR,"Remove LocalServerTcpSession of type:%i not Exist", type);
	}
}

std::shared_ptr<LocalServerTcpSession> LocalTcpServer::GetSessionByType(eCltType type)
{
	//std::lock_guard<std::mutex> lock(mTypeSessionMapMutex);
	if (m_TypeSessionMap.find(type) != m_TypeSessionMap.end())
	{
		auto spMap = m_TypeSessionMap[type];
		if (spMap->size() > 0)
		{
			auto sp = spMap->begin()->second;
			return sp;
		}
	}
	return nullptr;
}

std::shared_ptr<LocalServerTcpSession> LocalTcpServer::GetSessionByTypeName(eCltType type, std::string sname)
{
	//std::lock_guard<std::mutex> lock(mTypeSessionMapMutex);
	if (m_TypeSessionMap.find(type) == m_TypeSessionMap.end())
	{
		return nullptr;
	}
	auto& spMap = m_TypeSessionMap[type];
	if (spMap->find(sname) != spMap->end())
	{
		return spMap->at(sname);
	}
	else
	{
		return nullptr;
	}
}

