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

��ָ���ַ�����д��Ŀ���ļ�.
(pInfo2Burn, nInfoLen) ָ�����ַ�������. ע���ַ������Ȳ��ó���100�ֽ�.
pTarExeFile ָ����Ŀ���ļ��ļ���
*/
BURN_RESULT BurnExecutable(const char *pInfo2Burn, int nInfoLen,
	                       const char *pTarExeFile);

std::string GetAnyMac();

}

#endif // __DDR_SOFT_BURNER_H_INCLUDED__
