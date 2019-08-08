#include "DownloadManagerBase.h"

#include <iostream>
#include <fstream>
#include "Thirdparty/zlib/zlib.h"
#include "Src/Utility/CommonFunc.h"
#include "../Common/RandWrapper.h"

namespace DDRCloudService {

static const char g_InfoFileName[] = "/__INFO__.ini";
static int g_DownloadedRouteIndicator = 1;
static int g_maxConsecutiveFailCnt = 5;
extern int MAX_SZ_ONE_DATA_SLICE;

bool _writeFile(char *pRelativeFN, size_t curPos, const char *pDataHead, size_t len)
{
	if (!pRelativeFN || !pDataHead || 0 == len) {
		return false;
	}
	for (char *pC = pRelativeFN; *pC; ++pC) {
		if ('/' == *pC || '\\' == *pC) {
			*pC = '\0';
			DDRSys::createDir(pRelativeFN);
			*pC = '/';
		}
	}
	std::ofstream ofs;
	if (curPos > 0) {
		ofs.open(pRelativeFN, std::ios::binary | std::ios::in | std::ios::out);
		if (!ofs.is_open()) {
			return false;
		}
		ofs.seekp(curPos);
	} else {
		ofs.open(pRelativeFN, std::ios::binary);
		if (!ofs.is_open()) {
			return false;
		}
	}
	ofs.write(pDataHead, len);
	return ofs.good();
}

DownloadManagerBase::DownloadManagerBase(const char *pWorkingDir, const routeInfo &ri)
	: m_stage(-1), m_except(), m_nFailCnt(0)
{
	if (!pWorkingDir || !(*pWorkingDir)) {
		m_dir = "";
	} else {
		m_dir = pWorkingDir;
		if (m_dir.back() != '/' && m_dir.back() != '\\') {
			m_dir += "/";
		}
	}
	m_dir += ri.robotID + "_" + ri.routeName;
	std::string str = m_dir + g_InfoFileName;
	if (!DDRSys::isFileExisting(str.c_str())) {
		DDRSys::createDir(m_dir.c_str());
		std::ofstream ofs(str);
		if (!ofs.is_open()) {
			m_except = -1;
			return;
		}
		ofs << ri.version << "\n" << ri.robotID << "\n"
			<< g_DownloadedRouteIndicator << "\n" << ri.downloadID << "\n0"
			<< std::endl; // ver rid typeIndicator did nExistingFiles
		if (ofs.good()) {
			m_msg2Snd = std::make_shared<RemoteRouteProto::reqListFiles_downloader>();
			((RemoteRouteProto::reqListFiles_downloader*)m_msg2Snd.get())->set_downloadid(ri.downloadID);
			((RemoteRouteProto::reqListFiles_downloader*)m_msg2Snd.get())->set_reqguid(DDRSys::_rand_u64());
			m_RI = ri;
			m_stage = 0;
			return;
		} else {
			DDRSys::deleteFile(str.c_str());
			m_except = -1;
			return;
		}
	}

	std::ifstream ifs(str);
	if (!ifs.is_open()) {
		m_except = -1;
		return;
	}
	int _ver, _typeInd, _nFiles;
	std::string _rid;
	unsigned __int64 _did;
	if (!(ifs >> _ver >> _rid >> _typeInd >> _did >> _nFiles)) {
		m_except = 1;
		return;
	}
	std::string tfn = m_dir + "/", fn, fn_;
	ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	for (int i = 0; i < _nFiles; ++i,
		ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n')) {
		if (!std::getline(ifs, fn)) {
			continue;
		}
		size_t _sz, _pos, sz;
		__int64 _mtime, mtime;
		if (!(ifs >> _sz >> _pos >> _mtime) || _mtime <= 0 || _pos > _sz) {
			continue;
		}
		fn_ = tfn + fn;
		sz = DDRSys::getFileSize(fn_.c_str());
		if (0 == sz || sz != _pos) {
			continue;
		}
		mtime = DDRSys::getModTime(fn_.c_str());
		if (-1 == mtime || mtime != _mtime) {
			continue;
		}
		fn = DDRSys::sysStr_to_utf8(fn.c_str());
		m_existingFiles.emplace_back(OneExistingFile({ fn, _mtime, _sz, _pos  }));
	}
	std::sort(m_existingFiles.begin(), m_existingFiles.end(),
		[](const OneExistingFile &oef1, const OneExistingFile &oef2) {return oef1.name.compare(oef2.name) < 0; });
	m_msg2Snd = std::make_shared<RemoteRouteProto::reqListFiles_downloader>();
	((RemoteRouteProto::reqListFiles_downloader*)m_msg2Snd.get())->set_downloadid(ri.downloadID);
	((RemoteRouteProto::reqListFiles_downloader*)m_msg2Snd.get())->set_reqguid(DDRSys::_rand_u64());
	m_RI = ri;
	m_stage = 0;
}

DownloadManagerBase::~DownloadManagerBase()
{
	saveRec();
}

int DownloadManagerBase::GetStage() const
{
	return m_stage;
}

int DownloadManagerBase::GetException() const
{
	return m_except;
}

std::shared_ptr<google::protobuf::Message> DownloadManagerBase::GetNextMsg()
{
	return m_msg2Snd;
}

std::shared_ptr<google::protobuf::Message> DownloadManagerBase::Feed(std::shared_ptr<google::protobuf::Message> pRcvMsg)
{
	if (!m_msg2Snd || !pRcvMsg.get()) {
		return false;
	}
	std::string str = pRcvMsg->GetTypeName();
	if ("RemoteRouteProto.rspListFiles" == str) {
		_rsp_listFiles((RemoteRouteProto::rspListFiles*)pRcvMsg.get());
	} else if ("RemoteRouteProto.rspDownloadFiles" == str) {
		_rsp_downloadFiles((RemoteRouteProto::rspDownloadFiles*)pRcvMsg.get());
	} else {
		m_msg2Snd.reset();
		m_stage = -1;
		m_except = 3;
	}
	return m_msg2Snd;
}

bool DownloadManagerBase::IsFileNecessary(const char *pFileName)
{
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
	}
	else if (0 == cRet) {
		*pbExist = true;
		return 0;
	}
	cRet = strcmp(pStr, ((const std::string*)((const char*)pData + (nData - 1) * szPerData + nPStrOffset))->c_str());
	if (cRet > 0) {
		*pbExist = false;
		return nData;
	}
	else if (0 == cRet) {
		*pbExist = true;
		return (nData - 1);
	}
	int aa = 0, bb = nData - 1;
	while (bb - aa > 1) {
		int cc = (aa + bb) >> 1;
		cRet = strcmp(pStr, ((const std::string*)((const char*)pData + cc * szPerData + nPStrOffset))->c_str());
		if (cRet < 0) {
			bb = cc;
		}
		else if (cRet > 0) {
			aa = cc;
		}
		else {
			*pbExist = true;
			return cc;
		}
	}
	*pbExist = false;
	return bb;
}

