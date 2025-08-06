#include "Logger.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>

std::ofstream Logger::logFile;
CRITICAL_SECTION Logger::critSection;
bool Logger::initialized = false;

void Logger::enter() {
	if (initialized) EnterCriticalSection(&critSection);
}

void Logger::leave() {
	if (initialized) LeaveCriticalSection(&critSection);
}

void Logger::init(const std::string& logFilePath) {
	if (!initialized) {
		InitializeCriticalSection(&critSection);
		initialized = true;
	}
	enter();
	logFile.open(logFilePath, std::ios::app);
	if (!logFile) {
		std::cerr << "Failed to open log file: " << logFilePath << std::endl;
	}
	else {
		log("Logger initialized.");
	}
	leave();
}

void Logger::log(const std::string& message) {
	enter();
	if (!logFile.is_open()) {
		leave();
		return;
	}

	if (message.length()) {
		logFile << "[" << timestamp() << "] " << message << std::endl;
	}
	else {
		logFile << "" << std::endl;
	}

	leave();
}

void Logger::close() {
	enter();
	if (logFile.is_open()) {
		log("Logger closed.");
		logFile.close();
	}
	leave();
	if (initialized) {
		DeleteCriticalSection(&critSection);
		initialized = false;
	}
}

std::string Logger::timestamp() {
	auto now = std::chrono::system_clock::now();
	auto timeT = std::chrono::system_clock::to_time_t(now);
	std::tm tmResult;

	// XP-safe localtime
	if (localtime_s(&tmResult, &timeT) != 0) {
		std::memset(&tmResult, 0, sizeof(tmResult));
	}

	char buffer[32];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tmResult);
	return std::string(buffer);
}