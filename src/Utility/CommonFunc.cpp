#include "CommonFunc.h"

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)
#endif
#include <locale.h>
#include <vector>
#include <cstdlib>
#include <codecvt>
#include <locale>
#include <ctime>
#include <thread>
#include <chrono>
#include <codecvt>
#include <regex>
#include <sys/types.h>
#include <sys/stat.h>
#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#include <direct.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(__linux__)
#include <stdio.h>
#include <utime.h>
#include <sys/time.h>
#include <unistd.h>
#include <utime.h>
#include <glob.h>
#include <fcntl.h>
#include <termios.h>
#include <iconv.h>
#endif
#include "MsgStruct.h"

namespace DDRFramework {

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

	bool Txt_Encrypt(const void *pSrc, int lenSrc,
		const void *pTar, int lenTar) {
		if (!pSrc || lenSrc <= 0 || !pTar) { return 0; }
		if (lenTar < lenSrc + 4 || lenSrc <= 0) {
			// one char index, two chars check
			return false;
		}
		const unsigned char *pTxt = (const unsigned char*)pSrc;
		unsigned char *pCode = (unsigned char*)pTar;

		int ms, aa, bb, cc;
		//SYSTEMTIME st;
		//::GetSystemTime(&st);
		//ms = st.wMilliseconds;
		ms = (int)(std::chrono::duration_cast<std::chrono::milliseconds>
			(std::chrono::system_clock::now().time_since_epoch()).count() % 1000);
		aa = 0;
		bb = nPrimes - 1;
		while (bb > aa + 1) {
			if (ms == Primes[aa]) {
				cc = aa;
				bb = aa;
				break;
			}
			else if (ms == Primes[bb]) {
				cc = bb;
				aa = bb;
				break;
			}
			cc = (aa + bb) >> 1;
			if (ms == Primes[cc]) {
				aa = bb = cc;
				break;
			}
			else if (ms > Primes[cc]) {
				aa = cc;
			}
			else {
				bb = cc;
			}
		}
		if (aa != bb) {
			if (ms - Primes[aa] > Primes[bb] - ms) {
				cc = bb;
			}
			else {
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

	bool Txt_Encrypt2(const void *pSrc1, int len1,
		const void *pSrc2, int len2,
		void *pTar, int lenTar) {
		if (!pSrc1 || len1 <= 0 || !pSrc2 || len2 <= 0) { return 0; }
		if (!pTar || lenTar < len1 + len2 + 4) {
			// one char index, two chars check
			return false;
		}
		int lenTxt = len1 + len2;
		const unsigned char *pTxt1 = (const unsigned char*)pSrc1;
		const unsigned char *pTxt2 = (const unsigned char*)pSrc2;
		unsigned char *pCode = (unsigned char*)pTar;

		int ms, aa, bb, cc;
		//SYSTEMTIME st;
		//::GetSystemTime(&st);
		//ms = st.wMilliseconds;
		ms = (int)(std::chrono::duration_cast<std::chrono::milliseconds>
			(std::chrono::system_clock::now().time_since_epoch()).count() % 1000);
		aa = 0;
		bb = nPrimes - 1;
		while (bb > aa + 1) {
			if (ms == Primes[aa]) {
				cc = aa;
				bb = aa;
				break;
			}
			else if (ms == Primes[bb]) {
				cc = bb;
				aa = bb;
				break;
			}
			cc = (aa + bb) >> 1;
			if (ms == Primes[cc]) {
				aa = bb = cc;
				break;
			}
			else if (ms > Primes[cc]) {
				aa = cc;
			}
			else {
				bb = cc;
			}
		}
		if (aa != bb) {
			if (ms - Primes[aa] > Primes[bb] - ms) {
				cc = bb;
			}
			else {
				cc = aa;
			}
		}
		const unsigned char *pTxt = pTxt1;
		bb = 0;
		for (aa = 0; aa < lenTxt; ++aa) {
			*pCode = (unsigned char)(((int)(*pTxt) + Primes[cc]) & 0xFF);
			bb += (*pCode) * (*pCode);
			*pCode = ((*pCode) + ((Primes[(cc + aa) % nPrimes] *
				Primes[(cc + aa) % nPrimes]) & 0xFF)) & 0xFF;
			bb &= 0xFFFF;
			++pCode, ++pTxt;
			cc = (cc + 1) % nPrimes;
			if (len1 - 1 == aa) {
				pTxt = pTxt2;
			}
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

	bool Txt_Decrypt(const void *pSrc, int lenSrc,
		void *pTar, int lenTar) {
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
			}
			else if (x == Primes[bb]) {
				cc = bb;
				aa = bb;
				break;
			}
			cc = (aa + bb) >> 1;
			if (x == Primes[cc]) {
				aa = bb = cc;
				break;
			}
			else if (x > Primes[cc]) {
				aa = cc;
			}
			else {
				bb = cc;
			}
		}
		if (aa != bb) {
			if (x - Primes[aa] > Primes[bb] - x) {
				cc = bb;
			}
			else {
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

	I64 EncryptDataLen_64(unsigned int len) {
		I64 x = (I64)(~len);
		x <<= 32;
		x |= len;
		return x;
	}

	bool VerifyDataLen_64(const void *pDataHead, unsigned int *plen) {
		*plen = *((unsigned int*)pDataHead);
		return (*((unsigned int*)pDataHead + 1) == ~(*plen));
	}

	int EncryptDataLen_32(int len) {
		if (len > 0xFFFFFF || len < 0) { return -1; }
		int ret = (len >> 16) ^ (len >> 8) ^ (len & 0xFF);
		if (len & 0xFF0000) {
			ret = len | ((~ret) << 24);
		}
		else {
			ret = len | (ret << 24);
		}
		return ret;
	}

	bool VerifyDataLen_32(const void *pDataHead, int *plen)
	{
		const unsigned char *pC = (const unsigned char*)pDataHead;
		*plen = pC[0] | (pC[1] << 8) | (pC[2] << 16);
		unsigned char vv = pC[0] ^ pC[1] ^ pC[2];
		if (pC[2] != 0) {
			vv = 255 - vv;
		}
		if (pC[3] == vv) {
			return true;
		}
		else {
			*plen = -1;
			return false;
		}
	}

	void ConvertUIP2Str_IPv4(U32 ip, char *pStr, int *pEndingPos)
	{
		char *pStrHead = pStr;
		for (int i = 0; i < 4; ++i) {
			BYTE seg = ((BYTE*)(&ip))[i];
			bool bSegStarted = false;
			if (seg / 100 > 0) {
				(*pStr++) = (int)(seg / 100) + '0';
				bSegStarted = true;
			}
			if ((seg % 100) / 10 > 0 || bSegStarted) {
				(*pStr++) = (int)((seg % 100) / 10) + '0';
			}
			(*pStr++) = (int)(seg % 10) + '0';
			if (i < 3) {
				(*pStr++) = '.';
			}
		}
		*pStr = '\0';
		if (pEndingPos) {
			*pEndingPos = pStr - pStrHead;
		}
	}

	bool ConvertStr2UIP_IPv4(const char *pStr, U32 *ip,
		char cDelimiter, int *pEndingPos)
	{
		*ip = 0;
		const char *pStrHead = pStr;
		BYTE *seg = (BYTE*)ip;
		int segPos = 0, segVal = 0;
		for (; *pStr != cDelimiter && *pStr != '\0'; ++pStr) {
			if (*pStr >= '0' && *pStr <= '9') {
				segVal = segVal * 10 + (*pStr - '0');
			}
			else if (*pStr == '.') {
				if (segVal > 255) { return false; }
				else {
					seg[segPos] = (BYTE)segVal;
					segVal = 0;
				}
				if (++segPos > 3) {
					return false;
				}
			}
			else { return false; }
		}
		seg[segPos] = (BYTE)segVal;
		if (pEndingPos) {
			*pEndingPos = pStr - pStrHead;
		}
		return true;
	}

	bool ConvertStr2Int(const char *pStr, int *x,
		char cDelimiter, int *pEndingPos) {
		const char *pStrHead = pStr;
		for ((*x) = 0; *pStr != '\0' && *pStr != cDelimiter; ++pStr) {
			if (*pStr < '0' || *pStr > '9') {
				return false;
			}
			(*x) = (*x) * 10 + (*pStr - '0');
		}
		if (pEndingPos) {
			*pEndingPos = pStr - pStrHead;
		}
		return true;
	}

	void ConvertInt2Str(int x, char *pStr, int *pEndingPos) {
		bool bNeg = false;
		if (x < 0) {
			bNeg = true;
			x = -x;
		}
		int pos;
		if (x != 0) {
			for (pos = 0; x > 0; x = x / 10, ++pos) {
				pStr[pos] = (x % 10) + '0';
			}
			if (bNeg) {
				pStr[pos++] = '-';
			}
			for (int a = 0, b = pos - 1; a < b; ++a, --b) {
				char t = pStr[a];
				pStr[a] = pStr[b];
				pStr[b] = t;
			}
			pStr[pos] = '\0';
		}
		else {
			pStr[0] = '0';
			pStr[1] = '\0';
			pos = 1;
		}
		if (pEndingPos) {
			*pEndingPos = pos;
		}
	}

	U16 ConvertHTONS(U16 u16val) {
		return (((u16val & 0xFF) << 8) | ((u16val >> 8) & 0xFF));
	}

	int KeyTest()
	{
#ifdef _WIN32
		HANDLE keyboard = GetStdHandle(STD_INPUT_HANDLE);
		DWORD dw, num;
		if (!::GetNumberOfConsoleInputEvents(keyboard, &num) || num == 0) {
			return 0;
		}
		for (int i = 0; i < (int)num; ++i) {
			INPUT_RECORD input;
			::ReadConsoleInputA(keyboard, &input, 1, (LPDWORD)(&dw));
			if (input.EventType == KEY_EVENT && !input.Event.KeyEvent.bKeyDown) {
				//return (int)(input.Event.KeyEvent.uChar.AsciiChar);
				return (int)(input.Event.KeyEvent.wVirtualKeyCode);
			}
		}
		return 0;
#endif
#ifdef __linux__
		struct termios oldt, newt;
		int c, oldf;
		tcgetattr(STDIN_FILENO, &oldt);
		newt = oldt;
		newt.c_lflag &= ~(ICANON | ECHO);
		tcsetattr(STDIN_FILENO, TCSANOW, &newt);
		oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
		fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
		c = getchar();
		tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
		fcntl(STDIN_FILENO, F_SETFL, oldf);
		if (EOF != c) {
			if (c >= 'a' && c <= 'z') {
				return (c - 'a' + 'A');
			}
			else {
				return c;
			}
		}
		else {
			return 0;
		}
#endif
	}

	void Sleep(int nMillisec)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(nMillisec));
	}

#ifdef __linux__

	int code_convert(char *from_charset, char *to_charset,
		char *inbuf, int inlen,
		char *outbuf, int outlen)
	{
		iconv_t cd = iconv_open(to_charset, from_charset);
		if (cd == 0) {
			return -1;
		}
		size_t iLen = (size_t)inlen;
		size_t oLen = (size_t)outlen;
		if (iconv(cd, &inbuf, &iLen, &outbuf, &oLen) < 0) {
			return -1;
		}
		*outbuf = '\0';
		iconv_close(cd);
		return (outlen - (int)oLen);
	}

	int StrConv_g2u(char *inbuf, int inlen, char *outbuf, int outlen)
	{
		return code_convert("gb2312", "utf-8", inbuf, inlen, outbuf, outlen);
	}

	int StrConv_u2g(char *inbuf, int inlen, char *outbuf, int outlen)
	{
		return code_convert("utf-8", "gb2312", inbuf, inlen, outbuf, outlen);
	}
#endif

	bool GetUTCTimeStr(char *pBuf, int nBufCap, int *pLen) {
		if (nBufCap < 24) {
			return false;
		}
		int pos = 0;
		std::time_t t = std::time(0);
		std::tm *now = std::gmtime(&t);
		int ms = (int)(std::chrono::duration_cast<std::chrono::milliseconds>
			(std::chrono::system_clock::now().time_since_epoch()).count() % 1000);
		pos += snprintf(pBuf, nBufCap - pos, "%04d/%02d/%02d-%02d:%02d:%02d.%03d",
			now->tm_year + 1900, now->tm_mon + 1,
			now->tm_mday, now->tm_hour,
			now->tm_min, now->tm_sec, ms);
		pBuf[pos] = '\0';
		if (pLen) {
			*pLen = pos;
		}
		return true;
	}

	bool GetLocalTimeStr(char *pBuf, int nBufCap, int *pLen) {
		if (nBufCap < 24) {
			return false;
		}
		int pos = 0;
		std::time_t t = std::time(0);
		std::tm *now = std::localtime(&t);
		int ms = (int)(std::chrono::duration_cast<std::chrono::milliseconds>
			(std::chrono::system_clock::now().time_since_epoch()).count() % 1000);
		pos += snprintf(pBuf, nBufCap - pos, "%04d/%02d/%02d-%02d:%02d:%02d.%03d",
			now->tm_year + 1900, now->tm_mon + 1,
			now->tm_mday, now->tm_hour,
			now->tm_min, now->tm_sec, ms);
		pBuf[pos] = '\0';
		if (pLen) {
			*pLen = pos;
		}
		return true;
	}


	std::string MBToUTF8String(std::string mbstr)
	{

		std::vector<char> utf8;
		bool b = DDRFramework::MBToUTF8(utf8, mbstr.c_str(), mbstr.length());
		if (b == false)
		{
			return mbstr;
		}
		else
		{
			std::string full(utf8.begin(), utf8.end());
			return full;
		}
	}
	std::string UTF8ToMBString(std::string utf8str)
	{

		std::vector<char> mb;
		bool b = DDRFramework::UTF8ToMB(mb, utf8str.c_str(), utf8str.length());
		if (b == false)
		{
			return utf8str;
		}
		else
		{
			std::string full(mb.begin(), mb.end());
			return full;
		}
	}

	bool MBToUTF8(std::vector<char>& pu8, const char* pmb, int mLen)
	{
		// convert an MBCS string to widechar   
		int nLen = MultiByteToWideChar(CP_ACP, 0, pmb, mLen, NULL, 0);

		WCHAR* lpszW = NULL;
		try
		{
			lpszW = new WCHAR[nLen];
		}
		catch (std::bad_alloc &memExp)
		{
			return false;
		}

		int nRtn = MultiByteToWideChar(CP_ACP, 0, pmb, mLen, lpszW, nLen);

		if (nRtn != nLen)
		{
			delete[] lpszW;
			return false;
		}
		// convert an widechar string to utf8  
		int utf8Len = WideCharToMultiByte(CP_UTF8, 0, lpszW, nLen, NULL, 0, NULL, NULL);
		if (utf8Len <= 0)
		{
			return false;
		}
		pu8.resize(utf8Len);
		nRtn = WideCharToMultiByte(CP_UTF8, 0, lpszW, nLen, &*pu8.begin(), utf8Len, NULL, NULL);
		delete[] lpszW;

		if (nRtn != utf8Len)
		{
			pu8.clear();
			return false;
		}
		return true;
	}

	bool UTF8ToMB(std::vector<char>& pmb, const char* pu8, int utf8Len)
	{
		// convert an UTF8 string to widechar   
		int nLen = MultiByteToWideChar(CP_UTF8, 0, pu8, utf8Len, NULL, 0);

		WCHAR* lpszW = NULL;
		try
		{
			lpszW = new WCHAR[nLen];
		}
		catch (std::bad_alloc &memExp)
		{
			return false;
		}

		int nRtn = MultiByteToWideChar(CP_UTF8, 0, pu8, utf8Len, lpszW, nLen);

		if (nRtn != nLen)
		{
			delete[] lpszW;
			return false;
		}

		// convert an widechar string to Multibyte   
		int MBLen = WideCharToMultiByte(CP_ACP, 0, lpszW, nLen, NULL, 0, NULL, NULL);
		if (MBLen <= 0)
		{
			return false;
		}
		pmb.resize(MBLen);
		nRtn = WideCharToMultiByte(CP_ACP, 0, lpszW, nLen, &*pmb.begin(), MBLen, NULL, NULL);
		delete[] lpszW;

		if (nRtn != MBLen)
		{
			pmb.clear();
			return false;
		}
		return true;
	}

	bool MBToUnicode(std::vector<wchar_t>& pun, const char* pmb, int mLen)
	{
		// convert an MBCS string to widechar   
		int uLen = MultiByteToWideChar(CP_ACP, 0, pmb, mLen, NULL, 0);

		if (uLen <= 0)
		{
			return false;
		}
		pun.resize(uLen);

		int nRtn = MultiByteToWideChar(CP_ACP, 0, pmb, mLen, &*pun.begin(), uLen);

		if (nRtn != uLen)
		{
			pun.clear();
			return false;
		}
		return true;
	}

	bool UnicodeToMB(std::vector<char>& pmb, const wchar_t* pun, int uLen)
	{
		// convert an widechar string to Multibyte   
		int MBLen = WideCharToMultiByte(CP_ACP, 0, pun, uLen, NULL, 0, NULL, NULL);
		if (MBLen <= 0)
		{
			return false;
		}
		pmb.resize(MBLen);
		int nRtn = WideCharToMultiByte(CP_ACP, 0, pun, uLen, &*pmb.begin(), MBLen, NULL, NULL);

		if (nRtn != MBLen)
		{
			pmb.clear();
			return false;
		}
		return true;
	}

	bool UTF8ToUnicode(std::vector<wchar_t>& pun, const char* pu8, int utf8Len)
	{
		// convert an UTF8 string to widechar   
		int nLen = MultiByteToWideChar(CP_UTF8, 0, pu8, utf8Len, NULL, 0);
		if (nLen <= 0)
		{
			return false;
		}
		pun.resize(nLen);
		int nRtn = MultiByteToWideChar(CP_UTF8, 0, pu8, utf8Len, &*pun.begin(), nLen);

		if (nRtn != nLen)
		{
			pun.clear();
			return false;
		}

		return true;
	}

	bool UnicodeToUTF8(std::vector<char>& pu8, const wchar_t* pun, int uLen)
	{
		// convert an widechar string to utf8  
		int utf8Len = WideCharToMultiByte(CP_UTF8, 0, pun, uLen, NULL, 0, NULL, NULL);
		if (utf8Len <= 0)
		{
			return false;
		}
		pu8.resize(utf8Len);
		int nRtn = WideCharToMultiByte(CP_UTF8, 0, pun, uLen, &*pu8.begin(), utf8Len, NULL, NULL);

		if (nRtn != utf8Len)
		{
			pu8.clear();
			return false;
		}
		return true;
	}

	std::wstring s2w(const std::string& utf8string)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
		return conv.from_bytes(utf8string);
	}

	std::string w2s(const std::wstring& utf16Str)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
		return conv.to_bytes(utf16Str);
	}

	std::wstring StringToWString(const std::string& str)
	{
#if defined(WIN32)
		size_t sz = str.length();
		int nd = MultiByteToWideChar(CP_ACP, 0, &str[0], sz, NULL, 0);
		std::wstring ret(nd, 0);
		int w = MultiByteToWideChar(CP_ACP, 0, &str[0], sz, &ret[0], nd);
		if (str.length() != sz) {

			throw std::exception("StringToWString Err");
		}
		return ret;
#else
		const char* p = str.c_str();
		size_t len = str.length();
		size_t sz = len * sizeof(wchar_t);
		wchar_t* tp = new wchar_t[sz];
		size_t w = mbstowcs(tp, p, sz);
		if (w != len) {
			delete[] tp;
			throw std::exception("StringToWString Err");
		}
		std::wstring ret(tp);
		delete[] tp;
		return ret;
#endif
	}

	std::string WStringToString(const std::wstring& str)
	{
		size_t sz = str.length();
#if defined(WIN32)
		int nd = WideCharToMultiByte(CP_ACP, 0, &str[0], sz, NULL, 0, NULL, NULL);
		std::string ret(nd, 0);
		int w = WideCharToMultiByte(CP_ACP, 0, &str[0], sz, &ret[0], nd, NULL, NULL);
		/*if (ret.length() != sz) {
			throw std::exception("WStringToString Err");
		}*/
		return ret;
#else
		const wchar_t* p = str.c_str();
		char* tp = new char[sz];
		size_t w = wcstombs(tp, p, sz);
		if (w != sz) {
			delete[] tp;

			throw std::exception("WStringToString Err");
		}
		std::string ret(tp);
		delete[] tp;
		return ret;
#endif
	}
#ifdef _WINDOWS
	std::string getexepath()
	{
		wchar_t result[MAX_PATH];

		std::wstring wstr = std::wstring(result, GetModuleFileName(NULL, result, MAX_PATH));
		return WStringToString(wstr);
	}
#else 
#include <string>
#include <limits.h>
#include <unistd.h>
	std::string getexepath()
	{
		char result[PATH_MAX];
		ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
		return std::string(result, (count > 0) ? count : 0);
	}

#endif
	std::string getStarWildRegex(const std::string& s,bool bbegin,std::string root)
	{
		size_t pos = 0;
		std::vector<size_t> posvec;
		std::string sformat;
		if (bbegin)
		{
			if (!root.empty())
			{
				if (s.find('*') == 0)
				{
					sformat = "(" + root + ")(" + s;

					pos = root.length() + 3;
				}
				else
				{
					sformat = "(" + root + "/" + s;

					pos = root.length() + 2;
				}

			}
			else
			{
				sformat = "(.*)(" + s;

				pos = 5;
			}
		}
		else
		{

			sformat = "(" + s;
			pos = 1;
		}
		do
		{

			pos = sformat.find('*', pos);
			if (pos == std::string::npos)
			{
				sformat = sformat.insert(sformat.length(), ")");
				break;
			}

			sformat = sformat.insert(pos, ")(.");
			pos += 4;

			if (pos == sformat.length())
			{
				sformat = sformat.insert(pos, ")");
				break;

			}
			else if (pos < sformat.length())
			{

				sformat = sformat.insert(pos, ")(");
				pos += 3;
			}


		} while (pos > 0);


		sformat = replace_all(sformat, "()", "");
		sformat = replace_all(sformat, "(.)", "(\\.)");
		return sformat;
	}
	std::string replace_all(std::string& str, std::string sub, std::string replaceto)
	{
		size_t index = 0;
		while (true) {
			/* Locate the substring to replace. */
			index = str.find(sub, index);
			if (index == std::string::npos) break;

			/* Make the replacement. */
			str.replace(index, sub.length(), replaceto);

			/* Advance index forward so the next iteration doesn't pick it up as well. */
			index += sub.length();
		}
		return str;
	}

	std::vector<std::string> split(const std::string &text, char sep)
	{
		std::vector<std::string> tokens;
		std::size_t start = 0, end = 0;
		while ((end = text.find(sep, start)) != std::string::npos) {
			if (end != start) {
				tokens.push_back(text.substr(start, end - start));
			}
			start = end + 1;
		}
		if (end != start) {
			tokens.push_back(text.substr(start));
		}
		return tokens;
	}

	void DisableMouseSelectConsole()
	{
#ifdef _WINDOWS
		//SetConsoleOutputCP(CP_UTF8);
		//FreeConsole();
		HWND hwnd = GetConsoleWindow();

		HANDLE hStdOut = GetStdHandle(STD_INPUT_HANDLE);
		DWORD prev_mode;
		GetConsoleMode(hStdOut, &prev_mode);
		SetConsoleMode(hStdOut, prev_mode & ~ENABLE_QUICK_EDIT_MODE);
		//ShowWindow(hwnd, SW_SHOWMINNOACTIVE);
#endif
	}

	bool DDRDeleteFile(const char* szFile)
	{
#ifdef _WINDOWS
		if (!::DeleteFileA(szFile))
		{
			return false;
		}
		return true;
#else
		// this is linux system
		return false;
#endif
	}

	bool DDRRemoveDir(const char* szFileDir)
	{
#ifdef _WINDOWS
		std::string strDir = szFileDir;
		if (strDir.at(strDir.length() - 1) != '\\')
		{
			strDir += '\\';
		}

		WIN32_FIND_DATAA wfd;
		HANDLE hFind = ::FindFirstFileA((strDir + "*.*").c_str(), &wfd);
		if (hFind == INVALID_HANDLE_VALUE)
			return false;
		do
		{
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (stricmp(wfd.cFileName, ".") != 0 &&
					stricmp(wfd.cFileName, "..") != 0)
					DDRRemoveDir((strDir + wfd.cFileName).c_str());
			}
			else
			{
				::DeleteFileA((strDir + wfd.cFileName).c_str());
			}
		} while (::FindNextFileA(hFind, &wfd));
		::FindClose(hFind);
		if (!::RemoveDirectoryA(szFileDir))
		{
			return false;
		}
		return true;
#else
		// this is linux system
		return false;
#endif
	}

}

namespace DDRSys {

bool createDir(const char *pDirName)
{
#if defined(_WIN32) || defined(_WIN64)
	return (0 == _mkdir(pDirName));
#endif
#ifdef __linux__
	return (0 == mkdir(pDirName, 0));
#endif
}

bool isFileExisting(const char *fileName)
{
	struct stat info;
	if (stat(fileName, &info) != 0 || (info.st_mode & S_IFDIR)) {
		return false;
	}
	return true;
}

bool isDirExisting(const char *dirName)
{
	struct stat info;
	if (stat(dirName, &info) != 0 || !(info.st_mode & S_IFDIR)) {
		return false;
	}
	return true;
}

int deleteFile(const char *pFileName)
{
	return remove(pFileName);
}

int deleteDir(const char *pDirName)
{
	void *pMsgSt = Create_MsgSt();
	if (!pMsgSt) {
		return -1;
	}
	int slen = (int)strlen(pDirName);
	char ttt = (char)0x00; // not expanded yet
	Combine_MsgSt(pMsgSt, &ttt, 1, pDirName, slen);
	std::vector<char> vec;

	while (GetNumMsg_MsgSt(pMsgSt) > 0) {
		int nlen = GetNextMsgLen_MsgSt(pMsgSt);
		if (nlen <= 0) {
			break;
		}
		vec.resize(nlen);
		PopMsg_MsgSt(pMsgSt, &vec[0], nlen);
		if ((char)0x01 == vec[0]) { // already expanded
			vec.emplace_back('\0');
#ifdef DEBUGGIN_INFO_DISP
			std::cout << "Deleting folder (EXPANDED) '" << &vec[1] << "'\n";
#endif
#if defined(_WIN32) || defined(_WIN64)
			_rmdir(&vec[1]);
#endif
#ifdef __linux__
			rmdir(&vec[1]);
#endif
			continue;
		}
		// expanding
		vec.resize(nlen + 3);
		memcpy(&vec[nlen], "/*\0", 3);
#ifdef DEBUGGIN_INFO_DISP
		std::cout << "Expanding '" << &vec[1] << "'\n";
#endif
		bool bAnySubFolder = false;
#if defined(_WIN32) || defined(_WIN64)
		struct _finddata_t fileinfo;
		auto hFile = _findfirst(&vec[1], &fileinfo);
		vec.resize(nlen + 1);
		if (-1 != hFile) {
			do {
				if (0 == strcmp(fileinfo.name, ".") ||
					0 == strcmp(fileinfo.name, "..")) {
					continue;
				}
				if (0 == (fileinfo.attrib & _A_SUBDIR)) { // file
					int pos = (int)vec.size();
					int fnLen = (int)strlen(fileinfo.name);
					vec.resize(pos + fnLen + 1);
					memcpy(&vec[pos], fileinfo.name, fnLen + 1);
#ifdef DEBUGGIN_INFO_DISP
					std::cout << "Deleting file '" << &vec[1] << "'\n";
#endif
					remove(&vec[1]);
					vec.resize(pos);
				} else { // sub folder
					if (!bAnySubFolder) {
						// push back current folder as expanded
						vec[0] = (char)0x01;
						Add_MsgSt(pMsgSt, &vec[0], nlen);
						bAnySubFolder = true;
					}
					int pos = (int)vec.size();
					int fnLen = (int)strlen(fileinfo.name);
					vec.resize(pos + fnLen);
					memcpy(&vec[pos], fileinfo.name, fnLen);
					vec[0] = (char)0x00; // not expanded yet
#ifdef DEBUGGIN_INFO_DISP
					vec.emplace_back('\0');
					std::cout << "Adding sub-folder '" << &vec[1] << "'\n";
					vec.pop_back();
#endif
					Add_MsgSt(pMsgSt, &vec[0], (int)vec.size());
					vec.resize(pos);
				}
			} while (0 == _findnext(hFile, &fileinfo));
		}
		_findclose(hFile);

#elif defined(__linux__)
		glob_t globbuf;
		glob(&vec[1], 0, NULL, &globbuf);
		vec.resize(nlen + 1);
		if (globbuf.gl_pathc > 0) {
			for (int i = 0; i < (int)globbuf.gl_pathc; ++i) {
				int fuLen = strlen(globbuf.gl_pathv[i]);
				if (0 == strcmp(".", globbuf.gl_pathv[i]) ||
					0 == strcmp("..", globbuf.gl_pathv[i]) ||
					(fuLen > 2 &&
						0 == strncmp("/.", globbuf.gl_pathv[i] + fuLen - 2, 2)) ||
						(fuLen > 3 &&
							0 == strncmp("/..", globbuf.gl_pathv[i] + fuLen - 3, 3))) {
					continue;
				}
				struct stat sb;
				if (stat(globbuf.gl_pathv[i], &sb) < 0) {
					continue;
				}
				if ((sb.st_mode & S_IFMT) != S_IFDIR) { // regular file
#ifdef DEBUGGIN_INFO_DISP
					std::cout << "Deleting file '" << globbuf.gl_pathv[i] << "'\n";
#endif
					remove(globbuf.gl_pathv[i]);
				} else {
					if (!bAnySubFolder) {
						// push back current folder as expanded
						vec[0] = (char)0x01;
#ifdef DEBUGGIN_INFO_DISP
						vec.emplace_back('\0');
						std::cout << "Adding back cur-folder '" << &vec[1] << "'\n";
						vec.pop_back();
#endif
						Add_MsgSt(pMsgSt, &vec[0], nlen);
						bAnySubFolder = true;
					}
#ifdef DEBUGGIN_INFO_DISP
					std::cout << "Adding sub-folder '" << globbuf.gl_pathv[i] << "'\n";
#endif
					ttt = (char)0x00;
					int xxx = Combine_MsgSt(pMsgSt, &ttt, 1, globbuf.gl_pathv[i], strlen(globbuf.gl_pathv[i]));
				}
			}
		}
		globfree(&globbuf);
#endif

		if (!bAnySubFolder) { // current folder is empty
			vec.back() = '\0';
#ifdef DEBUGGIN_INFO_DISP
			std::cout << "Deleting folder (EMPTY) '" << &vec[1] << "'\n";
#endif
#if defined(_WIN32) || defined(_WIN64)
			_rmdir(&vec[1]);
#elif defined(__linux__)
			rmdir(&vec[1]);
#endif
			continue;
		}
	}

	Destroy_MsgSt(pMsgSt);
#if defined(_WIN32) || defined(_WIN64)
	return (-1 == _access(pDirName, 0) ? 0 : (-1));
#elif defined(__linux__)
	return (-1 == access(pDirName, F_OK) ? 0 : (-1));
#endif
}

__int64 getModTime(const char *pFileDirName)
{
	struct stat result;
	if (0 == stat(pFileDirName, &result) && -1 != result.st_mtime) {
		return (__int64)result.st_mtime;
	}
	return -1;
}

bool setModTime(const char *pFileDirName, __int64 secSincEpoch)
{
#if defined(_WIN32) || defined(_WIN64)
	HANDLE hFile = ::CreateFileA(pFileDirName,
		                         GENERIC_READ | GENERIC_WRITE,
		                         FILE_SHARE_READ | FILE_SHARE_DELETE,
		                         NULL, OPEN_EXISTING,
		                         FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (INVALID_HANDLE_VALUE == hFile) {
		return false;
	}
	FILETIME new_time;
	secSincEpoch = Int32x32To64(secSincEpoch, 10000000) + 116444736000000000;
	new_time.dwHighDateTime = (DWORD)(secSincEpoch >> 32);
	new_time.dwLowDateTime = (DWORD)secSincEpoch;
	bool bRet = ::SetFileTime(hFile, NULL, &new_time, &new_time);
	BOOL BBRR = ::CloseHandle(hFile);
	return bRet;
#elif defined(__linux__)
	struct stat foo;
	if (0 != stat(filename, &foo)) {
		return false;
	}
	struct utimbuf new_times;
	new_times.actime = foo.st_atime;
	new_times.modtime = secSincEpoch;
	return (0 == utime(filename, &new_times));
#endif
	return false;
}

__int64 getCreationTime(const char *pFileDirName)
{
	struct stat result;
	if (0 == stat(pFileDirName, &result)) {
		return (__int64)result.st_ctime;
	}
	return -1;
}

size_t getFileSize(const char *pFileName)
{
	struct stat result;
	if (0 == stat(pFileName, &result)) {
		return (size_t)result.st_size;
	}
	return 0;
}

bool setWorkingDir2CurrentExe()
{
	char fullName[256];
#if defined(_WIN32) || defined(_WIN64)
	int bytes = ::GetModuleFileNameA(nullptr, fullName, 255);
	if (0 == bytes) {
		return false;
	}
#endif
#ifdef __linux__
	char szTmp[32];
	sprintf(szTmp, "/proc/%d/exe", getpid());
	int bytes = readlink(szTmp, fullName, 255);
	if (bytes > 254) {
		bytes = 254;
	}
	if (bytes > 0) {
		fullName[bytes] = '\0';
	} else {
		return false;
	}
#endif
	for (int i = bytes; i >= 0; --i) {
		if ('/' == fullName[i] || '\\' == fullName[i]) {
			fullName[i] = '\0';
			break;
		}
	}
#if defined(_WIN32) || defined(_WIN64)
	return (::SetCurrentDirectoryA(fullName) != 0);
#endif
#ifdef __linux__
	return (0 == chdir(fullName));
#endif
}


struct _fileListStruct
{
	std::string dirName;
	std::string tmpFN;
	int sw;
	bool bSubFolder;
	std::vector<char> str;
	void *pMsgQ;
	bool bFirstLevelExp;
};

void* findAllFiles_Open(const char *pDir, int switchDirFile,
	                    bool bSubFolder)
{
	if (!pDir) {
		return nullptr;
	}
	_fileListStruct *pStruct = new _fileListStruct;
	if (!pStruct) {
		return nullptr;
	}
	pStruct->pMsgQ = Create_MsgQ();
	if (!pStruct->pMsgQ) {
		delete pStruct;
		return nullptr;
	}
	if (!pDir || !(*pDir)) {
		pStruct->dirName = "./";
	} else {
		pStruct->dirName = pDir;
	}
	if (pStruct->dirName.back() != '/' &&
		pStruct->dirName.back() != '\\') {
		pStruct->dirName += '/';
	}
	pStruct->sw = switchDirFile;
	pStruct->bSubFolder = bSubFolder;
	pStruct->bFirstLevelExp = false;

	if (Add_MsgQ(pStruct->pMsgQ, "*", 1) > 0) {
		return pStruct;
	} else {
		Destroy_MsgQ(pStruct->pMsgQ);
		delete pStruct;
		return nullptr;
	}
}

bool findAllFiles_Next(void *pHandle, const char **pRelativeName, bool *pbFolder)
{
	if (!pHandle) {
		return false;
	}
	_fileListStruct &hhh = *((_fileListStruct*)pHandle);

	while (1) {
		int nextLen = GetNextMsgLen_MsgQ(hhh.pMsgQ);
		if (nextLen <= 0) {
			return false;
		}
		hhh.str.resize(nextLen + 1);
		if (PopMsg_MsgQ(hhh.pMsgQ, &(hhh.str[0]), (int)hhh.str.size()) <= 0) {
			return false;
		}
		hhh.str.back() = '\0';
		bool bFullNameReady = true;
		for (auto &x : hhh.str) {
			if ('*' == x || '?' == x) {
				bFullNameReady = false;
				break;
			}
		}

		if (bFullNameReady) { // file name okay to be fetched
			hhh.tmpFN = &hhh.str[0];
			hhh.str.insert(hhh.str.begin(), hhh.dirName.c_str(),
				hhh.dirName.c_str() + hhh.dirName.length());
			struct stat result;
			if (0 != stat(&(hhh.str[0]), &result)) {
				continue;
			}
			hhh.str.erase(hhh.str.begin(), hhh.str.begin() + hhh.dirName.length());
			bool bDir = ((result.st_mode & S_IFDIR) != 0);
			if ((bDir && (hhh.sw & 0x02)) || (!bDir && (hhh.sw & 0x01))) {
				if (pRelativeName) {
					*pRelativeName = hhh.tmpFN.c_str();
				}
				if (pbFolder) {
					*pbFolder = bDir;
				}
				return true;
			} else {
				continue;
			}
		} else { // str contains '*'
			if (hhh.bFirstLevelExp && !hhh.bSubFolder) {
				continue;
			}
			hhh.str.insert(hhh.str.begin(), hhh.dirName.c_str(),
				hhh.dirName.c_str() + hhh.dirName.length());
			struct _finddata_t fileinfo;
			auto hFile = _findfirst(&(hhh.str[0]), &fileinfo);
			if (-1 == hFile) {
				continue;
			}
			hhh.str.erase(hhh.str.begin(), hhh.str.begin() + hhh.dirName.length());
			hhh.str.erase(hhh.str.end() - 2, hhh.str.end());
			int pos = (int)hhh.str.size();
			do {
				if (0 == strcmp(fileinfo.name, ".") ||
					0 == strcmp(fileinfo.name, "..")) {
					continue;
				}
				if (0 == (fileinfo.attrib & _A_SUBDIR)) { // single file
					hhh.str.insert(hhh.str.end(), fileinfo.name,
						fileinfo.name + strlen(fileinfo.name));
				} else { // sub-folder
					hhh.str.insert(hhh.str.end(), fileinfo.name,
						fileinfo.name + strlen(fileinfo.name));
					if (Add_MsgQ(hhh.pMsgQ, &(hhh.str[0]), (int)hhh.str.size()) <= 0) {
						return false;
					}
					hhh.str.emplace_back('/');
					hhh.str.emplace_back('*');
				}
				if (Add_MsgQ(hhh.pMsgQ, &(hhh.str[0]), (int)hhh.str.size()) <= 0) {
					return false;
				}
				hhh.str.resize(pos);
			} while (0 == _findnext(hFile, &fileinfo));
			hhh.bFirstLevelExp = true;
		}
	}
	return false;
}

void findAllFile_Close(void *pHandle)
{
	if (pHandle) {
		if (((_fileListStruct*)pHandle)->pMsgQ) {
			Destroy_MsgQ(((_fileListStruct*)pHandle)->pMsgQ);
		}
		delete (_fileListStruct*)pHandle;
	}
}

class _LocaleSetter {
public:
	_LocaleSetter() {
		setlocale(LC_ALL, "");
	}
} __g_ls__;

std::string wstring_to_utf8(const std::wstring& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> _conv;
	return _conv.to_bytes(str);
}

std::wstring utf8_to_wstring(const std::string &str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> _conv;
	return _conv.from_bytes(str);
}

std::wstring str_to_wstr(const char *pStr)
{
	const std::locale locale("");
	size_t nStrLen = strlen(pStr);
	const char* data_from_next = nullptr;
	std::vector<wchar_t> data_to(nStrLen + 1);
	wchar_t* data_to_next = nullptr;
	wmemset(&data_to[0], 0, nStrLen + 1);
	typedef std::codecvt<wchar_t, char, mbstate_t> convert_facet;
	mbstate_t in_state = { 0 };
	auto result = std::use_facet<convert_facet>(locale).in(
		in_state, pStr, pStr + nStrLen, data_from_next,
		&data_to[0], &data_to[0] + nStrLen + 1, data_to_next);
	std::wstring wstr;
	if (convert_facet::ok == result) {
		wstr = &data_to[0];
	}
	return wstr;
}

std::string wstr_to_str(const std::wstring &ws)
{
	const std::locale locale("");
	typedef std::codecvt<wchar_t, char, std::mbstate_t> converter_type;
	const converter_type& converter = std::use_facet<converter_type>(locale);
	std::vector<char> to(ws.length() * converter.max_length());
	std::mbstate_t state;
	const wchar_t* from_next;
	char* to_next;
	const converter_type::result result = converter.out(state, ws.data(), ws.data() + ws.length(), from_next, &to[0], &to[0] + to.size(), to_next);
	if (result == converter_type::ok || result == converter_type::noconv) {
		return std::string(&to[0], to_next);
	} else {
		return std::string();
	}
}

std::string sysStr_to_utf8(const char *pSysStr)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> _conv;
	return _conv.to_bytes(str_to_wstr(pSysStr));
}

std::string utf8_to_sysStr(const char *pU8Str)
{
	// UTF-8 to wstring
	std::wstring_convert<std::codecvt_utf8<wchar_t>> wconv;
	std::wstring wstr = wconv.from_bytes(pU8Str);
	// wstring to (sys-encoded) string
	return wstr_to_str(wstr);
}

}