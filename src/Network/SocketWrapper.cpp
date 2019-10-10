#include "SocketWrapper.h"

#include <vector>
#include <atomic>
#if defined(_WIN32) || defined(_WIN64)
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>
#pragma comment (lib,"ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#elif defined(__linux__)
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
//#include <sys/epoll.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <linux/if_link.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#endif

namespace DDRCommLib {

#if defined(_WIN32) || defined(_WIN64)
class WSAInitializer
{
public:
	WSAInitializer() {
		WSAData wsaData;
		int iErrorMsg = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iErrorMsg != NO_ERROR) {
			//std::cout << "WSAStartup()失败,错误码=" << iErrorMsg << ".按回车键退出...\n";
		}
	}
	~WSAInitializer() {
		::WSACleanup();
	}
};
WSAInitializer __wsaInitializer;
#endif

struct SocketWrapper::__refInfo
{
#if defined(_WIN32) || defined(_WIN64)
	SOCKET soc;
#elif defined(__linux__)
	int soc;
#endif
	std::atomic<int> nRefCnt;
	int protocolType; // 1 - TCP; 2 - UDP
};

SocketWrapper::SocketWrapper(int type) : m_pRef(nullptr)
{
	switch (type) {
	case 1:
		UDP_Init();
		break;
	case 2:
		TCP_Init();
		break;
	default:
		break;
	}
}

SocketWrapper::SocketWrapper(const SocketWrapper &oriSW)
{
	m_pRef = oriSW.m_pRef;
	if (m_pRef) {
		++(m_pRef->nRefCnt);
	}
}

SocketWrapper::SocketWrapper(SocketWrapper &&oriSW)
{
	m_pRef = oriSW.m_pRef;
	oriSW.m_pRef = nullptr;
}

SocketWrapper& SocketWrapper::operator= (const SocketWrapper &oriSW)
{
	Close();
	m_pRef = oriSW.m_pRef;
	if (m_pRef) {
		++(m_pRef->nRefCnt);
	}
	return (*this);
}

SocketWrapper& SocketWrapper::operator= (SocketWrapper &&oriSW)
{
	Close();
	m_pRef = oriSW.m_pRef;
	oriSW.m_pRef = nullptr;
	return (*this);
}

SocketWrapper::~SocketWrapper()
{
	Close();
}

void SocketWrapper::Release()
{
	if (m_pRef) {
		if (--(m_pRef->nRefCnt) == 0) {
			delete m_pRef;
		}
		m_pRef = nullptr;
	}
}

void SocketWrapper::Close()
{
	if (m_pRef) {
		if (--(m_pRef->nRefCnt) == 0) {
#if defined(_WIN32) || defined(_WIN64)
			closesocket(m_pRef->soc);
#elif defined(__linux__)
			close(m_pRef->soc);
#endif
			delete m_pRef;
		}
		m_pRef = nullptr;
	}
}

bool SocketWrapper::IsValid() const
{
	return (m_pRef != nullptr);
}

bool SocketWrapper::SetBuffer(int sndBufSz, int rcvBufSz)
{
	if (m_pRef) {
		if (setsockopt(m_pRef->soc, SOL_SOCKET, SO_SNDBUF,
			           (const char*)(&sndBufSz), sizeof(int)) < 0) {
			return false;
		}
		if (setsockopt(m_pRef->soc, SOL_SOCKET, SO_RCVBUF,
			           (const char*)(&rcvBufSz), sizeof(int)) < 0) {
			return false;
		}
		return true;
	}
	return false;
}

bool SocketWrapper::SetBlocking(bool bBlocking)
{
	if (m_pRef) {
#if defined(_WIN32) || defined(_WIN64)
		unsigned long dBlk = bBlocking ? 0 : 1;
		return (0 == ioctlsocket(m_pRef->soc, FIONBIO, &dBlk));
#elif defined(__linux__)
		int flags = fcntl(m_pRef->soc, F_GETFL, 0);
		if (-1 == flags) {
			return false;
		}
		flags = bBlocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
		return (0 == fcntl(m_pRef->soc, F_SETFL, flags));
#endif
	}
	return false;
}

