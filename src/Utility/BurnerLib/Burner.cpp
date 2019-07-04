#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "Burner.h"

#include <vector>
#include <string>
#include <fstream>
#include <thread>
#include <chrono>
#include <random>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")

namespace DDRGadgets {

const size_t N_STR_REPS = 3;
static const size_t STD_STR_LEN = 130;
const int MIN_EXTRA_LEN_FOR_ENC = 10;

static bool __WriteBuffer(char *pTarBuf, int tarBufLen,
	                      const char *pTxt2Enc, int txtLen,
	                      int minExtraLen);

BURN_RESULT BurnExecutable(const char *pInfo2Burn, int nInfoLen,
	                       const char *pTarExeFile)
{
	if (!pInfo2Burn || nInfoLen <= 0) {
		return BURN_RESULT::INVALID_PIECE_SPEC;
	}
	if (12 + nInfoLen + MIN_EXTRA_LEN_FOR_ENC > (int)STD_STR_LEN) {
		return BURN_RESULT::PIECES_TOO_LONG;
	}
	std::string std_str(STD_STR_LEN, 'A');
	for (int i = 0; i < (int)STD_STR_LEN; ++i) {
		std_str[i] = (i % 26) + 'A';
	}

	std::ifstream ifs(pTarExeFile, std::ios::binary);
	if (!ifs.is_open()) {
		return BURN_RESULT::TARGET_FILE_IO_ERROR;
	}
	ifs.seekg(0, std::ios::end);
	size_t fSz = ifs.tellg();
	if (fSz <= 0) {
		return BURN_RESULT::TARGET_FILE_IO_ERROR;
	}
	std::vector<char> fileContent(fSz);
	ifs.seekg(0, std::ios::beg);
	if (!ifs.read(&fileContent[0], fSz)) {
		return BURN_RESULT::TARGET_FILE_IO_ERROR;
	}
	ifs.close();

	std::vector<char*> txtPos;
	for (size_t i = 0; i < fSz; ) {
		if (0 == memcmp(&fileContent[i], std_str.c_str(), std_str.length())) {
			txtPos.emplace_back(&fileContent[i]);
			if (txtPos.size() > N_STR_REPS) {
				break;
			}
			i += std_str.length();
		} else {
			++i;
		}
	}
	//std::cout << "txtPos.size() = " << txtPos.size() << std::endl;
	if (txtPos.size() != N_STR_REPS) {
		return BURN_RESULT::TARGET_FILE_FORMAT_ILLEGAL;
	}

	for (size_t i = 0; i < N_STR_REPS; ++i) {
		if (!__WriteBuffer(txtPos[i], (int)std_str.length(),
			               pInfo2Burn, nInfoLen, MIN_EXTRA_LEN_FOR_ENC)) {
			//std::cout << "BURN_RESULT::TARGET_FILE_IO_ERROR" << std::endl;
			return BURN_RESULT::TARGET_FILE_IO_ERROR;
		}
	}
	std::ofstream ofs(pTarExeFile, std::ios::binary);
	if (!ofs.is_open()) {
		return BURN_RESULT::TARGET_FILE_IO_ERROR;
	}
	if (!ofs.write(&fileContent[0], fSz)) {
		return BURN_RESULT::TARGET_FILE_IO_ERROR;
	}
	ofs.close();
	return BURN_RESULT::SUCCESSFUL;
}

std::string GetAnyMac()
{
	IP_ADAPTER_INFO AdapterInfos[2];
	ULONG outBufLen = sizeof(AdapterInfos);
	DWORD dwRetVal = GetAdaptersInfo(AdapterInfos, &outBufLen);
	if (NO_ERROR != dwRetVal) {
		return "";
	}
	std::string str(3 * AdapterInfos[0].AddressLength - 1, '\0');
	int curInd = 0;
	for (int i = 0; i < (int)AdapterInfos[0].AddressLength; ++i) {
		if (i < (int)AdapterInfos[0].AddressLength - 1) {
			sprintf(&str[curInd], "%02x:", (int)AdapterInfos[0].Address[i]);
			curInd += 3;
		} else {
			sprintf(&str[curInd], "%02x", (int)AdapterInfos[0].Address[i]);
			curInd += 2;
		}
	}
	return str;
}

static bool __Encrypt(const void *pSrc, int lenSrc,
	           const void *pTar, int lenTar);

bool __WriteBuffer(char *pTarBuf, int tarBufLen,
	               const char *pTxt2Enc, int txtLen,
	               int minExtraLen)
{
	if (!pTarBuf || tarBufLen <= 0 || !pTxt2Enc || txtLen <= 0 || minExtraLen <= 4 ||
		12 + txtLen + minExtraLen > tarBufLen) {
		return false;
	}
	std::random_device rd;
	std::mt19937 mt(rd());
	int aa = 12;
	int bb = tarBufLen - txtLen - minExtraLen;
	std::uniform_int_distribution<int> dist(aa, bb);
	std::uniform_int_distribution<int> dist2(0, 255);
	char offset_txtLen[8];
	((int32_t*)offset_txtLen)[0] = dist(mt);
	((int32_t*)offset_txtLen)[1] = txtLen;
	__Encrypt(offset_txtLen, 8, pTarBuf, 12);
	for (int i = 12; i < ((int32_t*)offset_txtLen)[0]; ++i) {
		pTarBuf[i] = (char)dist2(mt);
	}
	//std::this_thread::sleep_for(std::chrono::milliseconds(dist2(mt) + 100));
	return __Encrypt(pTxt2Enc, txtLen, pTarBuf + ((int32_t*)offset_txtLen)[0],
		             tarBufLen - ((int32_t*)offset_txtLen)[0]);
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

bool __Encrypt(const void *pSrc, int lenSrc,
	           const void *pTar, int lenTar)
{
	if (!pSrc || lenSrc <= 0 || !pTar) { return 0; }
	if (lenTar < lenSrc + 4 || lenSrc <= 0) {
		return false;
	}
	const unsigned char *pTxt = (const unsigned char*)pSrc;
	unsigned char *pCode = (unsigned char*)pTar;

	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> dist(0, 999);
	std::uniform_int_distribution<int> dist2(0, 255);

	int ms, aa, bb, cc;
	ms = dist(mt);
	aa = 0;
	bb = nPrimes - 1;
	while (bb > aa + 1) {
		if (ms == Primes[aa]) {
			cc = aa;
			bb = aa;
			break;
		} else if (ms == Primes[bb]) {
			cc = bb;
			aa = bb;
			break;
		}
		cc = (aa + bb) >> 1;
		if (ms == Primes[cc]) {
			aa = bb = cc;
			break;
		} else if (ms > Primes[cc]) {
			aa = cc;
		} else {
			bb = cc;
		}
	}
	if (aa != bb) {
		if (ms - Primes[aa] > Primes[bb] - ms) {
			cc = bb;
		} else {
			cc = aa;
		}
	}
	bb = 0;
	for (aa = 0; aa < lenSrc; ++aa) {
		*pCode = (unsigned char)(((int)(*pTxt) + Primes[cc]) & 0xFF);
		bb += (*pCode) * (*pCode);
		*pCode = ((*pCode) + ((Primes[(cc + aa) % nPrimes] *
			Primes[(cc + aa) % nPrimes]) & 0xFF)) & 0xFF;
		bb &= 0xFFFF;
		++pCode, ++pTxt;
		cc = (cc + 1) % nPrimes;
	}
	*(unsigned short*)pCode = ms;
	pCode += 2;
	*(unsigned short*)pCode = (unsigned short)bb;
	pCode += 2;
	for (aa += 4; aa < lenTar; ++aa) {
		bb += (pCode[-4] * pCode[-4]);
		bb &= 0xFFFF;
		*pCode = (unsigned char)((bb >> 8) ^ (bb & 0xFF) ^ (Primes[(cc + aa) % nPrimes] & 0xFF));
		++pCode;
	}
	return true;
}

}

