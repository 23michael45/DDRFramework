/*
DaDaoRobot group, Communication Library
Basic data structure - message queue and message stack
*/
#ifndef __DDRCOMMLIB_CAP_ADAPTIVE_VARIABLE_SIZED_MESSAGE_QUEUE_H_INCLUDED__
#define __DDRCOMMLIB_CAP_ADAPTIVE_VARIABLE_SIZED_MESSAGE_QUEUE_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

/* Create a variable-sized message queue, and return its address. */
void* Create_MsgQ();
/* Adjust memory capacity of the queue to fit (as long as newCap >= current
size). The new capacity will be returned. */
int AdjustCap_MsgQ(void *pMsgQ, int newCap);
/* Clear contents and release memory. */
void Clear_MsgQ(void *pMsgQ);
/* Destroy a message queue from heap. */
void Destroy_MsgQ(void *pMsgQ);
/* Add a message starting from pSrc with length nSrcLen, to a message queue.
Return actual length added (<0 for error). */
int Add_MsgQ(void *pMsgQ, const void *pSrc, int nSrcLen);
/* Combine two strings ([pSrc1, pSrc1 + nSrcLen1) and
[pSrc2, pSrc2 + nStrLen2)) to the message queue stored at pMsgQ. Return total
length added (<0 for error). */
int Combine_MsgQ(void *pMsgQ,
                 const void *pSrc1, int nSrcLen1,
			     const void *pSrc2, int nSrcLen2);

/* Return the number of messages in the queue. */
int GetNumMsg_MsgQ(void *pMsgQ);

/* Return the number of bytes currently occupied. */
int GetCurSz_MsgQ(void *pMsgQ);

/* Return current capcity (in bytes) of the Q. */
int GetCurCap_MsgQ(void *pMsgQ);

/* Return the length of next message (0 for empty queue). */
int GetNextMsgLen_MsgQ(void *pMsgQ);

/* Pop next message from the queue to the buffer speicified by
[pBuf, pBuf + nBufCap). Return length of the poped message (<0 for empty queue
or insufficient buffer capacity */
int PopMsg_MsgQ(void *pMsgQ, void *pBuf, int nBufCap);


/* Create a variable-sized message stack, and return its address. */
void* Create_MsgSt();
/* Adjust memory capacity of the stack to fit (as long as newCap >= current
size). The new capacity will be returned. */
int AdjustCap_MsgSt(void *pMsgSt, int newCap);
/* Clear contents and release memory. */
void Clear_MsgSt(void *pMsgSt);
/* Destroy a message queue from heap. */
void Destroy_MsgSt(void *pMsgSt);
/* Add a message starting from pSrc with length nSrcLen, to a message stack.
Return actual length added (<0 for error). */
int Add_MsgSt(void *pMsgSt, const void *pSrc, int nSrcLen);
/* Combine two strings ([pSrc1, pSrc1 + nSrcLen1) and
[pSrc2, pSrc2 + nStrLen2)) to the message queue stored at pMsgQ. Return total
length added (<0 for error). */
int Combine_MsgSt(void *pMsgSt,
	              const void *pSrc1, int nSrcLen1,
				  const void *pSrc2, int nSrcLen2);
/* Return the number of messages in the stack. */
int GetNumMsg_MsgSt(void *pMsgSt);
/* Return the number of bytes currently occupied. */
int GetCurSz_MsgSt(void *pMsgSt);
/* Return current capcity (in bytes) of the stack. */
int GetCurCap_MsgSt(void *pMsgSt);
/* Return the length of next message (0 for empty stack). */
int GetNextMsgLen_MsgSt(void *pMsgSt);
/* Pop next message from the stack to the buffer speicified by
[pBuf, pBuf + nBufCap). Return length of the poped message (<0 for empty stack
or insufficient buffer capacity */
int PopMsg_MsgSt(void *pMsgSt, void *pBuf, int nBufCap);

#ifdef __cplusplus
}
#endif

#endif

