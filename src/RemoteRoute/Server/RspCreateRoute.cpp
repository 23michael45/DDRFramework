#include "ServerSideManager.h"

#include "../../Shared/thirdparty/zlib/zlib.h"
#include "Src/Utility/DDRMacro.h"
#include "Src/Utility/CommonFunc.h"
#include "Src/MTLib/GeneralLockGuard.hpp"
#include "../Common/RandWrapper.h"
#include "ServerFunc.h"

namespace DDRCloudService {

extern const char g_InfoFileName[];
extern const int g_version;
extern const __int64 g_minRouteCrInterval; // in seconds

std::shared_ptr<google::protobuf::Message> ServerSideRouteManager::_rspCreateRoute(google::protobuf::Message *pMsg)
{
	if (pMsg->GetTypeName() != "DDRCommProto.reqCreateRoute") {
		return std::shared_ptr<google::protobuf::Message>();
	}
	auto ret = std::make_shared<DDRCommProto::rspCreateRoute>();
	ret->set_ret(DDRCommProto::eReqVoid);
	ret->set_uploadid(0);

	auto pp = (DDRCommProto::reqCreateRoute*)pMsg;
	auto &info = pp->routeinfo();
	if (info.version() < 10000 || info.version() > g_version ||
		info.robotid().empty() || info.routename().empty()) {
		return ret;
	}
	pp->mutable_routeinfo()->set_robotid(DDRSys::utf8_to_sysStr(pp->routeinfo().robotid().c_str()));
	pp->mutable_routeinfo()->set_routename(DDRSys::utf8_to_sysStr(pp->routeinfo().routename().c_str()));

	LevelLog(DDRFramework::Log::Level::INFO, "rspCreateRoute(ver=%d, rID='%s', routeName='%s')",
		     pp->routeinfo().version(), pp->routeinfo().robotid().c_str(),
		     pp->routeinfo().routename().c_str());

	DDRMTLib::_lock_guard lg(true, m_gLoc, 50);
	if (!lg) {
		ret->set_ret(DDRCommProto::eTooBusy);
		return ret;
	}
	bool bTooClose = false;
	auto rID_routes = m_RIDMapper.find(info.robotid());
	if (m_RIDMapper.end() != rID_routes) {
		for (size_t did : rID_routes->second) {
			if (abs(m_routes[did]->crTime - info.datetime()) <= g_minRouteCrInterval) {
				bTooClose = true;
				break;
			}
		}
	}
	if (bTooClose) {
		ret->set_ret(DDRCommProto::eUp_TooFrequent);
		return ret;
	}
	u64 uid, did;
	for (int i = 0; ; ++i) { // generating random UID and DID
		did = DDRSys::_rand_u64();
		uid = DDRSys::_rand_u64();
		if (0 != uid && m_UIDMapper.find(uid) == m_UIDMapper.end() &&
			m_DIDMapper.find(uid) == m_DIDMapper.end() &&
			0 != did && m_UIDMapper.find(did) == m_UIDMapper.end() &&
			m_DIDMapper.find(did) == m_DIDMapper.end() && uid != did) {
			break;
		}
		if (i >= 15) {
			ret->set_ret(DDRCommProto::eTooBusy);
			return ret;
		}
	}

	// create a new route
	if (!lg.lock(true, m_gLoc, 50)) {
		ret->set_ret(DDRCommProto::eTooBusy);
		return ret;
	}

	std::string fn = "Data/";
	fn += _fromU64(did);
	if (!DDRSys::createDir(fn.c_str())) {
		ret->set_ret(DDRCommProto::eTooBusy);
		return ret;
	}
	fn += g_InfoFileName;
	std::ofstream ofs(fn);
	if (!ofs.is_open()) {
		ret->set_ret(DDRCommProto::eTooBusy);
		return ret;
	}
	ofs << info.version() << "\n"
		<< info.robotid() << "\n"
		<< info.routename() << "\n"
		<< info.datetime() << "\n"
		<< uid << "\n" << did << "\n0"
		<< std::endl;
	ofs.close();
	std::shared_ptr<OneRoute> pNewRoute = std::make_shared<OneRoute>();
	pNewRoute->ver = info.version();
	pNewRoute->rID = info.robotid();
	pNewRoute->routeName = info.routename();
	pNewRoute->crTime = info.datetime();
	pNewRoute->uploadID = uid;
	pNewRoute->downloadID = did;
	m_UIDMapper[pNewRoute->uploadID] = (int)m_routes.size();
	m_DIDMapper[pNewRoute->downloadID] = (int)m_routes.size();
	m_RIDMapper[pNewRoute->rID].emplace_back(m_routes.size());
	m_routes.emplace_back(pNewRoute);
	ret->set_ret(DDRCommProto::eOkay);
	ret->set_uploadid(uid);
	return ret;
}

}
