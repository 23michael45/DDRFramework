#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "Validator.h"

#include <iostream>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")

namespace DDRGadgets {

static char s_str0[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ";
static char s_str1[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ";
static char s_str2[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ";
static int s_stdStrLen = 130;
static const int s_StrRep = 3;

static bool __Decrypt(const void *pSrc, int lenSrc,
	                  void *pTar, int lenTar);

bool IsUnburnedState()
{
	const char* addrs[] = { s_str0, s_str1, s_str2 };
	for (int i = 0; i < s_StrRep; ++i) {
		for (int j = 0; j < s_stdStrLen; ++j) {
			if ((char)('A' + (j % 26)) != addrs[i][j]) {
				return false;
			}
		}
	}
	return true;
}

bool GetEncInfo(std::vector<char> &info)
{
	std::vector<char> ff0, ff_;
	const char* addrs[] = { s_str0, s_str1, s_str2 };
	int infoLen = 0;
	for (int i = 0; i < s_StrRep; ++i) {
		const char *pPtr = addrs[i];
		if (!pPtr) {
			return false;
		}
		int arr[2];
		if (!__Decrypt(pPtr, 12, arr, 8) || arr[0] < 12 ||
			arr[1] <= 0 || arr[0] + arr[1] > s_stdStrLen) {
			return false;
		}
		if (0 == i) {
			infoLen = arr[1];
			ff0.resize(infoLen);
			if (!__Decrypt(pPtr + arr[0], s_stdStrLen - arr[0], &ff0[0], infoLen)) {
				return false;
			}
		} else {
			if (infoLen != arr[1]) {
				return false;
			}
			ff_.resize(infoLen);
			if (!__Decrypt(pPtr + arr[0], s_stdStrLen - arr[0], &ff_[0], infoLen)) {
				return false;
			}
			for (int i = 0; i < infoLen; ++i) {
				if (ff0[i] != ff_[i]) {
					return false;
				}
			}
		}
	}
	info.swap(ff0);
	return true;
}

bool CheckMacMatch(const char *pMacStr)
{
	IP_ADAPTER_INFO AdapterInfos[32];
	ULONG outBufLen = sizeof(AdapterInfos);
	DWORD dwRetVal = GetAdaptersInfo(AdapterInfos, &outBufLen);
	if (NO_ERROR != dwRetVal) {
		return "";
	}

	std::string str;
	for (int i = 0; i < outBufLen / sizeof(IP_ADAPTER_INFO); ++i) {
		str.resize(AdapterInfos[i].AddressLength * 3 - 1);
		int curInd = 0;
		for (int j = 0; j < (int)AdapterInfos[0].AddressLength; ++j) {
			if (j < (int)AdapterInfos[0].AddressLength - 1) {
				sprintf(&str[curInd], "%02x:", (int)AdapterInfos[i].Address[j]);
				curInd += 3;
			} else {
				sprintf(&str[curInd], "%02x", (int)AdapterInfos[i].Address[j]);
				curInd += 2;
			}
		}
		if (0 == strcmp(str.c_str(), pMacStr)) {
			return true;
		}
	}
	
	return false;
}

#if 1
static const int nPrimes = 168;
static const int Primes[168] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
73, 79, 83, 89, 97, 101, 103, 107, 109, 113,
127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
179, 181, 191, 193, 197, 199, 211, 223, 227, 229,
233, 239, 241, 251, 257, 263, 269, 271, 277, 281,
283, 293, 307, 311, 313, 317, 331, 337, 347, 349,
353, 359, 367, 373, 379, 383, 389, 397, 401, 409,
419, 421, 431, 433, 439, 443, 449, 457, 461, 463,
467, 479, 487, 491, 499, 503, 509, 521, 523, 541,
547, 557, 563, 569, 571, 577, 587, 593, 599, 601,
607, 613, 617, 619, 631, 641, 643, 647, 653, 659,
661, 673, 677, 683, 691, 701, 709, 719, 727, 733,
739, 743, 751, 757, 761, 769, 773, 787, 797, 809,
811, 821, 823, 827, 829, 839, 853, 857, 859, 863,
877, 881, 883, 887, 907, 911, 919, 929, 937, 941,
947, 953, 967, 971, 977, 983, 991, 997 };
#endif

bool __Decrypt(const void *pSrc, int lenSrc, void *pTar, int lenTar)
{
	int x, aa, bb, cc, c_;
	const unsigned char *p1;
	unsigned char val;
	if (!pSrc || lenSrc <= 0 || !pTar || lenSrc < lenTar + 4) {
		// one unsigned char index, two unsigned chars check
		return false;
	}
	const unsigned char *pCode = (const unsigned char*)pSrc;
	unsigned char *pTxt = (unsigned char*)pTar;
	x = *(unsigned short*)(pCode + lenTar);
	aa = 0;
	bb = nPrimes - 1;
	while (bb > aa + 1) {
		if (x == Primes[aa]) {
			cc = aa;
			bb = aa;
			break;
		} else if (x == Primes[bb]) {
			cc = bb;
			aa = bb;
			break;
		}
		cc = (aa + bb) >> 1;
		if (x == Primes[cc]) {
			aa = bb = cc;
			break;
		} else if (x > Primes[cc]) {
			aa = cc;
		} else {
			bb = cc;
		}
	}
	if (aa != bb) {
		if (x - Primes[aa] > Primes[bb] - x) {
			cc = bb;
		} else {
			cc = aa;
		}
	}

	p1 = pCode;
	c_ = cc;
	bb = 0;
	for (aa = 0; aa < lenTar; ++aa) {
		val = (unsigned char)(*pCode - ((Primes[(cc + aa) % nPrimes] * Primes[(cc + aa) % nPrimes]) & 0xFF));
		bb += val * val;
		bb &= 0xFFFF;
		cc = (cc + 1) % nPrimes;
		++pCode;
	}

	if (*(unsigned short*)(pCode + 2) != bb) {
		return false;
	}
	pCode += 4;
	for (aa += 4; aa < lenSrc; ++aa) {
		bb += (pCode[-4] * pCode[-4]);
		bb &= 0xFFFF;
		if (*pCode != (unsigned char)((bb >> 8) ^ (bb & 0xFF) ^
			(Primes[(cc + aa) % nPrimes] & 0xFF))) {
			return false;
		}
		++pCode;
	}
	cc = c_;
	pCode = p1;
	for (aa = 0; aa < lenTar; ++aa) {
		val = (unsigned char)(*pCode - ((Primes[(cc + aa) % nPrimes] *
			Primes[(cc + aa) % nPrimes]) & 0xFF));
		*pTxt = (unsigned char)(val - (Primes[cc] & 0xFF));
		cc = (cc + 1) % nPrimes;
		++pCode, ++pTxt;
	}
	return true;
}

}
