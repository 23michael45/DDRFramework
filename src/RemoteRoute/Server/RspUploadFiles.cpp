#include "ServerSideManager.h"

#include "../../Shared/thirdparty/zlib/zlib.h"
#include "Src/Utility/DDRMacro.h"
#include "Src/MTLib/GeneralLockGuard.hpp"
#include "Src/Utility/CommonFunc.h"
#include "ServerFunc.h"

namespace DDRCloudService {

extern const size_t g_MaxDataSliceBytes;
extern const char g_CommentFileName[];

std::shared_ptr<google::protobuf::Message> ServerSideRouteManager::_rspUploadFiles(google::protobuf::Message *pMsg)
{
	if (pMsg->GetTypeName() != "DDRCommProto.reqUploadFiles") {
		return std::shared_ptr<google::protobuf::Message>();
	}
	auto ret = std::make_shared<RemoteRouteProto::rspUploadFiles>();
	ret->set_ret(RemoteRouteProto::eReqVoid);
	auto pReq = (RemoteRouteProto::reqUploadFiles*)pMsg;
	ret->set_reqguid(pReq->reqguid());

	if (pReq->filenames_size() <= 0 ||
		pReq->filenames_size() != pReq->filesz_size() ||
		pReq->filenames_size() != pReq->filemtimes_size() ||
		pReq->filenames_size() != pReq->file_pos0_size() ||
		pReq->filenames_size() != pReq->file_pos1_size() ||
		0 == pReq->content_len() || 0 == pReq->contents().length()) {
		return ret;
	}
	if ((size_t)pReq->content_len() > g_MaxDataSliceBytes) {
		ret->set_ret(RemoteRouteProto::eUp_TooBig);
		return ret;
	}
	__int64 accSz = 0;
	for (int i = 0; i < pReq->filenames_size(); ++i) {
		if (pReq->file_pos0()[i] < 0 ||
			pReq->file_pos0()[i] >= pReq->file_pos1()[i] ||
			pReq->file_pos1()[i] > pReq->filesz()[i]) {
			return ret;
		}
		accSz += pReq->file_pos1()[i] - pReq->file_pos0()[i];
	}
	if (accSz != pReq->content_len()) {
		return ret;
	}

	LevelLog(DDRFramework::Log::Level::INFO, "reqUploadFiles(uid=%llu, #files=%d, #bytes=%d",
		     pReq->uploadid(), pReq->filenames_size(), pReq->content_len());
	DDRMTLib::_lock_guard lg(true, m_gLoc, 50);
	if (!lg) {
		ret->set_ret(RemoteRouteProto::eTooBusy);
		return ret;
	}
	auto pInd = m_UIDMapper.find(pReq->uploadid());
	if (m_UIDMapper.end() == pInd) {
		ret->set_ret(RemoteRouteProto::eIDInvalid);
		return ret;
	}
	auto ele = m_routes[pInd->second];

	if (!lg.lock(false, ele->loc, 500)) {
		ret->set_ret(RemoteRouteProto::eTooBusy);
		return ret;
	}
	const char *pBuf = nullptr;
	size_t nBufLen = 0;
	std::vector<char> oriContent;
	if (RemoteRouteProto::eZLib == pReq->ziptype()) {
		oriContent.resize(pReq->content_len());
		uLongf dstLen = (uLongf)pReq->content_len();
		if (Z_OK != uncompress((Bytef*)&oriContent[0], &dstLen,
			                   (const Bytef*)pReq->contents().c_str(),
			                   (uLong)pReq->contents().length()) ||
			                   (__int64)dstLen != pReq->content_len()) {
			ret->set_ret(RemoteRouteProto::eUp_DataError);
			return ret;
		}
		pBuf = &oriContent[0];
		nBufLen = (size_t)pReq->content_len();
	} else if (RemoteRouteProto::eNoZip == pReq->ziptype()) {
		if (pReq->content_len() != (__int64)pReq->contents().length()) {
			ret->set_ret(RemoteRouteProto::eUp_DataError);
			return ret;
		}
		pBuf = pReq->contents().c_str();
		nBufLen = (size_t)pReq->content_len();
	}

	size_t bufPos = 0;
	std::string str, fn;
	str = "Data/" + _fromU64(ele->downloadID) + "/";
	for (int i = 0; i < (int)pReq->filenames_size(); ++i) {
		size_t pos0 = pReq->file_pos0()[i];
		size_t pos1 = pReq->file_pos1()[i];
		bool bExist = false;
		int tarFileIndex = ele->findFileName(pReq->filenames()[i].c_str(), bExist);
		fn = str + pReq->filenames()[i];
		fn = DDRSys::utf8_to_sysStr(fn.c_str());
		int nRet = _writeFile(&fn[0], (bExist ? ele->files[tarFileIndex].sz : 0), pReq->filesz()[i],
			                  pBuf + bufPos, pos0, pos1 - pos0);
		if (0 == nRet) { // successful
			DDRSys::setModTime(fn.c_str(), pReq->filemtimes()[i]);
			if (bExist) {
				ele->files[tarFileIndex].sz = pReq->filesz()[i];
				ele->files[tarFileIndex].pos = pos1;
				ele->files[tarFileIndex].mtime = pReq->filemtimes()[i];
			} else {
				ele->insertFile(tarFileIndex, pReq->filenames()[i].c_str(), pReq->filesz()[i], pos1, pReq->filemtimes()[i]);
			}
			if (0 == fn.compare(g_CommentFileName) && (__int64)pos1 == pReq->filesz()[i]) {
				ele->updateDescription();
			}
		} else { // file size matching error (-2) or failure to open or write the file (-1)
			if (bExist) { // erase the problematic file
				ele->eraseFile(tarFileIndex);
			}
		}
		bufPos += pos1 - pos0;
	}
	ele->save();

	ret->set_ret(RemoteRouteProto::eOkay);
	return ret;
}

}

