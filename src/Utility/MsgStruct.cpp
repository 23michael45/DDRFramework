#include "MsgStruct.h"

#include <malloc.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NULL_PTR 0
	
typedef struct {
	char *pBuffer;
	int capBytes;
	int headPos;
	int tailPos;
	int nMsg;
	int sz;
} __MsgQ_info__;

static int _trans2NewBuf(__MsgQ_info__ *pMsg, int nNewCap)
{
	if (!pMsg || nNewCap <= 0) {
		return 0;
	}
	if (nNewCap < pMsg->sz || nNewCap == pMsg->capBytes) {
		return pMsg->capBytes;
	}
	char *pNewBuf = (char*)malloc(nNewCap);
	if (!pNewBuf) {
		return pMsg->capBytes;
	}
	if (pMsg->pBuffer) {
		if (pMsg->sz > 0) {
			if (pMsg->headPos + pMsg->sz <= pMsg->capBytes) {
				// one contiguous segment
				memcpy(pNewBuf, pMsg->pBuffer + pMsg->headPos, pMsg->sz);
			} else {
				// two segments around the end
				int nSeg1 = pMsg->capBytes - pMsg->headPos;
				memcpy(pNewBuf, pMsg->pBuffer + pMsg->headPos, nSeg1);
				memcpy(pNewBuf + nSeg1, pMsg->pBuffer, pMsg->sz - nSeg1);
			}
		}
		free(pMsg->pBuffer);
	}
	pMsg->headPos = 0;
	pMsg->tailPos = pMsg->sz;
	pMsg->pBuffer = pNewBuf;
	pMsg->capBytes = nNewCap;
	return nNewCap;
}

static int _grow(__MsgQ_info__ *pMsg, int nMoreBytes) {
	int nNewCap = (pMsg->capBytes >= nMoreBytes ?
		          (pMsg->capBytes << 1) : (pMsg->capBytes + (nMoreBytes << 1)));
	return _trans2NewBuf(pMsg, nNewCap);
}

void* Create_MsgQ()
{
	__MsgQ_info__ *pObj = (__MsgQ_info__*)malloc(sizeof(__MsgQ_info__));
	if (pObj) {
		pObj->pBuffer = NULL_PTR;
		pObj->capBytes = 0;
		pObj->headPos = pObj->tailPos = 0;
		pObj->nMsg = 0;
		pObj->sz = 0;
	}
	return (void*)pObj;
}

int AdjustCap_MsgQ(void *pMsgQ, int newCap)
{
	if (!pMsgQ) {
		return -1;
	}
	__MsgQ_info__ *pObj = (__MsgQ_info__*)pMsgQ;
	if (newCap < pObj->sz) {
		return pObj->capBytes;
	}
	return _trans2NewBuf(pObj, newCap);
}

void Clear_MsgQ(void *pMsgQ)
{
	if (!pMsgQ) {
		return;
	}
	__MsgQ_info__ *pObj = (__MsgQ_info__*)pMsgQ;
	if (pObj->pBuffer) {
		free(pObj->pBuffer);
	}
	pObj->pBuffer = NULL_PTR;
	pObj->capBytes = 0;
	pObj->headPos = pObj->tailPos = 0;
	pObj->nMsg = 0;
}

void Destroy_MsgQ(void *pMsgQ)
{
	if (pMsgQ) {
		if (((__MsgQ_info__*)pMsgQ)->pBuffer) {
			free(((__MsgQ_info__*)pMsgQ)->pBuffer);
		}
		free(pMsgQ);
	}
}


int Add_MsgQ(void *pMsgQ, const void *pSrc, int nSrcLen)
{
	if (!pMsgQ || nSrcLen < 0 || (nSrcLen > 0 && !pSrc)) {
		return -1;
	}
	__MsgQ_info__ *pObj = (__MsgQ_info__*)pMsgQ;
	if (pObj->sz + 4 + nSrcLen >= pObj->capBytes) {
		if (_grow(pObj, 4 + nSrcLen) <= 0) {
			return 0;
		}
	}
	if (pObj->capBytes - pObj->tailPos >= 4) {
		*((int*)(pObj->pBuffer + pObj->tailPos)) = nSrcLen;
		pObj->tailPos = (pObj->tailPos + 4) % (pObj->capBytes);
	} else {
		pObj->pBuffer[pObj->tailPos++] = (char)(nSrcLen & 0xFF);
		pObj->tailPos %= pObj->capBytes;
		pObj->pBuffer[pObj->tailPos++] = (char)((nSrcLen >> 8) & 0xFF);
		pObj->tailPos %= pObj->capBytes;
		pObj->pBuffer[pObj->tailPos++] = (char)((nSrcLen >> 16) & 0xFF);
		pObj->tailPos %= pObj->capBytes;
		pObj->pBuffer[pObj->tailPos++] = (char)((nSrcLen >> 24) & 0xFF);
	}
	if (nSrcLen > 0) {
		int nContSpace = pObj->capBytes - pObj->tailPos;
		if (nContSpace >= nSrcLen) {
			memcpy(pObj->pBuffer + pObj->tailPos, pSrc, nSrcLen);
		} else {
			memcpy(pObj->pBuffer + pObj->tailPos, pSrc, nContSpace);
			memcpy(pObj->pBuffer, (const char*)pSrc + nContSpace, nSrcLen - nContSpace);
		}
		pObj->tailPos = (pObj->tailPos + nSrcLen) % (pObj->capBytes);
	}
	pObj->sz += 4 + nSrcLen;
	++(pObj->nMsg);
	return nSrcLen;
}


