#ifndef __DDR_LOCAL_ROUTE_UPLOADER_H_INCLUDED__
#define __DDR_LOCAL_ROUTE_UPLOADER_H_INCLUDED__

namespace DDRCloudService {

// return value is an internal thread ID that can later
// be used in Wait4BkgThread()
unsigned int StartUploadingRoute(const char *robotID,
		                         const char *routeName,
		                         const char *serverAddr,
		                         const char *serverPort,
	                             bool *pbSuccess = nullptr,
	                             bool bForceUpdate = false);

}

#endif // __DDR_LOCAL_ROUTE_UPLOADER_H_INCLUDED__
