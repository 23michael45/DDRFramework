#ifndef __DDR_CLOUD_SERVICE_LIB_SERVER_SIDE_COMMON_FUNCS_H_INCLUDED__
#define __DDR_CLOUD_SERVICE_LIB_SERVER_SIDE_COMMON_FUNCS_H_INCLUDED__

#include <string>

namespace DDRCloudService {

bool _isCompressable(const char *pFileName);

// -2 - file size mismatch
// -1 - failure to open or write file
// 0 - successful
int _writeFile(char *pFileName, size_t oriFullSize, size_t newFullSize,
	           const char *pContent, size_t sPos, size_t len);

// -2 - position error
// -1 - failure to open or read file
// 0 - successful
int _readFile(const char *pFileName, void *pBuffer, size_t sPos,
	          size_t ePos, size_t *pFileSz);

unsigned __int64 _toU64(const char *pStr);

std::string _fromU64(unsigned __int64 id);

}

#endif // __DDR_CLOUD_SERVICE_LIB_SERVER_SIDE_COMMON_FUNCS_H_INCLUDED__
