
#include <iostream>
#include <ctime>


std::wstring GetTimeNowString()
{
	time_t rawtime;
	struct tm * timeinfo;
	wchar_t buffer[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	//wcsftime(buffer, sizeof(buffer), L"%d-%m-%Y %H:%M:%S", timeinfo);
	wcsftime(buffer, sizeof(buffer), L"%d-%m-%Y-%H-%M-%S", timeinfo);
	std::wstring str(buffer);
	return str;
}