#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include "../../MTLib/BkgThread.h"
#include "RouteInfo.h"
#include "RouteLister.hpp"
#include "DownloadManagerBase.h"
#include "DownloaderClient.h"

namespace DDRCloudService {

class RouteDownloader
{
public:
	RouteDownloader() : m_infoPtr(-1) {}
	bool List(const char *pServerIP, const char *pServerPort)
	{
		std::lock_guard<std::mutex> lk(m_loc);
		auto rl = std::make_shared<RouteLister>(pServerIP, pServerPort, &m_info);
		rl->Start();
		while (1) {
			int curSt = rl->GetCurStage();
			if (0 == curSt) {
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			} else if (2 == curSt) {
				m_infoPtr = 0;
				return true;
			} else if (-1 == curSt) {
				return false;
			}
		}
		rl->Stop();
	}
	void RefreshLister()
	{
		std::lock_guard<std::mutex> lk(m_loc);
		m_infoPtr = 0;
	}
	bool GetNext(int *pVersion, const char **ppRobotID,
		         const char **ppRouteName, __int64 *pCrTime,
		         const char **ppDescription, unsigned __int64 *pDownID)
	{
		std::lock_guard<std::mutex> lk(m_loc);
		if (m_infoPtr < (int)m_info.size()) {
			auto &x = m_info[m_infoPtr++];
			if (pVersion) {
				*pVersion = x.version;
			}
			if (ppRobotID) {
				*ppRobotID = x.robotID.c_str();
			}
			if (ppRouteName) {
				*ppRouteName = x.routeName.c_str();
			}
			if (pCrTime) {
				*pCrTime = x.crTime;
			}
			if (ppDescription) {
				*ppDescription = x.description.c_str();
			}
			if (pDownID) {
				*pDownID = x.downloadID;
			}
			return true;
		}
		return false;
	}

private:
	std::vector<routeInfo> m_info;
	int m_infoPtr;
	std::mutex m_loc;
} g_Lister;

bool ListRoutes(const char *pServerIP, const char *pServerPort)
{
	return g_Lister.List(pServerIP, pServerPort);
}

void RefreshLister()
{
	g_Lister.RefreshLister();
}

// try to get info of the next route
bool GetNextRouteInfo(int *pVersion, const char **ppRobotID,
	                  const char **ppRouteName, __int64 *pCrTime,
	                  const char **ppDescription, unsigned __int64 *pDownID)
{
	return g_Lister.GetNext(pVersion, ppRobotID, ppRouteName, pCrTime, ppDescription, pDownID);
}

class myMgt : public DownloadManagerBase
{
public:
	myMgt(const char *pWorkingDir, const routeInfo &ri) : DownloadManagerBase(pWorkingDir, ri) {}
protected:
	bool IsFileNecessary(const char *pFileName) override
	{
		return (0 != strncmp(pFileName, "left/", 5) &&
			    0 != strncmp(pFileName, "right/", 6) &&
			    0 != strncmp(pFileName, "lidarscans/", 11));
	}
};

struct _ArgStruct
{
	std::string serIP, serPort;
	std::string workingDir;
	int version;
	std::string rID, routeName;
	__int64 crTime;
	unsigned __int64 downloadID;
};

static void _thrFunc(void *ptr, bool *pbQuit)
{
	std::shared_ptr<_ArgStruct> pArgs((_ArgStruct*)ptr);
	routeInfo ri;
	ri.version = pArgs->version;
	ri.robotID = pArgs->rID;
	ri.routeName = pArgs->routeName;
	ri.crTime = pArgs->crTime;
	ri.downloadID = pArgs->downloadID;
	auto pMgt = std::make_shared<myMgt>(pArgs->workingDir.c_str(), ri);
	if (pMgt->GetStage() < 0) {
		return;
	}
	auto pDClt = std::make_shared<RDTcpClient>(pMgt);
	if (!pDClt->IsValid()) {
		return;
	}
	pDClt->Start(1);
	pDClt->Connect(pArgs->serIP, pArgs->serPort);

	while (!pbQuit || !(*pbQuit)) {
		if (!pDClt->IsValid() || pDClt->GetSecondsSinceLastRcv() > 15) {
			break;
		}
		if (pDClt->IsDownloadingFinished()) {
			LevelLog(DDRFramework::Log::INFO, "SUCCESSFULLY downloaded the route!\n");
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	pDClt->Stop();
}

unsigned int StartDownloadingRoute(const char *pServerIP, const char *pServerPort, const char *pWorkingDir,
	                               int version, const char *pRobotID, const char *pRouteName, __int64 crTime,
	                               unsigned __int64 downloadID)
{
	_ArgStruct *pArgs = new _ArgStruct;
	pArgs->serIP = pServerIP;
	pArgs->serPort = pServerPort;
	pArgs->workingDir = pWorkingDir;
	pArgs->version = version;
	pArgs->rID = pRobotID;
	pArgs->routeName = pRouteName;
	pArgs->crTime = crTime;
	pArgs->downloadID = downloadID;
	unsigned int id;
	if (DDRMTLib::CreateBkgThread(_thrFunc, pArgs, &id)) {
		return id;
	} else {
		return -1;
	}
}

}

