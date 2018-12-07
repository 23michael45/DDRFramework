#ifndef DDRMacro_h__
#define DDRMacro_h__

#ifdef _DEBUG
#define DebugLog(...) printf(__VA_ARGS__);
#else
#define DebugLog(...) 
#endif



#endif // DDRMacro_h__
