#include "Logger.h"
#include <stdio.h>
#include <stdarg.h>
#include "CommonFunc.h"
#include "cppfs/cppfs.h"
#include "cppfs/FilePath.h"

#ifdef WIN32
#include <winsock.h>
#include <Windows.h>

int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
	// Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
	// This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
	// until 00:00:00 January 1, 1970 
	static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

	SYSTEMTIME  system_time;
	FILETIME    file_time;
	uint64_t    time;

	GetSystemTime(&system_time);
	SystemTimeToFileTime(&system_time, &file_time);
	time = ((uint64_t)file_time.dwLowDateTime);
	time += ((uint64_t)file_time.dwHighDateTime) << 32;

	tp->tv_sec = (long)((time - EPOCH) / 10000000L);
	tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
	return 0;
}
#else
#include <sys/time.h>

#endif // WIN32



using namespace std;
namespace DDRFramework
{
	std::string GetTimeNowstring()
	{
		time_t rawtime;
		struct tm * timeinfo;
		char buffer[80];

		time(&rawtime);
		timeinfo = localtime(&rawtime);

		//wcsftime(buffer, sizeof(buffer), L"%d-%m-%Y %H:%M:%S", timeinfo);
		std::strftime(buffer, sizeof(buffer), "%d-%m-%Y-%H-%M-%S", timeinfo);
		std::string str(buffer);
		return str;
	}

	// We have to define the static instance here so that member functions can use it.
	std::shared_ptr<Log> Log::log;

	std::shared_ptr<Log> Log::getInstance() {
		if (!log) {
			log = std::shared_ptr<Log>(new Log());

			// Set logger to write to a file
			//log->setTarget(Log::Target::LOG_FILE);
			log->setTarget(Log::Target::STDOUT);

			

		}
		return log;
	}

	void Log::setTarget(Log::Target target) {


		this->logTarget = target;
	}

	void Log::setLevel(Log::Level level) {
		this->logLevel = level;
	}

	int Log::setFile(std::string fileName) {
		// Make sure we can open the file for writing
		ofstream logFile(fileName, ofstream::app);
		if (!logFile.is_open()) {
			// Log the failure and return an error code
			this->log->write(Log::Level::ERR, "Failed to open log file '" + this->logFile + "'");
			return 1;
		}

		this->logFile = fileName;
		return 0;
	}

	Log::Level Log::getLevel() {
		return this->logLevel;
	}

	std::string Log::levelTostring(Log::Level level) {
		return this->levelMap[level];
	}


	void Log::write(Log::Level level, std::string message) {
		std::string toLog;

		// Only log if we're at or above the pre-defined severity
		if (level < this->logLevel) {
			return;
		}

		// Log::Target::DISABLED takes precedence over other targets
		if ((this->logTarget & Log::Target::DISABLED) == Log::Target::DISABLED) {
			return;
		}

		// Append the log level if enabled
		if (this->levelEnabled) {
			toLog += this->levelTostring(level);
		}

		// Append the current date and time if enabled
		if (this->timestampEnabled) {
			std::time_t time = chrono::system_clock::to_time_t(chrono::system_clock::now());
			struct tm * timeStruct = std::localtime(&time);


			struct timeval tv;
			gettimeofday(&tv, NULL);
			int millisec = lrint(tv.tv_usec / 1000.0); // Round to nearest millisec

			char timeStr[80];
			strftime(timeStr, 80, "%d/%b/%Y:%H:%M:%S", timeStruct);


			char fullstr[80];
			sprintf(fullstr, " [ %s:%03d ]", timeStr, millisec);
			toLog +=  std::string(fullstr);
		}

		// Append the message to our log statement
		toLog += " " + message;

		// Log to stdout if it's one of our targets
		if ((this->logTarget & Log::Target::STDOUT) == Log::Target::STDOUT) {
			std::cout << toLog << std::endl;
		}

		// Log to stderr if it's one of our targets
		if ((this->logTarget & Log::Target::STDERR) == Log::Target::STDERR) {
			std::cerr << toLog << std::endl;
		}

		// Log to a file if it's one of our targets and we've set a logFile
		if ((this->logTarget & Log::Target::LOG_FILE) == Log::Target::LOG_FILE && this->logFile != "") {
			ofstream logFile(this->logFile, ofstream::app);
			logFile << toLog << "\n";
			logFile.close();
		}
	}

	void Print(const char* format, ...)
	{
		va_list args;
		char buff[1024];
		va_start(args, format);
		_vsnprintf(buff, 1024, format, args);
		va_end(args);
		DDRFramework::Log::getInstance()->write(DDRFramework::Log::Level::INFO, buff);
	}

	ConsoleDebug::ConsoleDebug()
	{
		m_Quit = false;
		m_ToggleLogConsole = true;
		m_ToggleLogFile = false;
		AddCommand("l", std::bind(&ConsoleDebug::ToggleLogConsole, this));
		AddCommand("lf", std::bind(&ConsoleDebug::ToggleLogFile, this));
		AddCommand("q", std::bind(&ConsoleDebug::Quit, this));
	}

	void ConsoleDebug::ToggleLogConsole()
	{
		m_ToggleLogConsole = !m_ToggleLogConsole;

		auto target = DDRFramework::Log::getInstance()->getTarget();
		if (m_ToggleLogConsole == true)
		{

			printf_s("\nToggleLog Turn On Console");

			target = target | DDRFramework::Log::Target::STDOUT;

		}
		else
		{

			printf_s("\nToggleLog Turn Off Console");
			target = (DDRFramework::Log::Target)(target & ~DDRFramework::Log::Target::STDOUT);
		}

		DDRFramework::Log::getInstance()->setTarget(target);
	}	
	void ConsoleDebug::ToggleLogFile()
	{
		m_ToggleLogFile = !m_ToggleLogFile;

		auto target = DDRFramework::Log::getInstance()->getTarget();
		if (m_ToggleLogFile == true)
		{
			cppfs::FilePath path(DDRFramework::getexepath());
			std::string exename = path.baseName();
			std::string filename = exename + "-" + GetTimeNowstring() + ".log";
			Log::getInstance()->setFile(filename + ".log");

			printf_s("\nToggleLog Turn On File");
			target = target | DDRFramework::Log::Target::LOG_FILE;

		}
		else
		{

			printf_s("\nToggleLog Turn Off File");
			target = (DDRFramework::Log::Target)(target & ~DDRFramework::Log::Target::LOG_FILE);
		}
		DDRFramework::Log::getInstance()->setTarget(target);
	}
	void ConsoleDebug::Quit()
	{
		m_Quit = true;
	}


	void ConsoleDebug::ConsoleDebugLoop()
	{
		std::string input;
		do
		{
			getline(cin, input);

			m_CurrentCmd = input;

			auto vec = split(m_CurrentCmd,':');
			auto cmd = vec[0];

			if (m_Functionmap.find(cmd) != m_Functionmap.end())
			{
				m_Functionmap[cmd]();
			}

		} while (!m_Quit);
	}

	void ConsoleDebug::AddCommand(std::string cmd, std::function<void()> func)
	{
		m_Functionmap.insert(std::make_pair(cmd, func));
	}

}