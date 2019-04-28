#ifndef DDRMacro_h__
#define DDRMacro_h__


#include <src/Utility/Logger.h>

#include <thirdparty/protobuf/src/google/protobuf/descriptor.h>
#include <thirdparty/protobuf/src/google/protobuf/message.h>
#include <thirdparty/protobuf/src/google/protobuf/io/coded_stream.h>
#include <thirdparty/protobuf/src/google/protobuf/io/zero_copy_stream_impl.h>
#include <proto/BaseCmd.pb.h>
#include <proto/DDRModuleCmd.pb.h>
#ifdef _DEBUG
//#define DebugLog(...) printf(__VA_ARGS__);
#define DebugLog(...) DDRFramework::Print(__VA_ARGS__);
#else
#define DebugLog(...) DDRFramework::Print(__VA_ARGS__);
#endif

#define LevelLog(...) DDRFramework::DDRLog(__VA_ARGS__);

template<typename T> void SAFE_DELETE(T*& a) {
	delete a;
	a = nullptr;
}

#endif // DDRMacro_h__