void DownloadManagerBase::_rsp_listFiles(const RemoteRouteProto::rspListFiles *pRsp)
{
	if (0 != m_stage || m_msg2Snd->GetTypeName() != "RemoteRouteProto.reqListFiles_downloader" ||
		((RemoteRouteProto::reqListFiles_downloader*)m_msg2Snd.get())->reqguid() != pRsp->reqguid()) {
		m_stage = -1;
		m_except = 3;
		return;
	}

	switch (pRsp->ret()) {
	case RemoteRouteProto::eOkay:
		break;
	case RemoteRouteProto::eIDInvalid:
		m_stage = -1;
		m_except = 2;
		return;
		break;
	case RemoteRouteProto::eTooBusy:
		if (++m_nFailCnt > g_maxConsecutiveFailCnt) {
			m_stage = -1;
			m_except = 9;
		} else {
			((RemoteRouteProto::reqListFiles_downloader*)m_msg2Snd.get())->set_reqguid(DDRSys::_rand_u64());
		}
		return;
		break;
	default:
		m_stage = -1;
		m_except = 3;
		return;
		break;
	}	
	
	m_downloableFiles.resize(0);
	if (pRsp->filerecords_sz() > 0) {
		RemoteRouteProto::MultipleFileInfo mfi;
		if (RemoteRouteProto::eNoZip == pRsp->ziptype()) {
			if (pRsp->filerecords_sz() != pRsp->filerecords().length() ||
				!mfi.ParseFromArray(pRsp->filerecords().c_str(),
					                (int)pRsp->filerecords().length())) {
				m_stage = -1;
				m_except = 3;
				return;
			}
		} else if (RemoteRouteProto::eZLib == pRsp->ziptype()) {
			uLongf oriSz = (uLongf)pRsp->filerecords_sz();
			std::vector<char> buf(oriSz);
			if (Z_OK != uncompress((Bytef*)&buf[0], &oriSz,
				                   (const Bytef*)pRsp->filerecords().c_str(),
				                   (uLong)pRsp->filerecords().length()) ||
				oriSz != (uLongf)pRsp->filerecords_sz()) {
				m_stage = -1;
				m_except = 4;
				return;
			}
			if (!mfi.ParseFromArray(&buf[0], oriSz)) {
				m_stage = -1;
				m_except = 4;
				return;
			}
		}

		m_downloableFiles.resize(mfi.records().size());
		int nF = 0;
		for (int i = 0; i < (int)m_downloableFiles.size(); ++i) {
			if (!IsFileNecessary(mfi.records()[i].name().c_str()) ||
				mfi.records()[i].sz() != mfi.records()[i].curpos()) {
				const char *pName = mfi.records()[i].name().c_str();
				__int64 sz__ = mfi.records()[i].sz();
				__int64 curpos__ = mfi.records()[i].curpos();
				continue;
			}
			m_downloableFiles[nF].name = mfi.records()[i].name().c_str();
			std::transform(m_downloableFiles[nF].name.begin(), m_downloableFiles[nF].name.end(),
				           m_downloableFiles[nF].name.begin(), ::tolower);
			m_downloableFiles[nF].mtime = mfi.records()[i].mtime();
			m_downloableFiles[nF].sz = mfi.records()[i].sz();
			m_downloableFiles[nF].pos = 0;
			++nF;
			//std::cout << "nF = " << nF << ", i = " << i << std::endl;
		}
		m_downloableFiles.resize(nF);
		std::sort(m_downloableFiles.begin(), m_downloableFiles.end(),
			[](const RemoteFileStat &rfs1, const RemoteFileStat &rfs2) { return rfs1.name.compare(rfs2.name) < 0; });
	} else {
		m_downloableFiles.resize(0);
	}
	adjustDownloadableFiles();
	int nRet = prepareNextDownloadReq();
	if (0 == nRet) {
		m_nFailCnt = 0;
	} else if (-1 == nRet) {
		m_stage = -1;
	}
}

