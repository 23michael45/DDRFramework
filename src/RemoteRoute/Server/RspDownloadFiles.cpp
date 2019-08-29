#include "ServerSideManager.h"

#include "../../Shared/thirdparty/zlib/zlib.h"
#include "Src/Utility/DDRMacro.h"
#include "Src/Utility/CommonFunc.h"
#include "Src/MTLib/GeneralLockGuard.hpp"
#include "ServerFunc.h"

namespace DDRCloudService {

extern const size_t g_MaxDataSliceBytes;

std::shared_ptr<google::protobuf::Message> ServerSideRouteManager::_rspDownloadFiles(google::protobuf::Message *pMsg)
{
	if (pMsg->GetTypeName() != "DDRCommProto.reqDownloadFiles") {
		return std::shared_ptr<google::protobuf::Message>();
	}
	auto pReq = (RemoteRouteProto::reqDownloadFiles*)pMsg;
	auto ret = std::make_shared<RemoteRouteProto::rspDownloadFiles>();
	ret->set_ret(RemoteRouteProto::eReqVoid);
	ret->set_reqguid(pReq->reqguid());
	if (pReq->filenames_size() != pReq->file_pos0_size() ||
		pReq->file_pos0_size() != pReq->file_pos1_size()) {
		return ret;
	}

	size_t accSz = 0;
	for (int i = 0; i < (int)pReq->filenames_size(); ++i) {
		if (pReq->file_pos0()[i] >= pReq->file_pos1()[i]) {
			return ret;
		}
		accSz += pReq->file_pos1()[i] - pReq->file_pos0()[i];
		if (accSz > g_MaxDataSliceBytes) {
			ret->set_ret(RemoteRouteProto::eDown_TooBig);
			return ret;
		}
	}
	
	DDRMTLib::_lock_guard lg(true, m_gLoc, 50);
	if (!lg) {
		ret->set_ret(RemoteRouteProto::eTooBusy);
		return ret;
	}

	auto pp = m_DIDMapper.find(pReq->downloadid());
	if (m_DIDMapper.end() == pp) {
		ret->set_ret(RemoteRouteProto::eIDInvalid);
		return ret;
	}
	auto ele = m_routes[pp->second];
	if (!lg.lock(true, ele->loc, 500)) {
		ret->set_ret(RemoteRouteProto::eTooBusy);
		return ret;
	}

	std::vector<char> oriContent;
	size_t compSz = 0, nonCompSz = 0;

	std::string str = "Data/" + _fromU64(pReq->downloadid()) + "/", fn;
	for (int i = 0; i < (int)pReq->filenames_size(); ++i) {
		if (pReq->file_pos0()[i] >= pReq->file_pos1()[i]) {
			ret->mutable_filenames()->Clear();
			ret->mutable_filesz()->Clear();
			ret->mutable_file_pos0()->Clear();
			ret->mutable_file_pos1()->Clear();
			ret->set_ret(RemoteRouteProto::eReqVoid);
			return ret;
		}
		bool bExist = false;
		int ind = ele->findFileName(pReq->filenames()[i].c_str(), bExist);
		if (!bExist) {
			ret->mutable_filenames()->Clear();
			ret->mutable_filesz()->Clear();
			ret->mutable_file_pos0()->Clear();
			ret->mutable_file_pos1()->Clear();
			ret->set_ret(RemoteRouteProto::eDown_FileNotExisting);
			return ret;
		}
		if ((size_t)pReq->file_pos1()[i] > ele->files[ind].pos) {
			ret->mutable_filenames()->Clear();
			ret->mutable_filesz()->Clear();
			ret->mutable_file_pos0()->Clear();
			ret->mutable_file_pos1()->Clear();
			ret->set_ret(RemoteRouteProto::eDown_FilePosError);
			return ret;
		}
		fn = str + pReq->filenames()[i];
		fn = DDRSys::utf8_to_sysStr(fn.c_str());
		std::transform(fn.begin(), fn.end(), fn.begin(), ::tolower);
		size_t bufPos = oriContent.size(), fSz;
		oriContent.resize(bufPos + (size_t)pReq->file_pos1()[i] - (size_t)pReq->file_pos0()[i]);
		int nRet = _readFile(fn.c_str(), &oriContent[bufPos], (size_t)pReq->file_pos0()[i],
			                 (size_t)pReq->file_pos1()[i], &fSz);
		if (-1 == nRet) {
			ret->mutable_filenames()->Clear();
			ret->mutable_filesz()->Clear();
			ret->mutable_file_pos0()->Clear();
			ret->mutable_file_pos1()->Clear();
			ret->set_ret(RemoteRouteProto::eInternalErr);
			return ret;
		} else if (-2 == nRet) {
			ret->mutable_filenames()->Clear();
			ret->mutable_filesz()->Clear();
			ret->mutable_file_pos0()->Clear();
			ret->mutable_file_pos1()->Clear();
			ret->set_ret(RemoteRouteProto::eDown_FilePosError);
			return ret;
		}

		ret->add_filenames(pReq->filenames()[i]);
		ret->add_filesz(fSz);
		ret->add_file_pos0(pReq->file_pos0()[i]);
		ret->add_file_pos1(pReq->file_pos1()[i]);
		const char *pC = pReq->filenames()[i].c_str();
		if (_isCompressable(pReq->filenames()[i].c_str())) {
			compSz += (size_t)pReq->file_pos1()[i] - (size_t)pReq->file_pos0()[i];
		} else {
			nonCompSz += (size_t)pReq->file_pos1()[i] - (size_t)pReq->file_pos0()[i];
		}
	}

	bool bDataOrg = false;
	if (compSz > nonCompSz * 2) {
		ret->mutable_contents()->resize(oriContent.size());
		uLongf compSz = (uLongf)oriContent.size();
		if (Z_OK == compress((Bytef*)&((*ret->mutable_contents())[0]), &compSz,
			                 (const Bytef*)&oriContent[0], (uLong)oriContent.size())) {
			ret->mutable_contents()->resize(compSz);
			ret->set_ziptype(RemoteRouteProto::eZLib);
			bDataOrg = true;
		}
	}
	if (!bDataOrg) {
		ret->set_contents(&oriContent[0], oriContent.size());
		ret->set_ziptype(RemoteRouteProto::eNoZip);
	}
	ret->set_content_len((__int64)oriContent.size());
	ret->set_ret(RemoteRouteProto::eOkay);
	return ret;
}

}

