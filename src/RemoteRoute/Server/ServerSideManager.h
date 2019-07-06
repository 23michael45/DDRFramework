#ifndef __DDR_CLOUD_SERVICE_LIB_SERVER_SIDE_ROUTE_MANAGER_H_INCLUDED__
#define __DDR_CLOUD_SERVICE_LIB_SERVER_SIDE_ROUTE_MANAGER_H_INCLUDED__

#include <vector>
#include <string>
#include <map>
#include <memory>
#include <shared_mutex>
#include "src/Utility/Singleton.h"
#include "proto/RemoteRoute.pb.h"

namespace DDRCloudService {

#ifndef u64
typedef unsigned long long u64;
#endif

class ServerSideRouteManager : public DDRFramework::CSingleton<ServerSideRouteManager>
{
public:
	ServerSideRouteManager();
	~ServerSideRouteManager();

	std::shared_ptr<google::protobuf::Message> Feed(std::shared_ptr<google::protobuf::Message> pMsg);

private:
	struct OneFileRec {
		std::string fn;
		size_t sz, pos;
		__int64 mtime;
	};
	struct OneRoute {
		u64 uploadID;
		u64 downloadID;
		std::shared_timed_mutex loc;

		std::string description_u8;

		int ver;
		std::string rID;
		std::string routeName;
		__int64 crTime;
		std::vector<OneFileRec> files;
		int findFileName(const char *pFileName, bool &bExist);
		void sortFiles();
		void insertFile(int index, const char *pFileName, size_t fSz, size_t pos, __int64 mtime);
		void eraseFile(int index);
		void updateDescription();

		bool save();
	};
	std::shared_timed_mutex m_gLoc;
	std::vector<std::shared_ptr<OneRoute>> m_routes;
	std::map<u64, size_t> m_UIDMapper, m_DIDMapper;
	std::map<std::string, std::vector<size_t>> m_RIDMapper;

	std::map<std::string, std::function<std::shared_ptr<google::protobuf::Message>(google::protobuf::Message*)>> m_funcMapper;

	void load();
	void save();

	std::shared_ptr<google::protobuf::Message> _rspCreateRoute(google::protobuf::Message *pMsg);
	std::shared_ptr<google::protobuf::Message> _rspUploadFiles(google::protobuf::Message *pMsg);
	std::shared_ptr<google::protobuf::Message> _rspListRoutes(google::protobuf::Message *pMsg);
	std::shared_ptr<google::protobuf::Message> _rspListFiles(google::protobuf::Message *pMsg);
	std::shared_ptr<google::protobuf::Message> _rspDownloadFiles(google::protobuf::Message *pMsg);
};

}

#endif // __DDR_CLOUD_SERVICE_LIB_SERVER_SIDE_ROUTE_MANAGER_H_INCLUDED__
