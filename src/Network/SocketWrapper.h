#ifndef __DDRCOMMLIB_SOCKET_WRAPPER_H_INCLUDED__
#define __DDRCOMMLIB_SOCKET_WRAPPER_H_INCLUDED__

#include <vector>

#ifndef BYTE
typedef unsigned char BYTE;
#endif

namespace DDRCommLib {

class SocSet;
class MIMCServer;
class MIMCClient;

struct Addr_IPV4 {
	unsigned long ip;
	unsigned short port;
};

class SocketWrapper
{
public:
	/* type:
	0 - unspecified;
	1 - UDP socket;
	2 - TCP socket;
	*/
	SocketWrapper(int type = 0);
	SocketWrapper(const SocketWrapper &oriSW);
	SocketWrapper(SocketWrapper &&oriSW);
	SocketWrapper& operator= (const SocketWrapper &oriSW);
	SocketWrapper& operator= (SocketWrapper &&oriSW);
	~SocketWrapper();
	void Release();
	void Close();
	bool IsValid() const;

	// set socket's buffer sizes
	bool SetBuffer(int sndBufSz, int rcvBufSz);
	// set socket to block/non-blocking
	bool SetBlocking(bool bBlocking);
	// set the address this socket is bound to reusable or not
	bool SetAddrReuse(bool bReusable);
	// bind socket to target address
	bool Bind(unsigned long tarIP_BE, unsigned short tarPort_BE);
	// for bound (explicitly or implicitly), get its address
	bool GetBoundAddr(unsigned long *pIP_BE, unsigned short *pPort_BE);
	// get error (if any) using getsockopt()
	bool HasError();

	//********************************** UDP functions **********************************//
	// initialize as a UDP socket
	bool UDP_Init();
	// send one datagram to target address
	int UDP_SendTo(const char *pData, int nDataLen,
		           unsigned long tarIP_BE, unsigned short tarPort_BE);
	// try to receive a datagram (with peer's address info)
	int UDP_RecvFrom(char *pBuf, int nBufLen,
				     unsigned long *pPeerIP_BE, unsigned short *pPeerPort_BE);
	bool UDP_JoinMulticast(unsigned long srcIP_BE,
		                   unsigned long mcIP_BE, unsigned short mcPort_BE);

	//********************************** TCP functions **********************************//
	// initialized as a TCP socket
	bool TCP_Init();
	// send bytes
	int TCP_Send(const char *pData, int nDataLen);
	// receive bytes
	int TCP_Recv(char *pBuf, int nBufLen);
	// listen (as a server entrance socket)
	bool TCP_Listen();
	// accept (as a server) connection to pTarSoc,
	// and filled pointer (if non-null) with peer address
	bool TCP_Accept(SocketWrapper *pTarSoc,
		            unsigned long *pPeerIP_BE,
					unsigned short *pPeerPort_BE);
	// connect to target address
	bool TCP_Connect(unsigned long tarIP_BE, unsigned short tarPort_BE);

	friend class SocSet;
	friend class MIMCServer;
	friend class MIMCClient;

private:
	struct __refInfo;
	__refInfo *m_pRef;
};

bool ListAllInterfaces(std::vector<unsigned long> &IPVec_n);

class MIMCServer
{
public:
	MIMCServer();
	void Refresh();
	bool IsValid() const;
	bool ReInit();
	void Close();
	~MIMCServer();
	bool SetSockOpt(int level, int optname, const char *optval, int optlen);
	bool SetBuffer(int sndBufSz, int rcvBufSz);
	bool SetMCTarget(unsigned long tarMCIP_BE, unsigned short tarMCPort_BE);
	bool Broadcast(const char *pBuf, int nBufLen);
	// if an address used by this multicasting server
	bool IsFromCurServer(unsigned long srcIP_BE, unsigned short srcPort_BE) const;
private:
	struct _IMPL;
	_IMPL *m_pImpl;
};

class MIMCClient
{
public:
	MIMCClient();
	void Close();
	bool IsValid() const;
	bool ReInit();
	~MIMCClient();
	void AddToSet(SocSet *pSet);
	bool IsSet(SocSet *pSet);
	bool SetBuffer(int sndBufSz, int rcvBufSz);
	bool SetSockOpt(int level, int optname, const char *optval, int optlen);
	bool JoinGroup(unsigned long tarMCIP_BE, unsigned short tarMCPort_BE);
	int RecvFrom(char *pBuf, int nBufLen,
				 unsigned long *pPeerIP_BE, unsigned short *pPeerPort_BE);
private:
	struct _IMPL;
	_IMPL *m_pImpl;
};

int Select(SocSet *pRdSet, SocSet *pWrSet, SocSet *pExSet, int timeMs);

class SocSet
{
public:
	SocSet();
	void ClearAll();
	void Clear(SocketWrapper &soc);
	bool Add2Set(SocketWrapper &soc);
	bool IsSet(SocketWrapper &soc);
	friend int Select(SocSet*, SocSet*, SocSet*, int);
private:
	struct __IMPL;
	__IMPL *m_pImp;
};

}

#endif // __DDR_COMM_SOCKET_WRAPPER_H_INCLUDED__
