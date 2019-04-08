#ifndef DeviceUDID_h__
#define DeviceUDID_h__
#include <string>

short getVolumeHash();
std::string getCpuHash();
std::string getMacAddr();
const wchar_t* getMachineName();
    
#endif // DeviceUDID_h__