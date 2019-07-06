#ifndef __DDR_CLOUD_SERVICE_LIB_DOWNLOADER_SIDE_MANAGER_H_INCLUDED__
#define __DDR_CLOUD_SERVICE_LIB_DOWNLOADER_SIDE_MANAGER_H_INCLUDED__

#include <algorithm>
#include <vector>
#include <string>
#include <memory>
#include "RouteInfo.h"
#include "proto/RemoteRoute.pb.h"

namespace DDRCloudService {

#ifndef u64
typedef unsigned long long u64;
#endif

enum {
	NO_EXCEPTION = 0,
	INTERNAL_ERROR = -1, // file I/O error or route not prepared
	SERVER_RESP_INVALID = 1, // server not responding properly
	FAILED_TOO_MANY_TIMES = 2, // server too busy
	ROUTE_CREATION_DENIED = 3, // request to create a route too frequently
	ID_INVALID = 4, // upload ID not recognizable by server
	DATA_ERROR = 5 // too big, or not consistent (especially for compression)
};

class DownloadManagerBase {
public:
	DownloadManagerBase(const char *pWorkingDir, const routeInfo &ri);
	~DownloadManagerBase();

	// -1 - exception;
	// 0 - obtaining file list;
	// 1 - downloading;
	// 2 - finished downloading;
	int GetStage() const;

	// NO_EXCEPTION = 0,
	// INTERNAL_ERROR = -1, // file I/O error or route not prepared
	// SERVER_RESP_INVALID = 1, server not responding properly
	// FAILED_TOO_MANY_TIMES = 2, server too busy
	// ROUTE_CREATION_DENIED = 3, request to create a route too frequently
	// ID_INVALID = 4, upload ID not recognizable by server
	// DATA_ERROR = 5, too big, or not consistent (especially for compression)
	int GetException() const;

	std::shared_ptr<google::protobuf::Message> GetNextMsg();
	std::shared_ptr<google::protobuf::Message> Feed(std::shared_ptr<google::protobuf::Message>);

protected:
	virtual bool IsFileNecessary(const char *pFileName);

protected:
	std::string m_dir;
	routeInfo m_RI;
	std::shared_ptr<google::protobuf::Message> m_msg2Snd;

	// -1 - exception;
	// 0 - ready to download a remote route, trying to list files;
	// 1 - downloading...
	// 2 - finished downloading
	int m_stage;
	// 0 - no exception;
	// 1 - route with the same name already existing (robotID/crTime/version conflict(s));
	// 2 - downloader ID denied by server; 3 - server response invalid;
	// 4 - server file decompression failed;
	// 5 - server timeout;
	// 9 - failure count exceeds threshold;
	// -1 - internal error
	int m_except;

	int m_nFailCnt;

	struct OneExistingFile {
		std::string name;
		__int64 mtime;
		size_t sz, pos;
	};
	std::vector<OneExistingFile> m_existingFiles;
	int m_fListPtr;
	struct RemoteFileStat {
		std::string name;
		__int64 mtime;
		size_t sz, pos;
	};
	std::vector<RemoteFileStat> m_downloableFiles;
	
	void adjustDownloadableFiles();
	void adjustFilePtr();
	int prepareNextDownloadReq(); // 0 - success; 1 - finished; -1 - error
	void saveRec();
	void _rsp_listFiles(const DDRCommProto::rspListFiles *pRsp);
	void _rsp_downloadFiles(const DDRCommProto::rspDownloadFiles *pRsp);
};

}

#endif // __DDR_CLOUD_SERVICE_LIB_DOWNLOADER_SIDE_MANAGER_H_INCLUDED__

