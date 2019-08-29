#include "ServerSideManager.h"

#include "../../Shared/thirdparty/zlib/zlib.h"
#include "Src/Utility/DDRMacro.h"
#include "Src/MTLib/GeneralLockGuard.hpp"

namespace DDRCloudService {
	
std::shared_ptr<google::protobuf::Message> ServerSideRouteManager::_rspListRoutes(google::protobuf::Message *pMsg)
{
	if (pMsg->GetTypeName() != "DDRCommProto.reqListRoutes") {
		return std::shared_ptr<google::protobuf::Message>();
	}
	auto ret = std::make_shared<RemoteRouteProto::rspListRoutes>();

	DDRMTLib::_lock_guard lg(true, m_gLoc, 10);
	if (!lg) {
		ret->set_ret(RemoteRouteProto::eTooBusy);
		return ret;
	}
	RemoteRouteProto::MultipleRouteInfo mri;
	for (auto &x : m_routes) {
		auto pRoute = mri.add_routes();
		pRoute->set_version(x->ver);
		pRoute->set_robotid(x->rID);
		pRoute->set_routename(x->routeName);
		pRoute->set_datetime(x->crTime);
		pRoute->set_description(x->description_u8);
		mri.add_downloadid(x->downloadID);
	}
	lg.unlock();

	std::string mri_ = mri.SerializeAsString();
	if (mri_.empty()) {
		ret->set_ret(RemoteRouteProto::eInternalErr);
		return ret;
	}
	ret->set_routerecords_sz(mri_.length());

	bool bDataOrg = false;
	ret->mutable_routerecords()->resize(mri_.length());
	uLongf _afterCompSz = (uLongf)mri_.length();
	if (Z_OK == compress((Bytef*)&((*ret->mutable_routerecords())[0]), &_afterCompSz,
		                 (const Bytef*)mri_.c_str(), (uLong)mri_.length())) {
		ret->mutable_routerecords()->resize(_afterCompSz);
		ret->set_ziptype(RemoteRouteProto::eZLib);
		bDataOrg = true;
	}
	if (!bDataOrg) {
		ret->set_routerecords(mri_.c_str(), mri_.length());
		ret->set_ziptype(RemoteRouteProto::eNoZip);
	}
	ret->set_ret(RemoteRouteProto::eOkay);
	return ret;
}

}