void DownloadManagerBase::_rsp_downloadFiles(const RemoteRouteProto::rspDownloadFiles *pRsp)
{
	if (1 != m_stage || m_msg2Snd->GetTypeName() != "RemoteRouteProto.reqDownloadFiles" ||
		((RemoteRouteProto::reqDownloadFiles*)m_msg2Snd.get())->reqguid() != pRsp->reqguid() ||
		m_downloableFiles.empty()) {
		m_stage = -1;
		m_except = 3;
		return;
	}
	
	switch (pRsp->ret()) {
	case RemoteRouteProto::eOkay:
		break;
	case RemoteRouteProto::eIDInvalid:
		m_stage = -1;
		m_except = 2;
		m_msg2Snd.reset();
		return;
		break;
	case RemoteRouteProto::eTooBusy:
		if (++m_nFailCnt > g_maxConsecutiveFailCnt) {
			m_stage = -1;
			m_except = 9;
			m_msg2Snd.reset();
		} else {
			((RemoteRouteProto::reqDownloadFiles*)m_msg2Snd.get())->set_reqguid(DDRSys::_rand_u64());
		}
		return;
		break;
	default:
		m_stage = -1;
		m_except = 3;
		m_msg2Snd.reset();
		return;
		break;
	}

	m_msg2Snd.reset();
	if (pRsp->filenames_size() != pRsp->filesz_size() ||
		pRsp->filenames_size() != pRsp->file_pos0_size() ||
		pRsp->filenames_size() != pRsp->file_pos1_size() ||
		pRsp->content_len() > MAX_SZ_ONE_DATA_SLICE) {
		m_stage = -1;
		m_except = 3;
		return;
	}
	int nFiles = pRsp->filenames_size(), accSz = 0;
	for (int i = 0; i < nFiles; ++i) {
		if (pRsp->file_pos0()[i] < 0 ||
			pRsp->file_pos0()[i] >= pRsp->file_pos1()[i] ||
			pRsp->file_pos1()[i] > pRsp->filesz()[i]) {
			m_stage = -1;
			m_except = 3;
			return;
		}
		accSz += (int)(pRsp->file_pos1()[i] - pRsp->file_pos0()[i]);
	}
	if (accSz != pRsp->content_len()) {
		m_stage = -1;
		m_except = 3;
		return;
	}

	const char *pBuf = nullptr;
	size_t nBufLen = 0;
	std::vector<char> oriContent;
	if (RemoteRouteProto::eZLib == pRsp->ziptype()) {
		uLongf dstLen = (uLongf)pRsp->content_len();
		oriContent.resize(dstLen);
		if (Z_OK != uncompress((Bytef*)&oriContent[0], &dstLen,
			                   (const Bytef*)pRsp->contents().c_str(),
			                   (uLong)pRsp->contents().length()) ||
			(__int64)dstLen != pRsp->content_len()) {
			m_stage = -1;
			m_except = 4;
			return;
		}
		pBuf = &oriContent[0];
		nBufLen = (size_t)pRsp->content_len();
	} else if (RemoteRouteProto::eNoZip == pRsp->ziptype()) {
		if (pRsp->content_len() != (__int64)pRsp->contents().length()) {
			m_stage = -1;
			m_except = 3;
			return;
		}
		pBuf = pRsp->contents().c_str();
		nBufLen = (size_t)pRsp->content_len();
	}

	bool bWriteFailed = false;
	size_t bufPos = 0;
	std::string str, fn;
	str = m_dir + "/";
	int nNameOffset = (int)((const char*)(&(m_downloableFiles[0].name)) - (const char*)(&m_downloableFiles[0]));
	for (int i = 0; i < nFiles; ++i) {
		const char *pC = pRsp->filenames()[i].c_str();
		bool bExist = false;
		int tarFileIndex = _find(&m_downloableFiles[0], sizeof(RemoteFileStat), (int)m_downloableFiles.size(),
			                     nNameOffset, pRsp->filenames()[i].c_str(), &bExist);
		if (!bExist || pRsp->file_pos0()[i] != m_downloableFiles[tarFileIndex].pos ||
			pRsp->filesz()[i] != m_downloableFiles[tarFileIndex].sz) {
			m_stage = -1;
			m_except = 3;
			return;
		}
		fn = str + pRsp->filenames()[i];
		fn = DDRSys::utf8_to_sysStr(fn.c_str());
		if (_writeFile(&fn[0], m_downloableFiles[tarFileIndex].pos,
			           pBuf + bufPos, pRsp->file_pos1()[i] - pRsp->file_pos0()[i])) {
			DDRSys::setModTime(fn.c_str(), m_downloableFiles[tarFileIndex].mtime);
			m_downloableFiles[tarFileIndex].pos = pRsp->file_pos1()[i];
			if (pRsp->file_pos1()[i] == m_downloableFiles[tarFileIndex].sz) {
				//std::cout << "Completed file '" << fn << "'" << std::endl;
			}
		} else {
			bWriteFailed = true;
			continue;
		}
		bufPos += pRsp->file_pos1()[i] - pRsp->file_pos0()[i];
	}

	adjustFilePtr();
	saveRec();
	if (!bWriteFailed) {
		m_nFailCnt = 0;
		prepareNextDownloadReq();
	} else {
		if (++m_nFailCnt > g_maxConsecutiveFailCnt) {
			m_stage = -1;
			m_except = 9;
		}
	}
}

