#ifndef __DDRMTLIB_BACKGROUND_THREAD_H_INCLUDED__
#define __DDRMTLIB_BACKGROUND_THREAD_H_INCLUDED__

namespace DDRMTLib {

bool CreateBkgThread(void(*pFunc)(void*, bool*), void *pArg, unsigned int *pThreadID = nullptr);
bool Wait4BkgThread(unsigned int bkgThID, int waitTimeMilisec);

}

#endif // __DDRMTLIB_BACKGROUND_THREAD_H_INCLUDED__
