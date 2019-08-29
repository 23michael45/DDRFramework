#ifndef __DDR_SOFT_VALIDATOR_H_INCLUDED__
#define __DDR_SOFT_VALIDATOR_H_INCLUDED__

#include <vector>

namespace DDRGadgets {

/*
Call this function to extract encrypted info string.
获取加密的信息字符串, 成功返回时将填充至info中
*/
bool GetEncInfo(std::vector<char> &info);

bool IsUnburnedState();

bool CheckMacMatch(const char *pMacStr);

}

#endif // __DDR_SOFT_VALIDATOR_H_INCLUDED__
