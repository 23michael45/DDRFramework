#ifndef __DDRFramework_COMMON_FUNCTIONS_H_INCLUDED__
#define __DDRFramework_COMMON_FUNCTIONS_H_INCLUDED__

#include <map>
#include <vector>
#include <asio.hpp>

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
std::map<std::string,std::string> GetSameSegmentIPV4(std::vector<std::string> left, std::vector<std::string> right);
}

#endif // __DDRFramework_COMMON_FUNCTIONS_H_INCLUDED__
