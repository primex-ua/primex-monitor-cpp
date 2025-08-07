#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <string>
#include <windows.h>

class Logger {
public:
	static void init();
	static void log(const std::string& message);
	static void close();

private:
	static std::ofstream logFile;
	static std::string logDir;
	static CRITICAL_SECTION critSection;
	static bool initialized;

	static std::string timestamp();
	static void enter();
	static void leave();

	static std::string getCurrentDate();
	static void openLogFile();
	static void deleteOldLogs();
	static void createLogDirectoryIfNeeded();
	static std::string currentLogDate;
};

#endif // LOGGER_H
