#ifndef CommonFunc_h__
#define CommonFunc_h__


#include <new>
#include <map>
#include <vector>
#include <string>


#ifndef BYTE
typedef unsigned char BYTE;
#endif
#ifndef U32
typedef unsigned int U32;
#endif
#ifndef U16
typedef unsigned short U16;
#endif
#ifndef I64
typedef long long I64;
#endif

namespace DDRFramework {

	bool Txt_Encrypt(const void *pSrc, int lenSrc,
		const void *pTar, int lenTar);
	bool Txt_Encrypt2(const void *pSrc1, int len1,
		const void *pSrc2, int len2,
		void *pTar, int lenTar);
	bool Txt_Decrypt(const void *pSrc, int lenSrc,
		void *pTar, int lenTar);
	I64 EncryptDataLen_64(unsigned int len);
	bool VerifyDataLen_64(const void *pDataHead, unsigned int *plen);
	inline int SizeOfELen_64() { return 8; }
	int EncryptDataLen_32(int len);
	bool VerifyDataLen_32(const void *pDataHead, int *plen);
	inline int SizeOfELen_32() { return 4; }
	void ConvertUIP2Str_IPv4(U32 ip,
		char *pStr, int *pEndingPos = nullptr);
	bool ConvertStr2UIP_IPv4(const char *pStr, U32 *ip,
		char cDelimiter = '\0', int *pEndingPos = nullptr);
	bool ConvertStr2Int(const char *pStr, int *x,
		char cDelimiter = '\0', int *pEndingPos = nullptr);
	void ConvertInt2Str(int x, char *pStr, int *pEndingPos = nullptr);
	U16 ConvertHTONS(U16 val);

	int KeyTest();
	void Sleep(int nMillisec);
	bool GetUTCTimeStr(char *pBuf, int nBufCap, int *pLen);
	bool GetLocalTimeStr(char *pBuf, int nBufCap, int *pLen);

#ifdef __linux__
	// GB2312 to UTF-8
	int StrConv_g2u(char *inbuf, int inlen, char *outbuf, int outlen);
	// UTF-8 to GB2312
	int StrConv_u2g(char *inbuf, int inlen, char *outbuf, int outlen);
#endif



	std::vector<std::string> GetLocalIPV4();
	std::map<std::string, std::string> GetSameSegmentIPV4(std::vector<std::string> left, std::vector<std::string> right);






	// 多字节编码转为UTF8编码  
	bool MBToUTF8(std::vector<char>& pu8, const char* pmb, int mLen);

	// UTF8编码转为多字节编码  
	bool UTF8ToMB(std::vector<char>& pmb, const char* pu8, int utf8Len);

	// 多字节编码转为Unicode编码  
	bool MBToUnicode(std::vector<wchar_t>& pun, const char* pmb, int mLen);

	//Unicode编码转为多字节编码  
	bool UnicodeToMB(std::vector<char>& pmb, const wchar_t* pun, int uLen);

	// UTF8编码转为Unicode  
	bool UTF8ToUnicode(std::vector<wchar_t>& pun, const char* pu8, int utf8Len);

	// Unicode编码转为UTF8  
	bool UnicodeToUTF8(std::vector<char>& pu8, const wchar_t* pun, int uLen);
	bool UnicodeToUTF8(std::vector<char>& pu8, const wchar_t* pun, int uLen);

	std::wstring s2w(const std::string& utf8string);
	std::string w2s(const std::wstring& utf16Str);
	std::wstring StringToWString(const std::string& str);
	std::string WStringToString(const std::wstring& str);
	std::string getexepath();
	std::string getStarWildRegex(const std::string& s, bool bbegin);
	std::string replace_all(std::string& str, std::string sub, std::string replaceto);
	std::vector<std::string> split(const std::string &text, char sep);


	void DisableMouseSelectConsole();
}

#endif // CommonFunc_h__