bool SocketWrapper::SetAddrReuse(bool bReusable)
{
	if (m_pRef) {
		int ttt = bReusable ? 1 : 0;
#if defined(_WIN32) || defined(_WIN64)
		return (setsockopt(m_pRef->soc, SOL_SOCKET, SO_REUSEADDR,
			               (const char*)(&ttt), sizeof(int)) >= 0);
#elif defined(__linux__)
		if (setsockopt(m_pRef->soc, SOL_SOCKET, SO_REUSEADDR,
			           (const char*)(&ttt), sizeof(int)) < 0) {
			return false;
		}
		if (setsockopt(m_pRef->soc, SOL_SOCKET, SO_REUSEPORT,
			           (const char*)(&ttt), sizeof(int)) < 0) {
			return false;
		}
		return true;
#endif
	}
	return false;
}

bool SocketWrapper::Bind(unsigned long tarIP_BE, unsigned short tarPort_BE)
{
	if (m_pRef) {
		sockaddr_in tarAddr;
		tarAddr.sin_family = AF_INET;
		tarAddr.sin_addr.s_addr = tarIP_BE;
		tarAddr.sin_port = tarPort_BE;
		return (0 == bind(m_pRef->soc, (sockaddr*)(&tarAddr), sizeof(sockaddr_in)));
	}
	return false;
}

bool SocketWrapper::GetBoundAddr(unsigned long *pIP_BE, unsigned short *pPort_BE)
{
	sockaddr_in myAddr;
	int lll = sizeof(sockaddr_in);
#if defined(_WIN32) || defined(_WIN64)
	if (getsockname(m_pRef->soc, (sockaddr*)(&myAddr), &lll) < 0) {
#elif defined(__linux__)
	if (getsockname(m_pRef->soc, (sockaddr*)(&myAddr), (socklen_t*)(&lll)) < 0) {
#endif
		return false;
	}
	if (pIP_BE) {
		*pIP_BE = myAddr.sin_addr.s_addr;
	}
	if (pPort_BE) {
		*pPort_BE = myAddr.sin_port;
	}
	return true;
}

bool SocketWrapper::HasError()
{
	int err, len = sizeof(int);
	if (0 == getsockopt(m_pRef->soc, SOL_SOCKET, SO_ERROR, (char*)(&err), (socklen_t*)&len) &&
		0 == err) {
		return false;
	}
	return true;
}

bool SocketWrapper::UDP_Init()
{
	Close();
	m_pRef = new __refInfo;
	if (!m_pRef) {
		return false;
	}
	m_pRef->soc = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == m_pRef->soc) {
		delete m_pRef;
		m_pRef = nullptr;
		return false;
	}
	m_pRef->nRefCnt = 1;
	m_pRef->protocolType = 1;
	return true;
}

int SocketWrapper::UDP_SendTo(const char *pData, int nDataLen,
	                          unsigned long tarIP_BE, unsigned short tarPort_BE)
{
	if (!m_pRef || 1 != m_pRef->protocolType) {
		return -1;
	}
	sockaddr_in tarAddr;
	tarAddr.sin_family = AF_INET;
	tarAddr.sin_addr.s_addr = tarIP_BE;
	tarAddr.sin_port = tarPort_BE;
	return sendto(m_pRef->soc, pData, nDataLen, 0, (const sockaddr*)(&tarAddr), sizeof(sockaddr_in));
}

int SocketWrapper::UDP_RecvFrom(char *pBuf, int nBufLen,
	                            unsigned long *pPeerIP_BE,
								unsigned short *pPeerPort_BE)
{
	if (!m_pRef || 1 != m_pRef->protocolType) {
		return -1;
	}
	sockaddr_in peerAddr;
	int len = sizeof(sockaddr_in);
#if defined(_WIN32) || defined(_WIN64)
	int n = recvfrom(m_pRef->soc, pBuf, nBufLen, 0,
		             (sockaddr*)(&peerAddr), &len);
#elif defined(__linux__)
	int n = recvfrom(m_pRef->soc, pBuf, nBufLen, 0,
		             (sockaddr*)(&peerAddr), (socklen_t*)(&len));
#endif
	if (n >= 0) {
		if (pPeerIP_BE) {
			*pPeerIP_BE = peerAddr.sin_addr.s_addr;
		}
		if (pPeerPort_BE) {
			*pPeerPort_BE = peerAddr.sin_port;
		}
	}
	return n;
}