int Combine_MsgQ(void *pMsgQ,
	             const void *pSrc1, int nSrcLen1,
				 const void *pSrc2, int nSrcLen2)
{
	if (!pMsgQ || nSrcLen1 < 0 || nSrcLen2 < 0) {
		return -1;
	}
	if ((nSrcLen1 > 0 && !pSrc1) || (nSrcLen2 > 0 && !pSrc2)) {
		return -1;
	}
	if (0 == nSrcLen1) {
		return Add_MsgQ(pMsgQ, pSrc2, nSrcLen2);
	} else if (0 == nSrcLen2) {
		return Add_MsgQ(pMsgQ, pSrc1, nSrcLen1);
	}
	int nLen = nSrcLen1 + nSrcLen2;
	__MsgQ_info__ *pObj = (__MsgQ_info__*)pMsgQ;
	if (pObj->sz + 4 + nLen >= pObj->capBytes) {
		if (_grow(pObj, 4 + nLen) <= 0) {
			return -1;
		}
	}
	if (pObj->capBytes - pObj->tailPos >= 4) {
		*((int*)(pObj->pBuffer + pObj->tailPos)) = nLen;
		pObj->tailPos = (pObj->tailPos + 4) % pObj->capBytes;
	} else {
		pObj->pBuffer[pObj->tailPos++] = (char)(nLen & 0xFF);
		pObj->tailPos %= pObj->capBytes;
		pObj->pBuffer[pObj->tailPos++] = (char)((nLen >> 8) & 0xFF);
		pObj->tailPos %= pObj->capBytes;
		pObj->pBuffer[pObj->tailPos++] = (char)((nLen >> 16) & 0xFF);
		pObj->tailPos %= pObj->capBytes;
		pObj->pBuffer[pObj->tailPos++] = (char)((nLen >> 24) & 0xFF);
	}
	int nContSpace = pObj->capBytes - pObj->tailPos;
	if (nContSpace >= nSrcLen1) {
		memcpy(pObj->pBuffer + pObj->tailPos, pSrc1, nSrcLen1);
	} else {
		memcpy(pObj->pBuffer + pObj->tailPos, pSrc1, nContSpace);
		memcpy(pObj->pBuffer, (const char*)pSrc1 + nContSpace, nSrcLen1 - nContSpace);
	}
	pObj->tailPos = (pObj->tailPos + nSrcLen1) % pObj->capBytes;
	nContSpace = pObj->capBytes - pObj->tailPos;
	if (nContSpace >= nSrcLen2) {
		memcpy(pObj->pBuffer + pObj->tailPos, pSrc2, nSrcLen2);
	} else {
		memcpy(pObj->pBuffer + pObj->tailPos, pSrc2, nContSpace);
		memcpy(pObj->pBuffer, (const char*)pSrc2 + nContSpace, nSrcLen2 - nContSpace);
	}
	pObj->tailPos = (pObj->tailPos + nSrcLen2) % pObj->capBytes;
	pObj->sz += 4 + nLen;
	++(pObj->nMsg);
	return nLen;
}

int GetNumMsg_MsgQ(void *pMsgQ)
{
	if (pMsgQ) {
		return ((__MsgQ_info__*)pMsgQ)->nMsg;
	} else {
		return -1;
	}
}

int GetCurSz_MsgQ(void *pMsgQ)
{
	if (pMsgQ) {
		return ((__MsgQ_info__*)pMsgQ)->sz;
	} else {
		return -1;
	}
}

