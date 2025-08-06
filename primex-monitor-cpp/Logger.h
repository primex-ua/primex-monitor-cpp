#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <string>
#include <windows.h>

class Logger {
public:
	static void init(const std::string& logFilePath);
	static void log(const std::string& message);
	static void close();

private:
	static std::ofstream logFile;
	static CRITICAL_SECTION critSection;
	static bool initialized;

	static std::string timestamp();
	static void enter();
	static void leave();
};
#endif // !LOGGER_H