bool SocketWrapper::UDP_JoinMulticast(unsigned long srcIP_BE,
	                                  unsigned long mcIP_BE, unsigned short mcPort_BE)
{
	if (!m_pRef || 1 != m_pRef->protocolType) {
		return false;
	}
	if (!SetAddrReuse(true)) {
		return false;
	}
	std::vector<unsigned long> IP_n;
	if (!ListAllInterfaces(IP_n) || IP_n.empty()) {
		return false;
	}
	sockaddr_in myAddr;
	myAddr.sin_family = AF_INET;
	myAddr.sin_port = mcPort_BE;
	myAddr.sin_addr.s_addr = 0;
	for (int i = 0; i < (int)IP_n.size(); ++i) {
		if ((IP_n[i] & 0xFFFFFF) == (srcIP_BE & 0xFFFFFF)) {
			myAddr.sin_addr.s_addr = IP_n[i];
			break;
		}
	}
	if (0 == myAddr.sin_addr.s_addr) { return false; }
	if (bind(m_pRef->soc, (struct sockaddr*)&myAddr, sizeof(myAddr)) < 0) {
		return false;
	}
	ip_mreq mreq;
	mreq.imr_multiaddr.s_addr = mcIP_BE;
	mreq.imr_interface.s_addr = myAddr.sin_addr.s_addr;
	if (setsockopt(m_pRef->soc, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*)&mreq, sizeof(mreq)) < 0) {
		return false;
	}
	return true;
}

bool SocketWrapper::TCP_Init()
{
	Close();
	m_pRef = new __refInfo;
	if (!m_pRef) {
		return false;
	}
	m_pRef->soc = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == m_pRef->soc) {
		delete m_pRef;
		m_pRef = nullptr;
		return false;
	}
	m_pRef->nRefCnt = 1;
	m_pRef->protocolType = 2;
	return true;
}

int SocketWrapper::TCP_Send(const char *pData, int nDataLen)
{
	if (!m_pRef || 2 != m_pRef->protocolType) {
		return -1;
	}
	return (int)send(m_pRef->soc, pData, nDataLen, 0);
}

int SocketWrapper::TCP_Recv(char *pBuf, int nBufLen)
{
	if (!m_pRef || 2 != m_pRef->protocolType) {
		return -1;
	}
#if defined(_WIN32) || defined(_WIN64)
	return recv(m_pRef->soc, pBuf, nBufLen, 0);
#elif defined(__linux__)
	return (int)recv(m_pRef->soc, (void*)pBuf, (size_t)nBufLen, 0);
#endif
}

bool SocketWrapper::TCP_Listen()
{
	if (!m_pRef || 2 != m_pRef->protocolType) {
		return false;
	}
	return (0 == listen(m_pRef->soc, 1));
}