int GetCurCap_MsgQ(void *pMsgQ)
{
	if (pMsgQ) {
		return ((__MsgQ_info__*)pMsgQ)->capBytes;
	} else {
		return -1;
	}
}

int GetNextMsgLen_MsgQ(void *pMsgQ)
{
	if (pMsgQ) {
		__MsgQ_info__ *pObj = (__MsgQ_info__*)pMsgQ;
		if (0 == pObj->nMsg) {
			return -1;
		}
		if (pObj->capBytes - pObj->headPos >= 4) {
			return *((int*)(pObj->pBuffer + pObj->headPos));
		} else {
			int x = 0;
			int pp = pObj->headPos;
			x |= pObj->pBuffer[pp++];
			pp %= pObj->capBytes;
			x |= (pObj->pBuffer[pp++] << 8);
			pp %= pObj->capBytes;
			x |= (pObj->pBuffer[pp++] << 16);
			pp %= pObj->capBytes;
			x |= (pObj->pBuffer[pp] << 24);
			return x;
		}
	} else {
		return -1;
	}
}

int PopMsg_MsgQ(void *pMsgQ, void *pBuf, int nBufCap)
{
	if (!pMsgQ) {
		return -1;
	}
	__MsgQ_info__ *pObj = (__MsgQ_info__*)pMsgQ;
	int nLen = GetNextMsgLen_MsgQ(pMsgQ);
	if (nBufCap < nLen || nLen < 0) {
		return -1;
	}
	pObj->headPos = (pObj->headPos + 4) % pObj->capBytes;
	pObj->sz -= 4;
	if (nLen > 0) {
		int nContSpace = pObj->capBytes - pObj->headPos;
		if (nContSpace >= nLen) {
			memcpy(pBuf, pObj->pBuffer + pObj->headPos, nLen);
		} else {
			memcpy(pBuf, pObj->pBuffer + pObj->headPos, nContSpace);
			memcpy((char*)pBuf + nContSpace, pObj->pBuffer, nLen - nContSpace);
		}
		pObj->headPos = (pObj->headPos + nLen) % pObj->capBytes;
		pObj->sz -= nLen;
	}
	if (0 == --(pObj->nMsg)) {
		pObj->headPos = pObj->tailPos = 0;
	}
	return nLen;
}


typedef struct {
	char *pBuffer;
	int cap;
	int sz;
	int nMsg;
} __MsgSt_info__;

static int _trans2NewBuf_MsgSt(__MsgSt_info__ *pMsg, int nNewCap)
{
	if (!pMsg || nNewCap <= 0) {
		return -1;
	}
	if (nNewCap < pMsg->sz || nNewCap == pMsg->cap) {
		return pMsg->cap;
	}
	char *pNewBuf = (char*)malloc(nNewCap);
	if (!pNewBuf) {
		return pMsg->cap;
	}
	if (pMsg->pBuffer) {
		if (pMsg->sz > 0) {
			memcpy(pNewBuf, pMsg->pBuffer, pMsg->sz);
		}
		free(pMsg->pBuffer);
	}
	pMsg->pBuffer = pNewBuf;
	pMsg->cap = nNewCap;
	return nNewCap;
}

static int _grow_MsgSt(__MsgSt_info__ *pMsg, int nMoreBytes) {
	int nNewCap = (pMsg->cap >= nMoreBytes ?
		          (pMsg->cap << 1) : (pMsg->cap + (nMoreBytes << 1)));
	return _trans2NewBuf_MsgSt(pMsg, nNewCap);
}

void* Create_MsgSt()
{
	__MsgSt_info__ *pMsgSt = (__MsgSt_info__*)malloc(sizeof(__MsgSt_info__));
	if (pMsgSt) {
		pMsgSt->pBuffer = NULL_PTR;
		pMsgSt->cap = 0;
		pMsgSt->sz = 0;
		pMsgSt->nMsg = 0;
	}
	return (void*)pMsgSt;
}

int AdjustCap_MsgSt(void *pMsgSt, int newCap)
{
	if (!pMsgSt) {
		return -1;
	}
	__MsgSt_info__ *pObj = (__MsgSt_info__*)pMsgSt;
	if (newCap < pObj->sz) {
		return pObj->cap;
	}
	return _trans2NewBuf_MsgSt(pObj, newCap);
}

void Clear_MsgSt(void *pMsgSt)
{
	if (!pMsgSt) {
		return;
	}
	__MsgSt_info__ *pObj = (__MsgSt_info__*)pMsgSt;
	if (pObj->pBuffer) {
		free(pObj->pBuffer);
	}
	pObj->pBuffer = NULL_PTR;
	pObj->cap = 0;
	pObj->sz = 0;
	pObj->nMsg = 0;
}

