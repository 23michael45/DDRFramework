#ifndef __DDR_CLOUD_SERVICE_LIB_ROUTE_INFO_H_INCLUDED__
#define __DDR_CLOUD_SERVICE_LIB_ROUTE_INFO_H_INCLUDED__

#include <string>

namespace DDRCloudService {

struct routeInfo
{
	int version;
	std::string robotID;
	std::string routeName;
	__int64 crTime; // seconds since EPOCH
	std::string description;
	unsigned __int64 downloadID;
};

}

#endif // __DDR_CLOUD_SERVICE_LIB_ROUTE_INFO_H_INCLUDED__
