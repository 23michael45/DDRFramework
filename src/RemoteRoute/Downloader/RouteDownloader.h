#ifndef __DDR_REMOTE_ROUTE_DOWNLOADER_H_INCLUDED__
#define __DDR_REMOTE_ROUTE_DOWNLOADER_H_INCLUDED__

namespace DDRCloudService {

// list all routes (blocking call)
bool ListRoutes(const char *pServerIP, const char *pServerPort);

void RefreshLister();

// try to get info of the next route
bool GetNextRouteInfo(int *pVersion, const char **ppRobotID,
	                  const char **ppRouteName, __int64 *pCrTime,
	                  const char **ppDescription, unsigned __int64 *pDownID);

// return value is an internal thread ID that can later
// be used in Wait4BkgThread()
unsigned int StartDownloadingRoute(const char *pServerIP, const char *pServerPort, const char *pWorkingDir,
	                               int version, const char *pRobotID, const char *pRouteName, __int64 crTime,
	                               unsigned __int64 downloadID);

}

#endif // __DDR_LOCAL_ROUTE_UPLOADER_H_INCLUDED__
