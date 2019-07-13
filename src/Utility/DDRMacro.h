/*!
 * File: DDRMacro.h
 * Date: 2019/04/29 16:00
 *
 * Author: michael
 * Contact: michael2345@live.cn
 *
 * Description:Log and proto buf headers
 *
*/
#ifndef DDRMacro_h__
#define DDRMacro_h__

#include <thirdparty/protobuf/src/google/protobuf/descriptor.h>
#include <thirdparty/protobuf/src/google/protobuf/message.h>
#include <thirdparty/protobuf/src/google/protobuf/io/coded_stream.h>
#include <thirdparty/protobuf/src/google/protobuf/io/zero_copy_stream_impl.h>
#include <proto/BaseCmd.pb.h>
#include <proto/ThirdpartyCmd.pb.h>
#include <proto/DDRVLNMap.pb.h>
#ifdef USE_MODULE_PROTO
#include <proto/DDRModuleCmd.pb.h>
#endif // USE_MODULE_PROTO

#include "LoggerDef.h"

template<typename T> void SAFE_DELETE(T*& a) {
	delete a;
	a = nullptr;
}

#endif // DDRMacro_h__
