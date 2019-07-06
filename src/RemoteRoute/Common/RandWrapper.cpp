#include "RandWrapper.h"

#include <random>

namespace DDRSys {

__int64 _rand_i64()
{
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> dist(0, 65535);
	__int64 ret;
	((unsigned short*)(&ret))[0] = dist(mt);
	((unsigned short*)(&ret))[1] = dist(mt);
	((unsigned short*)(&ret))[2] = dist(mt);
	((unsigned short*)(&ret))[3] = dist(mt);
	return ret;
}

unsigned __int64 _rand_u64()
{
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> dist(0, 65535);
	unsigned __int64 ret;
	((unsigned short*)(&ret))[0] = dist(mt);
	((unsigned short*)(&ret))[1] = dist(mt);
	((unsigned short*)(&ret))[2] = dist(mt);
	((unsigned short*)(&ret))[3] = dist(mt);
	return ret;
}

}