void DownloadManagerBase::adjustDownloadableFiles()
{
	m_fListPtr = 0;
	if (m_downloableFiles.empty() || m_existingFiles.empty()) {
		return;
	}
	int nNameOffset = (int)((const char*)(&(m_downloableFiles[0].name)) - (const char*)(&m_downloableFiles[0]));
	int lLim = 0;
	for (int i = 0; lLim < (int)m_existingFiles.size() && i < (int)m_downloableFiles.size(); ++i) {
		bool bExist;
		int mID = _find(&m_existingFiles[lLim], sizeof(OneExistingFile), (int)m_existingFiles.size() - lLim,
			nNameOffset, m_downloableFiles[i].name.c_str(), &bExist);
		mID += lLim;
		if (bExist && m_existingFiles[mID].mtime == m_downloableFiles[i].mtime &&
			m_existingFiles[mID].sz == m_downloableFiles[i].sz) {
			m_downloableFiles[i].pos = m_existingFiles[mID].pos;
		}
		lLim = mID + (bExist ? 1 : 0);
	}
	adjustFilePtr();
}

void DownloadManagerBase::adjustFilePtr()
{
	for (; m_fListPtr < (int)m_downloableFiles.size(); ++m_fListPtr) {
		if (m_downloableFiles[m_fListPtr].pos < m_downloableFiles[m_fListPtr].sz) {
			break;
		}
	}
}

