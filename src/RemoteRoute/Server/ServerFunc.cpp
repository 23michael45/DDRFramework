#include "ServerFunc.h"

#include <fstream>
#include "Src/Utility/CommonFunc.h"

namespace DDRCloudService {

bool _isCompressable(const char *pFileName)
{
	size_t len = strlen(pFileName);
	if (len > 4 && (0 == strcmp(".png", pFileName + len - 4) ||
		0 == strcmp(".jpg", pFileName + len - 4) ||
		0 == strcmp(".zip", pFileName + len - 4) ||
		0 == strcmp(".rar", pFileName + len - 4))) {
		return false;
	}
	return true;
}

int _writeFile(char *pFileName, size_t oriFullSize, size_t newFullSize,
	           const char *pContent, size_t sPos, size_t len)
{
	if (!pFileName || !pContent || 0 == len || 0 == newFullSize) {
		return -2;
	}
	size_t curSz = DDRSys::getFileSize(pFileName);
	bool bRefresh = (oriFullSize != newFullSize);
	if (bRefresh && sPos != 0) {
		return -2;
	} else if (!bRefresh && sPos > curSz) {
		return -2;
	}
	std::ofstream ofs;
	if (bRefresh) { // updating whole file
		for (char *pC = pFileName; *pC; ++pC) {
			if ('/' == *pC || '\\' == *pC) {
				*pC = '\0';
				DDRSys::createDir(pFileName);
				*pC = '/';
			}
		}
		ofs.open(pFileName, std::ios::binary);
	} else { // modifying
		ofs.open(pFileName, std::ios::binary | std::ios::in | std::ios::out);
		ofs.seekp(sPos);
	}
	ofs.write(pContent, len);
	return (ofs.good() ? 0 : (-1));
}

int _readFile(const char *pFileName, void *pBuffer, size_t sPos,
	          size_t ePos, size_t *pFileSz)
{
	std::ifstream ifs(pFileName, std::ios::binary | std::ios::ate);
	if (!ifs.is_open()) {
		return -1;
	}
	if (ePos <= sPos || ePos > (size_t)ifs.tellg()) {
		return -2;
	}
	if (pFileSz) {
		*pFileSz = ifs.tellg();
	}
	ifs.seekg(sPos);
	return (ifs.read((char*)pBuffer, ePos - sPos) ? 0 : -1);
}

unsigned __int64 _toU64(const char *pStr)
{
	if (!pStr) {
		return 0;
	}
	unsigned __int64 seg[2] = { 0, 0 };
	int curSeg = 0;
	for (; *pStr; ++pStr) {
		if (*pStr >= '0' && *pStr <= '9') {
			seg[curSeg] = seg[curSeg] * 10 + (*pStr - '0');
			if (seg[curSeg] > (unsigned __int64)0xFFFFFFFF) {
				return 0;
			}
		} else if ('_' == *pStr) {
			++curSeg;
			if (curSeg > 1) {
				return 0;
			}
		} else {
			return 0;
		}
	}
	return ((seg[0] << 32) | seg[1]);
}

std::string _fromU64(unsigned __int64 id)
{
	std::string ret = std::to_string((unsigned int)(id >> 32));
	ret += "_";
	ret += std::to_string((unsigned int)id);
	return ret;
}

}

