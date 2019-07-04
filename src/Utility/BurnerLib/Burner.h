#ifndef __DDR_SOFT_BURNER_H_INCLUDED__
#define __DDR_SOFT_BURNER_H_INCLUDED__

#include <vector>
#include <string>

namespace DDRGadgets {

enum BURN_RESULT {
	SUCCESSFUL,
	INVALID_PIECE_SPEC,
	PIECES_TOO_LONG,
	TARGET_FILE_IO_ERROR,
	TARGET_FILE_FORMAT_ILLEGAL
};

/*
Burn certain info string into a file.
(pInfo2Burn, nInfoLen) specify a string to burn. Note the length (nInfoLen)
should not be exceeding 100.
pTarExeFile specifies the target file name.

将指定字符串烧写入目标文件.
(pInfo2Burn, nInfoLen) 指定了字符串内容. 注意字符串长度不得超过100字节.
pTarExeFile 指定了目标文件文件名
*/
BURN_RESULT BurnExecutable(const char *pInfo2Burn, int nInfoLen,
	                       const char *pTarExeFile);

std::string GetAnyMac();

}

#endif // __DDR_SOFT_BURNER_H_INCLUDED__
