#ifndef __DDR_CLOUD_SERVICE_LIB_ROBOT_SIDE_ROUTE_MANAGER_H_INCLUDED__
#define __DDR_CLOUD_SERVICE_LIB_ROBOT_SIDE_ROUTE_MANAGER_H_INCLUDED__

#include <vector>
#include <string>
#include <memory>
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

class RobotSideRouteManager {
public:
	RobotSideRouteManager(const char *pRouteName);
	void Reset();
	~RobotSideRouteManager();

	// -1 - invalid;
	// 0 - ready to create a remote route, obtaining UpID;
	// 1 - uploading (requiring server to list its current files first);
	// 2 - finished uploading
	// -2 - exception
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
	std::string m_dir;

	// -1 - invalid;
	// 0 - ready to create a remote route, obtaining UpID;
	// 1 - uploading (reqiring server to list its current files first);
	// 2 - finished uploading
	// -2 - exception
	int m_stage;
	// 0 - no exception; 1 - request inconsistent;
	// 2 - creation of route denied for too frequent uploading;
	// 3 - uploader ID denied; 4 - file decompression failed;
	// 9 - failure count exceeds threshold;
	// -1 - internal error
	int m_except;

	int m_uploadFailCnt;

	int m_ver;
	std::string m_rID;
	std::string m_routeName;
	std::string m_str;
	long long m_cTime;

	u64 m_uploadID;

	std::shared_ptr<google::protobuf::Message> m_msg2Snd;

	struct OneFileProg {
		bool bCompressable;
		std::string name;
		long long mtime;
		size_t sz, pos;
		void resetCompressable();
	};
	std::vector<OneFileProg> m_uploadableFiles;
	int m_fListPtr;
	struct RemoteFileStat {
		std::string name;
		long long mtime;
		size_t sz, pos;
	};
	std::vector<RemoteFileStat> m_remoteSavedFiles;
	std::vector<char> m_oriContent;

	bool listUploadableFiles();
	bool processRemoteList(const RemoteRouteProto::rspListFiles *pMsg_RspListFile);
	void adjustUploadableFiles();
	int prepareNextSliceUpload(); // 0 - success; 1 - finished; -1 - error
	bool readFile(const char *pRelativeFN, size_t sPos, size_t len,
		          std::vector<char> &content);

	void _rsp_createRoute(const RemoteRouteProto::rspCreateRoute *pRsp);
	void _rsp_uploadFiles(const RemoteRouteProto::rspUploadFiles *pRsp);
	void _rsp_listFiles(const RemoteRouteProto::rspListFiles *pRsp);
};

}

#endif // __DDR_CLOUD_SERVICE_LIB_ROBOT_SIDE_ROUTE_MANAGER_H_INCLUDED__

