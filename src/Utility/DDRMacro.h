#ifndef DDRMacro_h__
#define DDRMacro_h__




#include "../../../Shared/proto/BaseCmd.pb.h"

#ifdef _DEBUG
//#define DebugLog(...) printf(__VA_ARGS__);
#define DebugLog(...) DDRFramework::Print(__VA_ARGS__);
#else
#define DebugLog(...) DDRFramework::Print(__VA_ARGS__);
#endif

template<typename T> void SAFE_DELETE(T*& a) {
	delete a;
	a = nullptr;
}

#endif // DDRMacro_h__
