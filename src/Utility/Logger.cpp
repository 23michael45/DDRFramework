#include "Logger.h"
#include <stdio.h>
#include <stdarg.h>

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
			log->setTarget(Log::Target::STDOUT | Log::Target::LOG_FILE);
			log->setFile(GetTimeNowstring() + ".log");

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

			char timeStr[80];
			strftime(timeStr, 80, "%d/%b/%Y:%H:%M:%S %z", timeStruct);
			toLog += " [" + std::string(timeStr) + "]";
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

}