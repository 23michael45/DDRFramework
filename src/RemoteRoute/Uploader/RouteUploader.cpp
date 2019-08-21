#include "RouteUploader.h"

//#include <random>
#include <fstream>
#include <chrono>

#include "src/MTLib/BkgThread.h"
#include "src/Network/TcpClientBase.h"
#include "RobotSideManager.h"
#include "RUDispatcher.h"
#include "UploaderClient.h"

namespace DDRCloudService {

extern int g_CurrentRouteVersion;
int g_MaxDisRetrials = 5;

struct RouteUploaderARG
{
	std::string rID;
	std::string routeName;
	std::string serAddr;
	std::string serPort;
	bool *pbSuccess;
	bool bForceUpdate;
	RouteUploaderARG(const char *_rid, const char *_rname,
		             const char *_addr, const char *_port,
		             bool *_pbSucc, bool _bForce) :
		rID(_rid), routeName(_rname), serAddr(_addr), serPort(_port), pbSuccess(_pbSucc), bForceUpdate(_bForce)
	{}
};

static void _thrFunc(void *ptr, bool *pbQuit)
{
#if 0
	std::shared_ptr<RouteUploaderARG> pArg_((RouteUploaderARG*)ptr);
	std::this_thread::sleep_for(std::chrono::seconds(5));
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> dist(0, 1);
	*(pArg_->pbSuccess) = (dist(mt) > 0);
	return;
#endif

	std::shared_ptr<RouteUploaderARG> pArg((RouteUploaderARG*)ptr);
	auto pClt = std::make_shared<RUTcpClient>(pArg->rID, pArg->routeName,
		                                      pArg->serAddr, pArg->serPort,
		                                      pArg->bForceUpdate);
	if (pClt->IsRSRMWrong()) {
		return;
	}
	pClt->Start(1);
	pClt->Connect(pArg->serAddr, pArg->serPort);
	if (pArg->pbSuccess) {
		*(pArg->pbSuccess) = false;
	}

	while (!pbQuit || !(*pbQuit)) {
		if (pClt->IsRSRMWrong() || pClt->GetSecondsSinceLastRcv() > 15) {
			pClt->Try2reconnect();
			break;
		}
		if (pClt->IsStopped()) {
			//LevelLog(DDRFramework::Log::INFO, "Route uploading (%s, %s) not finished...\n", pArg->rID, pArg->routeName);
			break;
		}
		if (pClt->IsUploadingFinished()) {
			//LevelLog(DDRFramework::Log::INFO, "SUCCESSFULLY uploaded the route (%s, %s)!\n", pArg->rID, pArg->routeName);
			if (pArg->pbSuccess) {
				*(pArg->pbSuccess) = true;
			}
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	pClt->Stop();
}

unsigned int StartUploadingRoute(const char *robotID,
	                             const char *routeName,
	                             const char *serverAddr,
	                             const char *serverPort,
	                             bool *pbSuccess, bool bForceUpdate)
{
	auto arg = new RouteUploaderARG(robotID, routeName, serverAddr, serverPort, pbSuccess, bForceUpdate);
	unsigned int id;
	if (DDRMTLib::CreateBkgThread(_thrFunc, arg, &id)) {
		return id;
	} else {
		return -1;
	}
}

}