int DownloadManagerBase::prepareNextDownloadReq()
{
	m_msg2Snd.reset();
	if (m_fListPtr >= (int)m_downloableFiles.size()) {
		m_stage = 2;
		return 1;
	}

	m_msg2Snd = std::make_shared<RemoteRouteProto::reqDownloadFiles>();
	auto pMsg = (RemoteRouteProto::reqDownloadFiles*)m_msg2Snd.get();
	int nBytes = 0;
	for (int i = m_fListPtr; i < (int)m_downloableFiles.size(); ++i) {
		if (m_downloableFiles[i].pos >= m_downloableFiles[i].sz) {
			continue;
		}
		int thisSz = (int)(m_downloableFiles[i].sz - m_downloableFiles[i].pos);
		if (nBytes + thisSz > MAX_SZ_ONE_DATA_SLICE) {
			thisSz = (MAX_SZ_ONE_DATA_SLICE - nBytes);
		}
		pMsg->add_filenames(m_downloableFiles[i].name);
		pMsg->add_file_pos0(m_downloableFiles[i].pos);
		pMsg->add_file_pos1(m_downloableFiles[i].pos + thisSz);
		nBytes += thisSz;
		if (MAX_SZ_ONE_DATA_SLICE == nBytes) {
			break;
		}
	}
	if (nBytes > 0) {
		pMsg->set_reqguid(DDRSys::_rand_u64());
		pMsg->set_downloadid(m_RI.downloadID);
		m_stage = 1;
		return 0;
	} else {
		m_fListPtr = (int)m_downloableFiles.size();
		m_msg2Snd.reset();
		m_stage = 2;
		return 1;
	}
}

void DownloadManagerBase::saveRec()
{
	if (m_RI.routeName.empty()) {
		return;
	}
	std::string str = m_dir + g_InfoFileName;
	std::ofstream ofs(str);
	ofs << m_RI.version << "\n" << m_RI.robotID << "\n"
		<< g_DownloadedRouteIndicator << "\n"
		<< m_RI.downloadID << "\n" << m_downloableFiles.size() << std::endl;
	for (auto &x : m_downloableFiles) {
		ofs << DDRSys::utf8_to_sysStr(x.name.c_str()) << "\n"
			<< x.sz << "\t" << x.pos << "\t" << x.mtime << "\n";
	}
	ofs.close();
}


}