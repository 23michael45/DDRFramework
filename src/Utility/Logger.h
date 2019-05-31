/*!
 * File: Logger.h
 * Date: 2019/04/29 16:00
 *
 * Author: michael
 * Contact: michael2345@live.cn
 *
 * Description:A common log help class
 *
*/
#ifndef Logger_h__
#define Logger_h__


#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <memory>
#include <stdarg.h>
#include "Singleton.h"
#include <functional>
#include <vector>

using namespace std;

namespace DDRFramework {
	class Log {
	public:
		enum Target : short {
			DISABLED = 1,
			STDOUT = 2,
			STDERR = 4,
			LOG_FILE = 8
		};

		enum Level : short {
			DEBUG = 1,
			INFO = 2,
			NOTICE = 3,
			WARNING = 4,
			ERR = 5,
			CRIT = 6,
			ALERT = 7,
			EMERG = 8
		};

		/**
		 * Get singleton instance.
		 *
		 * \return	Log*	Pointer to Log instance
		 */
		static std::shared_ptr<Log> getInstance();

		/**
		 * Set the location to log messages.
		 *
		 * \param	Target	The log target
		 */
		void setTarget(Target target);

		/**
		 * Set the severity of messages to log.
		 *
		 * \param	Level	The log level to set
		 */
		void setLevel(Level level);

		/**
		 * Set a file to log to if the target is LOG_FILE.
		 *
		 * \param	std::string	The file to which we will log
		 */
		int setFile(std::string fileName);

		/**
		 * Get the current log level. Only messages
		 * with equal or higher severity will be written.
		 *
		 * \return	Log::Level	The current log level
		 */
		Level getLevel();

		/**
		 * Convert the Log::Level enum to a std::string.
		 *
		 * \param	Log::Level	The level to convert
		 * \return	std::string	The converted level
		 */
		std::string levelTostring(Level level);

		/**
		 * Log a message.
		 *
		 * \param	Level	The severity of the message
		 * \param	std::string	The message to write
		 */
		void write(Level level, std::string message);

		/**
		 * Date and time will no longer be printed with each log message.
		 */
		void excludeTimestamp() {
			this->timestampEnabled = false;
		}

		/**
		 * Date and time will be printed with each log message.
		 */
		void includeTimestamp() {
			this->timestampEnabled = true;
		}

		/**
		 * Log level will no longer be printed with each log message.
		 */
		void excludeLogLevel() {
			this->levelEnabled = false;
		}

		/**
		 * Log level will be printed with each log message.
		 */
		void includeLogLevel() {
			this->levelEnabled = true;
		}

		Target getTarget()
		{
			return logTarget;
		}

	protected:
		Log() {}

	private:
		// Our single instance of the class
		static std::shared_ptr<Log> log;

		// write() uses these variables to determine which messages should be written where.
		Level logLevel = Level::INFO;
		Target logTarget = Target::STDOUT;
		std::string logFile = "";

		// Flags that change log style
		bool timestampEnabled = true;
		bool levelEnabled = true;

		// std::string representations of log levels
		map< Level, std::string > levelMap = {
			{ Level::DEBUG, "DEBUG" },
			{ Level::INFO, "INFO" },
			{ Level::NOTICE, "NOTICE" },
			{ Level::WARNING, "WARNING" },
			{ Level::ERR, "ERROR" },
			{ Level::CRIT, "CRITICAL" },
			{ Level::ALERT, "ALERT" },
			{ Level::EMERG, "EMERGENCY" }
		};
	};

	////////////////////////
	// Bit-wise operators //
	////////////////////////
	inline Log::Target operator&(Log::Target a, Log::Target b) {
		return static_cast<Log::Target>(static_cast<short>(a) & static_cast<short>(b));
	}
	inline Log::Target operator|(Log::Target a, Log::Target b) {
		return static_cast<Log::Target>(static_cast<short>(a) | static_cast<short>(b));
	}

	void Print(const char* format, ...);

	void DDRLog(Log::Level level,const char* format, ...);
	void DDRLogArgs(DDRFramework::Log::Level level, const char* format, va_list args);
	class ConsoleDebug
	{

	public:
		ConsoleDebug();

		void ConsoleDebugLoop();
		void AddCommand(std::string cmd, std::function<void()> func);

		void ToggleLogConsole();
		void ToggleLogFile();
		void Quit();



	protected:

		std::map<std::string, std::function<void()>> m_Functionmap;

		bool m_Quit;
		bool m_ToggleLogConsole;
		bool m_ToggleLogFile;

		std::string m_CurrentCmd;
	};


}

#endif // Logger_h__