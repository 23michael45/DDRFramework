#include "ServerSideManager.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include "Src/Utility/CommonFunc.h"
#include "Src/MTLib/GeneralLockGuard.hpp"
#include "ServerFunc.h"

namespace DDRCloudService  {

extern const int g_version = 10001;
extern const __int64 g_minRouteCrInterval = 300; // in seconds
extern const size_t g_MaxDataSliceBytes = 1024 * 1024;
extern const char g_InfoFileName[] = "/__INFO__.ini";
extern const char g_CommentFileName[] = "/__DESCR__.ini";

/*
Version 10001 Info.ini (server side)
[VersionNo.] (10001)
[RobotID]
[routeName]
[creationTime]
[uploadID]
[downloadID]
[# of files] (N)
[file #1: [name] [size] [pos] [mtime]] ...
[file #N: [name] [size] [pos] [mtime]]
*/

void ServerSideRouteManager::load()
{
	std::string stName = "Data/";
	void *pH = DDRSys::findAllFiles_Open(stName.c_str(), 0x02, false);
	const char *pDirName;
	std::string fn, tfn, fn_;
	while (DDRSys::findAllFiles_Next(pH, &pDirName, nullptr)) {
		u64 dID = _toU64(pDirName);
		if (0 == dID) {
			continue;
		}
		if (m_DIDMapper.end() != m_DIDMapper.find(dID)) { // already existing
			continue;
		}
		fn = stName;
		fn += pDirName;
		tfn = fn + "/";
		fn += g_InfoFileName;
		std::ifstream ifs(fn);
		if (!ifs.is_open()) {
			continue;
		}
		std::shared_ptr<OneRoute> pNewRoute = std::make_shared<OneRoute>();
		int nFiles;
		if (!(ifs >> (pNewRoute->ver) >> (pNewRoute->rID) >> (pNewRoute->routeName) >> (pNewRoute->crTime)
			>> (pNewRoute->uploadID) >> (pNewRoute->downloadID) >> nFiles)) {
			continue;
		}
		if (m_UIDMapper.find(pNewRoute->uploadID) != m_UIDMapper.end() ||
			m_DIDMapper.find(pNewRoute->downloadID) != m_DIDMapper.end() ||
			m_UIDMapper.find(pNewRoute->downloadID) != m_UIDMapper.end() ||
			m_DIDMapper.find(pNewRoute->uploadID) != m_DIDMapper.end()) { // existing
			continue;
		}
		ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		for (int i = 0; i < nFiles; ++i,
			 ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n')) {
			if (!std::getline(ifs, fn)) {
				continue;
			}
			size_t sz, pos;
			__int64 mtime;
			if (!(ifs >> sz >> pos >> mtime)) {
				continue;
			}
			fn_ = tfn + fn;
			sz = DDRSys::getFileSize(fn_.c_str());
			if (0 == sz) {
				continue;
			}
			mtime = DDRSys::getModTime(fn_.c_str());
			if (-1 == mtime) {
				continue;
			}
			fn = DDRSys::sysStr_to_utf8(fn.c_str());
			pNewRoute->files.emplace_back(OneFileRec{ fn, sz, pos, mtime });
		}
		pNewRoute->sortFiles();

		fn = stName;
		fn += pDirName;
		fn += g_CommentFileName;
		ifs.close();
		ifs.open(fn, std::ios::binary | std::ios::ate);
		if (ifs.is_open() && ifs.tellg() > 0) {
			std::vector<char> descr_(ifs.tellg());
			ifs.seekg(0);
			ifs.read(&descr_[0], descr_.size());
			if (ifs.good()) {
				if (descr_.back() != '\0') {
					descr_.emplace_back('\0');
				}
				pNewRoute->description_u8 = &descr_[0];
			}
		}

		m_UIDMapper[pNewRoute->uploadID] = (int)m_routes.size();
		m_DIDMapper[pNewRoute->downloadID] = (int)m_routes.size();
		m_RIDMapper[pNewRoute->rID].emplace_back(m_routes.size());
		m_routes.emplace_back(pNewRoute);
	}
}

int ServerSideRouteManager::OneRoute::findFileName(const char *pFileName, bool &bExist)
{
	if (files.empty()) {
		bExist = false;
		return 0;
	}
	int rr = files[0].fn.compare(pFileName);
	if (rr > 0) {
		bExist = false;
		return 0;
	} else if (0 == rr) {
		bExist = true;
		return 0;
	}
	rr = files.back().fn.compare(pFileName);
	if (rr < 0) {
		bExist = false;
		return (int)files.size();
	} else if (0 == rr) {
		bExist = true;
		return (int)files.size() - 1;
	}
	int aa = 0, bb = (int)files.size() - 1;
	while (bb - aa > 1) {
		int cc = (aa + bb) >> 1;
		rr = files[cc].fn.compare(pFileName);
		if (rr < 0) {
			aa = cc;
		} else if (rr > 0) {
			bb = cc;
		} else {
			bExist = true;
			return cc;
		}
	}
	bExist = false;
	return bb;
}

void ServerSideRouteManager::OneRoute::insertFile(int index, const char *pFileName, size_t fSz, size_t pos, __int64 mtime)
{
	if (index < 0 || index > (int)files.size()) {
		return;
	}
	files.emplace_back(OneFileRec());
	for (int i = (int)files.size() - 1; i > index; --i) {
		std::swap(files[i], files[i - 1]);
	}
	files[index].fn = pFileName;
	files[index].sz = fSz;
	files[index].pos = pos;
	files[index].mtime = mtime;
}

void ServerSideRouteManager::OneRoute::eraseFile(int index)
{
	if (index < 0 || index >= (int)files.size()) {
		return;
	}
	for (int i = index; i < (int)files.size() - 1; ++i) {
		std::swap(files[i], files[i + 1]);
	}
	files.pop_back();
}

void ServerSideRouteManager::OneRoute::updateDescription()
{
	std::string fn = "Data/" + _fromU64(downloadID);
	fn += g_CommentFileName;
	std::ifstream ifs(fn, std::ios::binary | std::ios::ate);
	if (!ifs.is_open() || ifs.tellg() == 0) {
		return;
	}
	std::vector<char> bytes((int)ifs.tellg() + 1);
	ifs.seekg(0);
	ifs.read(&bytes[0], bytes.size());
	if (!ifs.good()) {
		return;
	}
	bytes.back() = '\0';
	description_u8 = std::string(&bytes[0]);
}

void ServerSideRouteManager::OneRoute::sortFiles()
{
	std::sort(files.begin(), files.end(),
		[](const OneFileRec &ofr1, const OneFileRec &ofr2) {return (ofr1.fn.compare(ofr2.fn) < 0); });
}

bool ServerSideRouteManager::OneRoute::save()
{
	std::string str = "Data/";
	str += _fromU64(downloadID);
	str += g_InfoFileName;
	std::ofstream ofs(str);
	if (!ofs.is_open()) {
		return false;
	}
	ofs << this->ver << "\n"
		<< this->rID << "\n"
		<< this->routeName << "\n"
		<< this->crTime << "\n"
		<< this->uploadID << "\n"
		<< this->downloadID << "\n"
		<< this->files.size() << "\n";
	for (auto &ff : this->files) {
		ofs << DDRSys::utf8_to_sysStr(ff.fn.c_str()) << "\n" << ff.sz << '\t' << ff.pos << '\t' << ff.mtime << "\n";
	}
	return (ofs.good());
}

void ServerSideRouteManager::save()
{
	DDRMTLib::_lock_guard lll(true, m_gLoc);

	std::string stName = "Data/", fn;
	for (auto &ele : m_routes) {
		ele->save();
	}
}

ServerSideRouteManager::ServerSideRouteManager()
{
	DDRSys::createDir("Data");
	load();
	m_funcMapper["DDRCommProto.reqCreateRoute"] = std::bind(&ServerSideRouteManager::_rspCreateRoute, this, std::placeholders::_1);
	m_funcMapper["DDRCommProto.reqUploadFiles"] = std::bind(&ServerSideRouteManager::_rspUploadFiles, this, std::placeholders::_1);
	m_funcMapper["DDRCommProto.reqListRoutes"] = std::bind(&ServerSideRouteManager::_rspListRoutes, this, std::placeholders::_1);
	m_funcMapper["DDRCommProto.reqListFiles_uploader"] = std::bind(&ServerSideRouteManager::_rspListFiles, this, std::placeholders::_1);
	m_funcMapper["DDRCommProto.reqListFiles_downloader"] = std::bind(&ServerSideRouteManager::_rspListFiles, this, std::placeholders::_1);
	m_funcMapper["DDRCommProto.reqDownloadFiles"] = std::bind(&ServerSideRouteManager::_rspDownloadFiles, this, std::placeholders::_1);
}

ServerSideRouteManager::~ServerSideRouteManager()
{
	save();
	m_routes.clear();
}

std::shared_ptr<google::protobuf::Message> ServerSideRouteManager::Feed(std::shared_ptr<google::protobuf::Message> pMsg)
{
	if (pMsg) {
		auto pEle = m_funcMapper.find(pMsg->GetTypeName());
		if (m_funcMapper.end() != pEle) {
			auto ret = (pEle->second)(pMsg.get());
			return ret;
		}
	}
	return nullptr;
}

}