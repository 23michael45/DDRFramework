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

short getVolumeHash();
std::string getCpuHash();
std::string getMacAddr();
const wchar_t* getMachineName();
    
#endif // DeviceUDID_h__