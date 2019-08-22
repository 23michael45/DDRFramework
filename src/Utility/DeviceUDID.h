/*!
 * File: DeviceUDID.h
 * Date: 2019/04/29 16:02
 *
 * Author: michael
 * Contact: michael2345@live.cn
 *
 * Description: functions to get device info
 *
*/
#ifndef DeviceUDID_h__
#define DeviceUDID_h__
#include <string>


std::string getMacAddr();

#ifdef _WINDOWS
const wchar_t* getMachineName();
short getVolumeHash();
std::string getCpuHash();
#else
const char* getMachineName();
unsigned short getVolumeHash();
unsigned short getCpuHash();
#endif
#endif // DeviceUDID_h__