#include <vector>
#include <iostream>
#include "zlib/zlib.h"

int main()
{
	char src[] = "Yes, this is a test string. It can grow exponentially verbose, but I decided to start from here, without too many hassles...";

	std::vector<char> compBuf(sizeof(src));
	uLong compSz;
	int ret = compress((Bytef*)&compBuf[0], &compSz, (const Bytef*)src, sizeof(src));
	if (Z_OK != ret) {
		return -1;
	}
	compBuf.resize(compSz);

	std::vector<char> oriBuf(sizeof(src));
	uLong uncompSz = 125;
	ret = uncompress((Bytef*)&oriBuf[0], &uncompSz, (const Bytef*)&compBuf[0], compBuf.size());

	return 0;
}