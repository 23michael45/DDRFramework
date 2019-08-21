#ifndef __DDRFRAMEWORK_LOGGER_DEFINITIONS_H_INCLUDED__
#define __DDRFRAMEWORK_LOGGER_DEFINITIONS_H_INCLUDED__

#include "src/Utility/Logger.h"
#ifdef _DEBUG
//#define DebugLog(...) printf(__VA_ARGS__);
#define DebugLog(...) DDRFramework::Print(__VA_ARGS__);
#else
#define DebugLog(...) DDRFramework::Print(__VA_ARGS__);
#endif
#define LevelLog(...) DDRFramework::DDRLog(__VA_ARGS__);

#endif // __DDRFRAMEWORK_LOGGER_DEFINITIONS_H_INCLUDED__
