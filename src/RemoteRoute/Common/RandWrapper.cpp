#include "RandWrapper.h"

#include <random>

namespace DDRSys {

long long _rand_i64()
{
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> dist(0, 65535);
	long long ret;
	((unsigned short*)(&ret))[0] = dist(mt);
	((unsigned short*)(&ret))[1] = dist(mt);
	((unsigned short*)(&ret))[2] = dist(mt);
	((unsigned short*)(&ret))[3] = dist(mt);
	return ret;
}

unsigned long long _rand_u64()
{
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> dist(0, 65535);
	unsigned long long ret;
	((unsigned short*)(&ret))[0] = dist(mt);
	((unsigned short*)(&ret))[1] = dist(mt);
	((unsigned short*)(&ret))[2] = dist(mt);
	((unsigned short*)(&ret))[3] = dist(mt);
	return ret;
}

}