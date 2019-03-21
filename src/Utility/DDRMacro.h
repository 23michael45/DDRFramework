#ifndef DDRMacro_h__
#define DDRMacro_h__

#ifdef _DEBUG
//#define DebugLog(...) printf(__VA_ARGS__);
#define DebugLog(...) DDRFramework::Print(__VA_ARGS__);
#else
#define DebugLog(...) //DDRFramework::Print(__VA_ARGS__);
#endif


#define RegisterProcessor(action,name) action##name action##name;\
m_ProcessorMap[action##name.GetTypeName()] = std::make_shared<name##Processor>(*this);

template<typename T> void SAFE_DELETE(T*& a) {
	delete a;
	a = NULL;
}

#endif // DDRMacro_h__
