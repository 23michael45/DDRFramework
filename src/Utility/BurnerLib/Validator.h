#ifndef __DDR_SOFT_VALIDATOR_H_INCLUDED__
#define __DDR_SOFT_VALIDATOR_H_INCLUDED__

#include <vector>

namespace DDRGadgets {

/*
Call this function to extract encrypted info string.
��ȡ���ܵ���Ϣ�ַ���, �ɹ�����ʱ�������info��
*/
bool GetEncInfo(std::vector<char> &info);

bool IsUnburnedState();

bool CheckMacMatch(const char *pMacStr);

}

#endif // __DDR_SOFT_VALIDATOR_H_INCLUDED__