bool SocketWrapper::TCP_Accept(SocketWrapper *pTarSoc,
	                           unsigned long *pPeerIP_BE,
							   unsigned short *pPeerPort_BE)
{
	if (!m_pRef || 2 != m_pRef->protocolType) {
		return false;
	}
	sockaddr_in peerAddr;
	int len = sizeof(sockaddr);
#if defined(_WIN32) || defined(_WIN64)
	SOCKET s = accept(m_pRef->soc, (sockaddr*)(&peerAddr), &len);
	if (s != INVALID_SOCKET) {
#elif defined(__linux__)
	int s = accept(m_pRef->soc, (sockaddr*)(&peerAddr), (socklen_t*)(&len));
	if (s >= 0) {
#endif
		if (pTarSoc) {
			pTarSoc->Close();
			pTarSoc->m_pRef = new __refInfo;
			if (!m_pRef) {
				return false;
			}
			pTarSoc->m_pRef->soc = s;
			pTarSoc->m_pRef->nRefCnt = 1;
			pTarSoc->m_pRef->protocolType = 2;
		}
		if (pPeerIP_BE) {
			*pPeerIP_BE = peerAddr.sin_addr.s_addr;
		}
		if (pPeerPort_BE) {
			*pPeerPort_BE = peerAddr.sin_port;
		}
		return true;
	}
	return false;
}

bool SocketWrapper::TCP_Connect(unsigned long tarIP_BE, unsigned short tarPort_BE)
{
	if (!m_pRef || 2 != m_pRef->protocolType) {
		return false;
	}
	sockaddr_in tarAddr;
	tarAddr.sin_family = AF_INET;
	tarAddr.sin_addr.s_addr = tarIP_BE;
	tarAddr.sin_port = tarPort_BE;
	return (0 == connect(m_pRef->soc, (const sockaddr*)(&tarAddr), sizeof(sockaddr)));
}

bool ListAllInterfaces(std::vector<unsigned long> &IPVec_n)
{
#if defined(_WIN32) || defined(_WIN64)
	PMIB_IPADDRTABLE pIPAddrTable;
	DWORD dwSize = 0;
	DWORD dwRetVal = 0;
	pIPAddrTable = (MIB_IPADDRTABLE *)malloc(sizeof(MIB_IPADDRTABLE));
	if (pIPAddrTable) {
		if (GetIpAddrTable(pIPAddrTable, &dwSize, 0) == ERROR_INSUFFICIENT_BUFFER) {
			free(pIPAddrTable);
			pIPAddrTable = (MIB_IPADDRTABLE *)malloc(dwSize);
		}
		if (!pIPAddrTable) { return false; }
	}
	if ((dwRetVal = GetIpAddrTable(pIPAddrTable, &dwSize, 0)) != NO_ERROR) {
		return false;
	}
	IPVec_n.resize(0);
	for (int i = 0; i < (int)pIPAddrTable->dwNumEntries; ++i) {
		if (pIPAddrTable->table[i].dwAddr != 0x0100007F) {
			IPVec_n.emplace_back(pIPAddrTable->table[i].dwAddr);
		}
	}
	if (pIPAddrTable) {
		free(pIPAddrTable);
	}
#elif defined(__linux__)
	struct ifaddrs *pIfAddr;
	if (-1 == getifaddrs(&pIfAddr)) {
		return false;
	}
	IPVec_n.resize(0);
	int n = 0;
	struct ifaddrs *pIFA = pIfAddr;
	for (; pIFA; pIFA = pIFA->ifa_next, ++n) {
		if (!(pIFA->ifa_addr) ||
		    AF_INET != pIFA->ifa_addr->sa_family) {
			continue;
        }
        unsigned long interfaceIP = ((sockaddr_in*)(pIFA->ifa_addr))->sin_addr.s_addr;
        if (0x0100007F != interfaceIP) {
            IPVec_n.emplace_back(interfaceIP);
        }
	}
	freeifaddrs(pIfAddr);
#endif
	return true;
}

struct MIMCServer::_IMPL
{
	std::vector<SocketWrapper> socs;
	std::vector<unsigned long> ips;
	std::vector<unsigned short> ports;
	unsigned long tarIP;
	unsigned short tarPort;
	_IMPL() {
		_refresh();
		_updateAddr();
	}
	void _refresh() {
		ips.clear();
		ports.clear();
		socs.clear();
		std::vector<unsigned long> IP_n;
		if (!ListAllInterfaces(IP_n) || IP_n.empty()) {
            return;
        }
		socs.resize(0);
		for (int i = 0; i < (int)IP_n.size(); ++i) {
			SocketWrapper s(1);
			if (!s.IsValid()) {
				continue;
			}
			if (s.Bind(IP_n[i], 0)) {
				socs.emplace_back(s);
			}
		}
	}
	void _updateAddr() {
		ips.resize(0);
		ports.resize(0);
		sockaddr_in myAddr;
		int lll;
		for (int i = (int)socs.size() - 1; i >= 0; --i) {
			lll = sizeof(sockaddr_in);
			if (getsockname(socs[i].m_pRef->soc, (sockaddr*)&myAddr, (socklen_t*)&lll) < 0) {
				socs.erase(socs.begin() + i);
			} else {
				ips.emplace_back(myAddr.sin_addr.s_addr);
				ports.emplace_back(myAddr.sin_port);
			}
		}
	}
};

MIMCServer::MIMCServer()
{
	m_pImpl = new _IMPL;
}

bool MIMCServer::IsValid() const
{
	return (m_pImpl != nullptr);
}

bool MIMCServer::ReInit()
{
	if (m_pImpl) {
		delete m_pImpl;
	}
	return ((m_pImpl = new _IMPL) != nullptr);
}

void MIMCServer::Refresh()
{
	if (m_pImpl) {
		m_pImpl->_refresh();
	} else {
		m_pImpl = new _IMPL;
	}
}

void MIMCServer::Close()
{
	if (m_pImpl) {
		delete m_pImpl;
		m_pImpl = nullptr;
	}
}

MIMCServer::~MIMCServer()
{
    Close();
}

bool MIMCServer::SetSockOpt(int level, int optname, const char *optval, int optlen)
{
	if (!m_pImpl) {
		return false;
	}
	bool bRet = true;
	for (int i = 0; i < (int)m_pImpl->socs.size(); ++i) {
		if (m_pImpl->socs[i].IsValid()) {
			if (setsockopt(m_pImpl->socs[i].m_pRef->soc, level, optname, optval, optlen) < 0) {
				bRet = false;
			}
		}
	}
	return bRet;
}

bool MIMCServer::SetBuffer(int sndBufSz, int rcvBufSz)
{
	if (!m_pImpl) {
		return false;
	}
	bool bRet = true;
	for (int i = 0; i < (int)m_pImpl->socs.size(); ++i) {
		if (m_pImpl->socs[i].IsValid()) {
			if (!m_pImpl->socs[i].SetBuffer(sndBufSz, rcvBufSz)) {
				bRet = false;
			}
		}
	}
	return true;
}

bool MIMCServer::SetMCTarget(unsigned long tarMCIP_BE, unsigned short tarMCPort_BE)
{
	if (m_pImpl) {
		m_pImpl->tarIP = tarMCIP_BE;
		m_pImpl->tarPort = tarMCPort_BE;
		return true;
	}
	return false;
}

bool MIMCServer::Broadcast(const char *pBuf, int nBufLen)
{
	if (!m_pImpl) {
		return false;
	}
	for (int i = 0; i < (int)m_pImpl->socs.size(); ++i) {
		m_pImpl->socs[i].UDP_SendTo(pBuf, nBufLen, m_pImpl->tarIP, m_pImpl->tarPort);
	}
	return true;
}

bool MIMCServer::IsFromCurServer(unsigned long srcIP_BE, unsigned short srcPort_BE) const
{
	if (m_pImpl) {
		for (int i = 0; i < (int)m_pImpl->ips.size(); ++i) {
			if (srcIP_BE == m_pImpl->ips[i] && srcPort_BE == m_pImpl->ports[i]) {
				return true;
			}
		}
	}
	return false;
}

struct MIMCClient::_IMPL
{
	std::vector<SocketWrapper> socs;
	bool _join(unsigned long tarMCIP_BE, unsigned short tarMCPort_BE) {
        socs.clear();
		std::vector<unsigned long> IP_n;
		if (!ListAllInterfaces(IP_n) || IP_n.empty()) {
            return false;
		}
		for (int i = 0; i < (int)IP_n.size(); ++i) {
            SocketWrapper s(1);
            if (!s.SetAddrReuse(true) ||
#if defined(_WIN32) || defined(_WIN64)
                !s.Bind(IP_n[i], tarMCPort_BE)) {
#elif defined(_linux__)
                !s.Bind(tarMCIP_BE, tarMCPort_BE)) {
#endif
                continue;
            }
			struct ip_mreq mreq;
			mreq.imr_multiaddr.s_addr = tarMCIP_BE;
			mreq.imr_interface.s_addr = IP_n[i];
			if (setsockopt(s.m_pRef->soc, IPPROTO_IP, IP_ADD_MEMBERSHIP,
				           (const char*)&mreq, sizeof(mreq)) < 0) {
                continue;
			}
			socs.emplace_back(s);
		}
		return (socs.size() > 0);
	}
};

MIMCClient::MIMCClient()
{
	m_pImpl = new _IMPL;
}

void MIMCClient::Close()
{
	if (m_pImpl) {
		delete m_pImpl;
		m_pImpl = nullptr;
	}
}

bool MIMCClient::IsValid() const
{
    return (m_pImpl != nullptr);
}

bool MIMCClient::ReInit()
{
	if (m_pImpl) {
		delete m_pImpl;
	}
	return ((m_pImpl = new _IMPL) != nullptr);
}

MIMCClient::~MIMCClient()
{
    Close();
}

void MIMCClient::AddToSet(SocSet *pSet)
{
	if (m_pImpl) {
		for (auto &soc : m_pImpl->socs) {
			pSet->Add2Set(soc);
		}
	}
}

bool MIMCClient::IsSet(SocSet *pSet)
{
	if (m_pImpl) {
		for (auto &soc : m_pImpl->socs) {
			if (pSet->IsSet(soc)) {
				return true;
			}
		}
	}
	return false;
}

bool MIMCClient::SetBuffer(int sndBufSz, int rcvBufSz)
{
	if (!m_pImpl) {
		return false;
	}
	bool bRet = true;
	for (int i = 0; i < (int)m_pImpl->socs.size(); ++i) {
		if (m_pImpl->socs[i].IsValid()) {
			if (!m_pImpl->socs[i].SetBuffer(sndBufSz, rcvBufSz)) {
				bRet = false;
			}
		}
	}
	return true;
}

bool MIMCClient::SetSockOpt(int level, int optname, const char *optval, int optlen)
{
	if (!m_pImpl) {
		return false;
	}
	bool bRet = true;
	for (int i = 0; i < (int)m_pImpl->socs.size(); ++i) {
		if (m_pImpl->socs[i].IsValid()) {
			if (setsockopt(m_pImpl->socs[i].m_pRef->soc, level, optname, optval, optlen) < 0) {
				bRet = false;
			}
		}
	}
	return bRet;
}

bool MIMCClient::JoinGroup(unsigned long tarMCIP_BE, unsigned short tarMCPort_BE)
{
    if (m_pImpl) {
        return m_pImpl->_join(tarMCIP_BE, tarMCPort_BE);
    }
    return false;
}

int MIMCClient::RecvFrom(char *pBuf, int nBufLen,
				         unsigned long *pPeerIP_BE, unsigned short *pPeerPort_BE)
{
    if (m_pImpl) {
		fd_set ss;
		FD_ZERO(&ss);
		for (int i = 0; i < (int)m_pImpl->socs.size(); ++i) {
            if (m_pImpl->socs[i].IsValid()) {
                FD_SET(m_pImpl->socs[i].m_pRef->soc, &ss);
			}
		}

		timeval ztv = { 0, 0 };
		if (select(255, &ss, nullptr, nullptr, &ztv) > 0) {
			for (int i = 0; i < (int)m_pImpl->socs.size(); ++i) {
				if (FD_ISSET(m_pImpl->socs[i].m_pRef->soc, &ss)) {
                    return m_pImpl->socs[i].UDP_RecvFrom(pBuf, nBufLen, pPeerIP_BE, pPeerPort_BE);
				}
			}
		}
	}
	return 0;
}


struct SocSet::__IMPL
{
	fd_set set;
#ifdef __linux__
	int maxFD;
#endif
};

SocSet::SocSet()
{
	m_pImp = new __IMPL;
	ClearAll();
}

void SocSet::ClearAll()
{
	if (m_pImp) {
		FD_ZERO(&(m_pImp->set));
#ifdef __linux__
		m_pImp->maxFD = 0;
#endif
	}
}

void SocSet::Clear(SocketWrapper &soc)
{
	if (m_pImp && soc.IsValid()) {
		FD_CLR(soc.m_pRef->soc, &(m_pImp->set));
	}
}

bool SocSet::Add2Set(SocketWrapper &soc)
{
	if (m_pImp && soc.IsValid()) {
		FD_SET(soc.m_pRef->soc, &(m_pImp->set));
#ifdef __linux__
		if (soc.m_pRef->soc > m_pImp->maxFD) {
			m_pImp->maxFD = soc.m_pRef->soc;
		}
#endif
		return true;
	}
	return false;
}

int Select(SocSet *pRdSet, SocSet *pWrSet, SocSet *pExSet, int timeMs)
{
	fd_set *pRD = nullptr, *pWR = nullptr, *pEX = nullptr;
	int maxfd = 0;
	if (pRdSet && pRdSet->m_pImp) {
		pRD = &(pRdSet->m_pImp->set);
#if defined(__linux__)
		if (pRdSet->m_pImp->maxFD > maxfd) {
			maxfd = pRdSet->m_pImp->maxFD;
		}
#endif
	}
	if (pWrSet && pWrSet->m_pImp) {
		pWR = &(pWrSet->m_pImp->set);
#if defined(__linux__)
		if (pWrSet->m_pImp->maxFD > maxfd) {
			maxfd = pWrSet->m_pImp->maxFD;
		}
#endif
	}
	if (pExSet && pExSet->m_pImp) {
		pEX = &(pExSet->m_pImp->set);
#if defined(__linux__)
		if (pExSet->m_pImp->maxFD > maxfd) {
			maxfd = pExSet->m_pImp->maxFD;
		}
#endif
	}
	if (!pRD && !pWR && !pEX) {
		return 0;
	}
	if (timeMs < 0) {
		timeMs = 0;
	}
	timeval tv = { timeMs / 1000, (timeMs % 1000) * 1000 };
#if defined(__linux__)
	++maxfd;
#endif
	return select(maxfd, pRD, pWR, pEX, &tv);
}

bool SocSet::IsSet(SocketWrapper &soc)
{
	if (m_pImp && soc.IsValid()) {
		return (FD_ISSET(soc.m_pRef->soc, &(m_pImp->set)) > 0);
	}
	return false;
}


}
