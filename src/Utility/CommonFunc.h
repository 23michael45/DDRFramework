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







	// 多字节编码转为UTF8编码  
	bool MBToUTF8(std::vector<char>& pu8, const char* pmb, int mLen);

	// UTF8编码转为多字节编码  
	bool UTF8ToMB(std::vector<char>& pmb, const char* pu8, int utf8Len);

	// 多字节编码转为Unicode编码  
	bool MBToUnicode(std::vector<wchar_t>& pun, const char* pmb, int mLen);
	std::string MBToUTF8String(std::string mbstr);

	//Unicode编码转为多字节编码  
	bool UnicodeToMB(std::vector<char>& pmb, const wchar_t* pun, int uLen);
	std::string UTF8ToMBString(std::string utf8str);

	// UTF8编码转为Unicode  
	bool UTF8ToUnicode(std::vector<wchar_t>& pun, const char* pu8, int utf8Len);

	// Unicode编码转为UTF8  
	bool UnicodeToUTF8(std::vector<char>& pu8, const wchar_t* pun, int uLen);

	std::wstring s2w(const std::string& utf8string);
	std::string w2s(const std::wstring& utf16Str);
	std::wstring StringToWString(const std::string& str);
	std::string WStringToString(const std::wstring& str);
	std::string getexepath();
	std::string getStarWildRegex(const std::string& s, bool bbegin,std::string root = "");
	std::string replace_all(std::string& str, std::string sub, std::string replaceto);
	std::vector<std::string> split(const std::string &text, char sep);


	void DisableMouseSelectConsole();

	bool DDRDeleteFile(const char* szFile);
	bool DDRRemoveDir(const char* szFileDir);
}

namespace DDRSys {

// file system functions
bool createDir(const char *pDirName);
bool isFileExisting(const char *fileName);
bool isDirExisting(const char *dirName);

/* return 0 for success, -1 for error. */
int deleteFile(const char *pFileName);
/* Try to clear a folder specified by pDirName, empty or not. Return 0 for
success, -1 for error. */
int deleteDir(const char *pDirName);

// get last modification time of a file (seconds since EPOCH)
long long getModTime(const char *pFileDirName);
// set last modification time of a file (seconds since EPOCH)
bool setModTime(const char *pFileDirName, long long secSincEpoch);
// get creation time of a file (seconds since EPOCH)
long long getCreationTime(const char *pFileDirName);
// get file size (in bytes) of a file
size_t getFileSize(const char *pFileName);

// set current working directory to where this EXE file is
bool setWorkingDir2CurrentExe();

// switchDirFile: bit 0 - non-folder file; bit 1 - folder
// bSubFolder: dig up sub directories or not
void* findAllFiles_Open(const char *pWorkingDir, int switchDirFile,
		                bool bSubFolder);
bool findAllFiles_Next(void *pHandle, const char **pRelativeName, bool *pbFolder);
void findAllFile_Close(void *pHandle);

std::string sysStr_to_utf8(const char *pSysStr);
std::string utf8_to_sysStr(const char *pU8Str);

}

#endif // CommonFunc_h__
