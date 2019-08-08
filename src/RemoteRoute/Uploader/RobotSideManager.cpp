#include "RobotSideManager.h"

#include <algorithm>
#include <fstream>
#include "Thirdparty/zlib/zlib.h"
#include "src/Utility/LoggerDef.h"
#include "src/Utility/CommonFunc.h"
#include "../Common/RandWrapper.h"

namespace DDRCloudService {

static std::string g_NecessaryFiles[] = { "__INFO__.ini" };
static std::string g_UploadExclusionFiles[] = { "__INFO__.ini" };
static char g_InfoFileName[] = "__INFO__.ini";
static int g_MaxFailureCnt4SameSlice = 5;
extern int MAX_SZ_ONE_DATA_SLICE;
int g_CurrentRouteVersion = 10001;

/*
Version 10001 __INFO__.ini
[VersionNo.] (10001)
[RobotID]
[upload (0) or download (1)]
[not started (0) or partial (1) or finished(2)] (only for upload)
[UploadID]
*/

RobotSideRouteManager::RobotSideRouteManager(const char *pRouteName)
	: m_stage(-1), m_except(0)
{
	if (!pRouteName || !(*pRouteName)) {
		return;
	}
	m_dir = pRouteName;
	while ('/' == m_dir.back() || '\\' == m_dir.back()) {
		m_dir.pop_back();
	}
	if (m_dir.empty()) {
		return;
	}
	m_routeName = m_dir;
	for (int i = (int)m_dir.length() - 1; i >= 0; --i) {
		if ('/' == m_dir.c_str()[i] || '\\' == m_dir.c_str()[i]) {
			if (i + 1 >= (int)m_dir.length() - 1) {
				return;
			}
			m_routeName = m_dir.c_str() + (i + 1);
			break;
		}
	}
	if (-1 == (m_cTime = DDRSys::getCreationTime(m_dir.c_str()))) {
		return;
	}
	m_dir += "/";
	for (auto &ch : m_dir) {
		if ('\\' == ch) {
			ch = '/';
		}
	}

	Reset();
}

void RobotSideRouteManager::Reset()
{
	if (m_stage >= 0) {
		return;
	}
	m_stage = -1;

	for (int i = 0; i < sizeof(g_NecessaryFiles) / sizeof(std::string); ++i) {
		m_str = m_dir + g_NecessaryFiles[i];
		if (!DDRSys::isFileExisting(m_str.c_str())) {
			return;
		}
	}
	if (!listUploadableFiles()) {
		return;
	}

	m_str = m_dir + g_InfoFileName;
	std::ifstream ifs(m_str);
	if (!ifs.is_open() || !(ifs >> m_ver >> m_rID)) {
		return;
	}
	int ud;
	if (!(ifs >> ud) || ud != 0) {
		return;
	}
	if (!(ifs >> ud) || (ud < 0 || ud > 2)) {
		return;
	}
	m_stage = ud;

	if (0 != ud) {
		u64 uid;
		if (!(ifs >> uid)) {
			m_stage = -1;
			return;
		}
		m_uploadID = uid;
	}

	switch (m_stage) {
	case 0: // req to create a new route
		m_msg2Snd = std::make_shared<RemoteRouteProto::reqCreateRoute>();
		((RemoteRouteProto::reqCreateRoute*)m_msg2Snd.get())->mutable_routeinfo()->set_version(m_ver);
		((RemoteRouteProto::reqCreateRoute*)m_msg2Snd.get())->mutable_routeinfo()->set_robotid(DDRSys::sysStr_to_utf8(m_rID.c_str()));
		((RemoteRouteProto::reqCreateRoute*)m_msg2Snd.get())->mutable_routeinfo()->set_routename(DDRSys::sysStr_to_utf8(m_routeName.c_str()));
		((RemoteRouteProto::reqCreateRoute*)m_msg2Snd.get())->mutable_routeinfo()->set_datetime(m_cTime);
		break;

	case 1: // req to list server's files (so that we can upload partially)
		m_msg2Snd = std::make_shared<RemoteRouteProto::reqListFiles_uploader>();
		((RemoteRouteProto::reqListFiles_uploader*)m_msg2Snd.get())->set_uploadid(m_uploadID);
		((RemoteRouteProto::reqListFiles_uploader*)m_msg2Snd.get())->set_reqguid(DDRSys::_rand_u64());
		break;
	}

	m_oriContent.reserve(MAX_SZ_ONE_DATA_SLICE);
}

int RobotSideRouteManager::GetStage() const
{
	return m_stage;
}

int RobotSideRouteManager::GetException() const
{
	return m_except;
}

RobotSideRouteManager::~RobotSideRouteManager()
{
}

void RobotSideRouteManager::_rsp_createRoute(const RemoteRouteProto::rspCreateRoute *pRsp)
{
	if (m_msg2Snd->GetTypeName() == "RemoteRouteProto.reqCreateRoute") {
		if (0 != m_stage) {
			m_stage = -2;
			m_except = SERVER_RESP_INVALID;
			return;
		}
		if (RemoteRouteProto::eOkay == pRsp->ret()) {
			m_uploadID = pRsp->uploadid();
			m_str = m_dir + g_InfoFileName;
			std::ofstream ofs(m_str);
			ofs << m_ver << std::endl;
			ofs << m_rID << std::endl;
			ofs << "0\n1\n" << m_uploadID << std::endl;
			if (!ofs || !listUploadableFiles()) {
				m_stage = -2;
				m_except = INTERNAL_ERROR;
				return;
			}
			int nRet = prepareNextSliceUpload();
			if (0 == nRet) { // next slice prepared
				m_stage = 1;
				m_uploadFailCnt = 0;
			} else if (1 == nRet) { // finished
				m_stage = 2;
				m_msg2Snd.reset();
			} else {
				m_stage = -2;
				m_except = INTERNAL_ERROR;
			}
		} else if (RemoteRouteProto::eTooBusy == pRsp->ret() ||
			       RemoteRouteProto::eInternalErr == pRsp->ret()) {
			if (++m_uploadFailCnt >= g_MaxFailureCnt4SameSlice) {
				m_stage = -2;
				m_except = FAILED_TOO_MANY_TIMES;
			}
		} else {
			m_stage = -2;
			switch (pRsp->ret()) {
			case RemoteRouteProto::eReqVoid:
				m_except = DATA_ERROR;
				break;
			case RemoteRouteProto::eIDInvalid:
				m_except = ID_INVALID;
				break;
			case RemoteRouteProto::eUp_TooFrequent:
				m_except = ROUTE_CREATION_DENIED;
				break;
			case RemoteRouteProto::eUp_TooBig:
			case RemoteRouteProto::eUp_DataError:
				m_except = DATA_ERROR;
				break;
			default:
				m_except = INTERNAL_ERROR;
				break;
			}
		}
	} else {
		m_stage = -2;
		m_except = SERVER_RESP_INVALID;
	}
}

void RobotSideRouteManager::_rsp_uploadFiles(const RemoteRouteProto::rspUploadFiles *pRsp)
{
	if (m_msg2Snd->GetTypeName() == "RemoteRouteProto.reqUploadFiles") {
		RemoteRouteProto::reqUploadFiles *pReq = (RemoteRouteProto::reqUploadFiles*)m_msg2Snd.get();
		if (1 != m_stage || pRsp->reqguid() != pReq->reqguid()) {
			m_stage = -2;
			m_except = SERVER_RESP_INVALID;
			return;
		}
		if (RemoteRouteProto::eOkay == pRsp->ret()) {
			int nRet = prepareNextSliceUpload();
			if (0 == nRet) {
				if (0 == m_stage) {
					m_str = m_dir + g_InfoFileName;
					std::ofstream ofs(m_str);
					ofs << m_ver << std::endl;
					ofs << m_rID << std::endl;
					ofs << "0\n1\n" << m_uploadID;
					if (!ofs) {
						m_stage = -2;
						m_except = INTERNAL_ERROR;
						return;
					}
				}
				m_stage = 1;
				m_uploadFailCnt = 0;
			} else if (1 == nRet) {
				if (1 == m_stage) {
					m_str = m_dir + g_InfoFileName;
					std::ofstream ofs(m_str);
					ofs << m_ver << std::endl;
					ofs << m_rID << std::endl;
					ofs << "0\n2\n" << m_uploadID;
					if (!ofs) {
						m_stage = -2;
						m_except = INTERNAL_ERROR;
						return;
					}
				}
				m_stage = 2;
				m_msg2Snd.reset();
			} else if (-1 == nRet) {
				m_stage = -2;
				m_except = INTERNAL_ERROR;
			}
		} else if (RemoteRouteProto::eTooBusy == pRsp->ret()) {
			if (++m_uploadFailCnt >= g_MaxFailureCnt4SameSlice) {
				m_stage = -2;
				m_except = FAILED_TOO_MANY_TIMES;
			}
		} else {
			m_stage = -2;
			switch (pRsp->ret()) {
			case RemoteRouteProto::eReqVoid:
				m_except = DATA_ERROR;
				break;
			case RemoteRouteProto::eIDInvalid:
				m_except = ID_INVALID; // UID invalid
				break;
			case RemoteRouteProto::eUp_TooBig:
			case RemoteRouteProto::eUp_DataError:
				m_except = DATA_ERROR;
				break;
			default:
				m_except = INTERNAL_ERROR;
				break;
			}
		}
	} else {
		m_stage = -2;
		m_except = SERVER_RESP_INVALID;
	}
}

void RobotSideRouteManager::_rsp_listFiles(const RemoteRouteProto::rspListFiles *pRsp)
{
	if (m_msg2Snd->GetTypeName() == "RemoteRouteProto.reqListFiles_uploader") {
		if (1 != m_stage) {
			m_stage = -2;
			m_except = SERVER_RESP_INVALID;
			return;
		}
		auto *pReq = (RemoteRouteProto::reqListFiles_uploader*)m_msg2Snd.get();
		if (RemoteRouteProto::eOkay == pRsp->ret()) {
			if (processRemoteList(pRsp)) {
				int nRet = prepareNextSliceUpload();
				if (0 == nRet) { // next slice prepared
					m_uploadFailCnt = 0;
				} else if (1 == nRet) { // finished
					m_str = m_dir + g_InfoFileName;
					std::ofstream ofs(m_str);
					ofs << m_ver << std::endl;
					ofs << m_rID << std::endl;
					ofs << "0\n2\n" << m_uploadID;
					if (!ofs) {
						m_stage = -2;
						m_except = INTERNAL_ERROR;
						return;
					}
					m_stage = 2;
					m_msg2Snd.reset();
				} else {
					m_stage = -2;
					m_except = INTERNAL_ERROR;
				}
			} else {
				m_stage = -2;
				m_except = SERVER_RESP_INVALID;
			}
		} else if (RemoteRouteProto::eTooBusy == pRsp->ret()) {
			if (++m_uploadFailCnt >= g_MaxFailureCnt4SameSlice) {
				m_stage = -2;
				m_except = FAILED_TOO_MANY_TIMES;
			}
		} else {
			m_stage = -2;
			switch (pRsp->ret()) {
			case RemoteRouteProto::eReqVoid:
				m_except = DATA_ERROR;
				break;
			case RemoteRouteProto::eIDInvalid:
				//m_except = ID_INVALID;
				m_stage = 0;
				m_msg2Snd = std::make_shared<RemoteRouteProto::reqCreateRoute>();
				((RemoteRouteProto::reqCreateRoute*)m_msg2Snd.get())->mutable_routeinfo()->set_version(m_ver);
				((RemoteRouteProto::reqCreateRoute*)m_msg2Snd.get())->mutable_routeinfo()->set_robotid(DDRSys::sysStr_to_utf8(m_rID.c_str()));
				((RemoteRouteProto::reqCreateRoute*)m_msg2Snd.get())->mutable_routeinfo()->set_routename(DDRSys::sysStr_to_utf8(m_routeName.c_str()));
				((RemoteRouteProto::reqCreateRoute*)m_msg2Snd.get())->mutable_routeinfo()->set_datetime(m_cTime);
				break;
			default:
				m_except = INTERNAL_ERROR;
				break;
			}
		}
	} else {
		m_stage = -2;
		m_except = SERVER_RESP_INVALID;
	}
}

std::shared_ptr<google::protobuf::Message> RobotSideRouteManager::GetNextMsg()
{
	return m_msg2Snd;
}

std::shared_ptr<google::protobuf::Message> RobotSideRouteManager::Feed(std::shared_ptr<google::protobuf::Message> pRcvMsg)
{
	if (!m_msg2Snd || !pRcvMsg.get()) {
		return false;
	}
	std::string str = pRcvMsg->GetTypeName();
	if ("RemoteRouteProto.rspCreateRoute" == str) {
		_rsp_createRoute((RemoteRouteProto::rspCreateRoute*)pRcvMsg.get());
	} else if ("RemoteRouteProto.rspUploadFiles" == str) {
		_rsp_uploadFiles((RemoteRouteProto::rspUploadFiles*)pRcvMsg.get());
	} else if ("RemoteRouteProto.rspListFiles" == str) {
		_rsp_listFiles((RemoteRouteProto::rspListFiles*)pRcvMsg.get());
	} else {
		m_stage = -2;
		m_except = SERVER_RESP_INVALID;
	}
	
	if (m_stage < 0) {
		m_msg2Snd.reset();
	}

	return m_msg2Snd;
}

void RobotSideRouteManager::OneFileProg::resetCompressable()
{
	if (name.length() > 4 && (0 == strcmp(".png", name.c_str() + name.length() - 4) ||
		0 == strcmp(".jpg", name.c_str() + name.length() - 4) ||
		0 == strcmp(".zip", name.c_str() + name.length() - 4) ||
		0 == strcmp(".rar", name.c_str() + name.length() - 4))) {
		bCompressable = false;
	} else {
		bCompressable = true;
	}
}

bool RobotSideRouteManager::listUploadableFiles()
{
	m_uploadableFiles.reserve(200);
	m_uploadableFiles.resize(0);
	void *pHandle = DDRSys::findAllFiles_Open(m_dir.c_str(), 0x01, true);
	if (!pHandle) {
		return false;
	}
	std::string fn;
	const char *pStr;
	while (DDRSys::findAllFiles_Next(pHandle, &pStr, nullptr)) {
		fn = pStr;
		bool bExcl = false;
		for (int i = 0; i < (int)(sizeof(g_UploadExclusionFiles) / sizeof(std::string)); ++i) {
			if (fn == g_UploadExclusionFiles[i]) {
				bExcl = true;
				break;
			}
		}
		if (bExcl) {
			continue;
		}
		fn = m_dir + fn;
		__int64 mtime = DDRSys::getModTime(fn.c_str());
		if (mtime < 0) {
			continue;
		}
		size_t sz = DDRSys::getFileSize(fn.c_str());
		if (0 == sz) {
			continue;
		}
		fn.erase(fn.begin(), fn.begin() + m_dir.length());
		fn = DDRSys::sysStr_to_utf8(fn.c_str());
		std::transform(fn.begin(), fn.end(), fn.begin(), ::tolower);
		m_uploadableFiles.emplace_back(OneFileProg{ true, fn, mtime, sz, 0 });
		m_uploadableFiles.back().resetCompressable();
	}
	DDRSys::findAllFile_Close(pHandle);
	std::sort(m_uploadableFiles.begin(), m_uploadableFiles.end(),
		[](const OneFileProg &ofp1, const OneFileProg &ofp2) {return ofp1.name.compare(ofp2.name) < 0; });

	m_fListPtr = 0;
	return true;
}

bool RobotSideRouteManager::processRemoteList(const RemoteRouteProto::rspListFiles *pRsp)
{
	if (pRsp->filerecords_sz() > 0) {
		RemoteRouteProto::MultipleFileInfo mfi;
		if (RemoteRouteProto::eNoZip == pRsp->ziptype()) {
			if (pRsp->filerecords_sz() != pRsp->filerecords().length() ||
				!mfi.ParseFromArray(pRsp->filerecords().c_str(),
					pRsp->filerecords().length())) {
				return false;
			}
		} else if (RemoteRouteProto::eZLib == pRsp->ziptype()) {
			uLongf oriSz = (uLongf)pRsp->filerecords_sz();
			std::vector<char> buf(oriSz);
			if (Z_OK != uncompress((Bytef*)&buf[0], &oriSz,
				                   (const Bytef*)pRsp->filerecords().c_str(),
				                   (uLong)pRsp->filerecords().length()) ||
				oriSz != (uLongf)pRsp->filerecords_sz()) {
				return false;
			}
			if (!mfi.ParseFromArray(&buf[0], oriSz)) {
				return false;
			}
		}

		m_remoteSavedFiles.resize(mfi.records().size());
		for (int i = 0; i < (int)m_remoteSavedFiles.size(); ++i) {
			m_remoteSavedFiles[i].name = mfi.records()[i].name().c_str();
			std::transform(m_remoteSavedFiles[i].name.begin(), m_remoteSavedFiles[i].name.end(), m_remoteSavedFiles[i].name.begin(), ::tolower);
			m_remoteSavedFiles[i].mtime = mfi.records()[i].mtime();
			m_remoteSavedFiles[i].sz = mfi.records()[i].sz();
			m_remoteSavedFiles[i].pos = mfi.records()[i].curpos();
		}
		std::sort(m_remoteSavedFiles.begin(), m_remoteSavedFiles.end(),
			[](const RemoteFileStat &rfs1, const RemoteFileStat &rfs2) {return rfs1.name.compare(rfs2.name) < 0; });
	} else {
		m_remoteSavedFiles.resize(0);
	}
	adjustUploadableFiles();
	return true;
}

static int _find(const void *pData, int szPerData, int nData,
	             int nPStrOffset, const char *pStr, bool *pbExist)
{
	if (nData <= 0) {
		*pbExist = false;
		return 0;
	}
	int cRet = strcmp(pStr, ((const std::string*)((const char*)pData + nPStrOffset))->c_str());
	if (cRet < 0) {
		*pbExist = false;
		return 0;
	} else if (0 == cRet) {
		*pbExist = true;
		return 0;
	}
	cRet = strcmp(pStr, ((const std::string*)((const char*)pData + (nData - 1) * szPerData + nPStrOffset))->c_str());
	if (cRet > 0) {
		*pbExist = false;
		return nData;
	} else if (0 == cRet) {
		*pbExist = true;
		return (nData - 1);
	}
	int aa = 0, bb = nData - 1;
	while (bb - aa > 1) {
		int cc = (aa + bb) >> 1;
		cRet = strcmp(pStr, ((const std::string*)((const char*)pData + cc * szPerData + nPStrOffset))->c_str());
		if (cRet < 0) {
			bb = cc;
		} else if (cRet > 0) {
			aa = cc;
		} else {
			*pbExist = true;
			return cc;
		}
	}
	*pbExist = false;
	return bb;
}

void RobotSideRouteManager::adjustUploadableFiles()
{
	m_fListPtr = 0;
	if (m_uploadableFiles.empty() || m_remoteSavedFiles.empty()) {
		return;
	}
	int nNameOffset = (const char*)(&(m_uploadableFiles[0].name)) - (const char*)(&m_uploadableFiles[0]);
	int lLim = 0;
	for (int i = 0; lLim < (int)m_uploadableFiles.size() && i < (int)m_remoteSavedFiles.size(); ++i) {
		bool bExist;
		int mID = _find(&m_uploadableFiles[lLim], sizeof(OneFileProg), (int)m_uploadableFiles.size() - lLim,
			            nNameOffset, m_remoteSavedFiles[i].name.c_str(), &bExist);
		mID += lLim;
		if (bExist && m_remoteSavedFiles[i].mtime == m_uploadableFiles[mID].mtime &&
			m_remoteSavedFiles[i].sz == m_uploadableFiles[mID].sz) {
			m_uploadableFiles[mID].pos = m_remoteSavedFiles[i].pos;
		}
		lLim = mID + (bExist ? 1 : 0);
	}
}

int RobotSideRouteManager::prepareNextSliceUpload()
{
	m_msg2Snd.reset();
	for (; m_fListPtr < (int)m_uploadableFiles.size() &&
		   m_uploadableFiles[m_fListPtr].pos >= m_uploadableFiles[m_fListPtr].sz;
		++m_fListPtr);
	if (m_fListPtr >= (int)m_uploadableFiles.size()) {
		return 1;
	}

	size_t accSz = 0, compSz = 0, plainSz = 0;
	m_oriContent.resize(0);
	auto pMsg = std::make_shared<RemoteRouteProto::reqUploadFiles>();
	pMsg->set_uploadid(m_uploadID);
	pMsg->set_reqguid(DDRSys::_rand_u64());
	
	std::string str;
	for (; m_fListPtr < (int)m_uploadableFiles.size(); ++m_fListPtr) {
		auto &ele = m_uploadableFiles[m_fListPtr];
		if (ele.pos >= ele.sz) {
			continue;
		}
		size_t remLen = ele.sz - ele.pos;
		size_t thisSz = (accSz + remLen <= MAX_SZ_ONE_DATA_SLICE ? remLen : (MAX_SZ_ONE_DATA_SLICE - accSz));
		str = DDRSys::utf8_to_sysStr(ele.name.c_str());
		if (0 == str.length()) {
			return -1; // unexpected
		}
		if (!readFile(str.c_str(), ele.pos, thisSz, m_oriContent)) {
			return -1;
		}
		//LevelLog(DDRFramework::Log::Level::INFO, "Added file '%s', [%d,%d)", str.c_str(), ele.pos, ele.pos + thisSz);
		pMsg->add_filenames(ele.name);
		pMsg->add_filesz(ele.sz);
		pMsg->add_filemtimes(ele.mtime);
		pMsg->add_file_pos0(ele.pos);
		ele.pos += thisSz;
		pMsg->add_file_pos1(ele.pos);
		if (ele.bCompressable) {
			compSz += thisSz;
		} else {
			plainSz += thisSz;
		}
		accSz += thisSz;
		if (accSz == MAX_SZ_ONE_DATA_SLICE) {
			break;
		}
	}

	if (0 == accSz) {
		return -1;
	}

	bool bDataOrg = false;
	if (compSz > plainSz * 2) {
		pMsg->mutable_contents()->resize(accSz);
		uLongf _afterCompSz = (uLongf)accSz;
		if (Z_OK == compress((Bytef*)&((*pMsg->mutable_contents())[0]), &_afterCompSz,
			                 (const Bytef*)&m_oriContent[0], (uLong)accSz)) {
			pMsg->set_ziptype(RemoteRouteProto::eZLib);
			pMsg->mutable_contents()->resize(_afterCompSz);
			bDataOrg = true;
		}
	}
	if (!bDataOrg) {
		pMsg->set_ziptype(RemoteRouteProto::eNoZip);
		pMsg->set_contents(&m_oriContent[0], accSz);
	}
	pMsg->set_content_len(accSz);
	m_msg2Snd = pMsg;

	return 0;
}

bool RobotSideRouteManager::readFile(const char *pRelativeFN, size_t sPos, size_t len,
	                                 std::vector<char> &content)
{
	m_str = m_dir + pRelativeFN;
	std::ifstream ifs(m_str, std::ios::binary | std::ios::ate);
	if (ifs.tellg() < sPos + len) {
		return false;
	}
	ifs.seekg(sPos);
	auto sz = content.size();
	content.resize(sz + len);
	return (bool)ifs.read(&content[sz], len);
}

}