void Destroy_MsgSt(void *pMsgSt)
{
	if (pMsgSt) {
		if (((__MsgSt_info__*)pMsgSt)->pBuffer) {
			free(((__MsgSt_info__*)pMsgSt)->pBuffer);
		}
		free(pMsgSt);
	}
}

int Add_MsgSt(void *pMsgSt, const void *pSrc, int nSrcLen)
{
	if (!pMsgSt || nSrcLen < 0 || (nSrcLen > 0 && !pSrc)) {
		return -1;
	}
	__MsgSt_info__ *pObj = (__MsgSt_info__*)pMsgSt;
	if (pObj->sz + nSrcLen + 4 >= pObj->cap) {
		if (_grow_MsgSt(pObj, nSrcLen + 4) <= 0) {
			return -1;
		}
	}
	if (nSrcLen > 0) {
		memcpy(pObj->pBuffer + pObj->sz, pSrc, nSrcLen);
		pObj->sz += nSrcLen;
	}
	*((int*)(pObj->pBuffer + pObj->sz)) = nSrcLen;
	pObj->sz += 4;
	++(pObj->nMsg);
	return nSrcLen;
}

int Combine_MsgSt(void *pMsgSt,
	              const void *pSrc1, int nSrcLen1,
				  const void *pSrc2, int nSrcLen2)
{

	if (!pMsgSt || nSrcLen1 < 0 || nSrcLen2 < 0) {
		return -1;
	}
	if ((nSrcLen1 > 0 && !pSrc1) || (nSrcLen2 > 0 && !pSrc2)) {
		return -1;
	}
	if (0 == nSrcLen1) {
		return Add_MsgSt(pMsgSt, pSrc2, nSrcLen2);
	}
	else if (0 == nSrcLen2) {
		return Add_MsgSt(pMsgSt, pSrc1, nSrcLen1);
	}
	int nLen = nSrcLen1 + nSrcLen2;
	__MsgSt_info__ *pObj = (__MsgSt_info__*)pMsgSt;
	if (pObj->sz + nLen + 4 >= pObj->cap) {
		if (_grow_MsgSt(pObj, nLen + 4) <= 0) {
			return -1;
		}
	}
	if (nSrcLen1 > 0) {
		memcpy(pObj->pBuffer + pObj->sz, pSrc1, nSrcLen1);
		pObj->sz += nSrcLen1;
	}
	if (nSrcLen2 > 0) {
		memcpy(pObj->pBuffer + pObj->sz, pSrc2, nSrcLen2);
		pObj->sz += nSrcLen2;
	}
	*((int*)(pObj->pBuffer + pObj->sz)) = nLen;
	pObj->sz += 4;
	++(pObj->nMsg);
	return nLen;
}

int GetNumMsg_MsgSt(void *pMsgSt)
{
	if (pMsgSt) {
		return ((__MsgSt_info__*)pMsgSt)->nMsg;
	} else {
		return -1;
	}
}

int GetCurSz_MsgSt(void *pMsgSt)
{
	if (pMsgSt) {
		return ((__MsgSt_info__*)pMsgSt)->sz;
	} else {
		return -1;
	}
}

int GetCurCap_MsgSt(void *pMsgSt)
{
	if (pMsgSt) {
		return ((__MsgSt_info__*)pMsgSt)->cap;
	} else {
		return -1;
	}
}

int GetNextMsgLen_MsgSt(void *pMsgSt)
{
	if (pMsgSt) {
		__MsgSt_info__ *pObj = (__MsgSt_info__*)pMsgSt;
		if (pObj->sz < 4) {
			return -1;
		}
		return *((int*)(pObj->pBuffer + pObj->sz - 4));
	} else {
		return -1;
	}
}

int PopMsg_MsgSt(void *pMsgSt, void *pBuf, int nBufCap)
{
	if (pMsgSt) {
		__MsgSt_info__ *pObj = (__MsgSt_info__*)pMsgSt;
		if (pObj->sz < 4) {
			return -1;
		}
		int nlen = *((int*)(pObj->pBuffer + pObj->sz - 4));
		if (pObj->sz < nlen + 4) {
			return -1;
		}
		if (nBufCap < nlen) {
			return -1;
		}
		memcpy(pBuf, pObj->pBuffer + pObj->sz - (nlen + 4), nlen);
		pObj->sz -= (nlen + 4);
		--(pObj->nMsg);
		return nlen;
	} else {
		return -1;
	}
}

#ifdef __cplusplus
}
#endif



