#include "Logger.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <direct.h>

std::ofstream Logger::logFile;
std::string Logger::logDir = "primex-monitor-logs";
CRITICAL_SECTION Logger::critSection;
bool Logger::initialized = false;
std::string Logger::currentLogDate;

void Logger::enter() {
	if (initialized) EnterCriticalSection(&critSection);
}

void Logger::leave() {
	if (initialized) LeaveCriticalSection(&critSection);
}

void Logger::init() {
	if (!initialized) {
		InitializeCriticalSection(&critSection);
		initialized = true;
	}

	enter();
	createLogDirectoryIfNeeded();

	currentLogDate = getCurrentDate();
	openLogFile();
	deleteOldLogs();
	log("Logger initialized.");
	leave();
}

void Logger::log(const std::string& message) {
	enter();

	std::string today = getCurrentDate();
	if (today != currentLogDate) {
		logFile.close();
		currentLogDate = today;
		openLogFile();
		deleteOldLogs();
	}

	if (!logFile.is_open()) {
		leave();
		return;
	}

	if (!message.empty()) {
		logFile << "[" << timestamp() << "] " << message << std::endl;
	}
	else {
		logFile << std::endl;
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
	localtime_s(&tmResult, &timeT);

	char buffer[32];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tmResult);
	return std::string(buffer);
}

std::string Logger::getCurrentDate() {
	auto now = std::chrono::system_clock::now();
	auto timeT = std::chrono::system_clock::to_time_t(now);
	std::tm tmResult;
	localtime_s(&tmResult, &timeT);

	char buffer[16];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &tmResult);
	return std::string(buffer);
}

void Logger::createLogDirectoryIfNeeded() {
	DWORD attribs = GetFileAttributesA(logDir.c_str());
	if (attribs == INVALID_FILE_ATTRIBUTES || !(attribs & FILE_ATTRIBUTE_DIRECTORY)) {
		CreateDirectoryA(logDir.c_str(), NULL);
	}
}

void Logger::openLogFile() {
	std::string path = logDir + "/" + currentLogDate + ".log";
	logFile.open(path, std::ios::app);
	if (!logFile) {
		std::cerr << "Failed to open log file: " << path << std::endl;
	}
}

void Logger::deleteOldLogs() {
	WIN32_FIND_DATAA findData;
	std::string pattern = logDir + "/*.log";
	HANDLE hFind = FindFirstFileA(pattern.c_str(), &findData);
	if (hFind == INVALID_HANDLE_VALUE) return;

	const std::time_t now = std::time(nullptr);
	std::tm nowTm;
	localtime_s(&nowTm, &now);

	// Cutoff: 7 days ago
	std::tm cutoffTm = nowTm;
	cutoffTm.tm_mday -= 7;
	std::mktime(&cutoffTm);

	do {
		const char* fileName = findData.cFileName;
		std::string dateStr(fileName);
		auto dotPos = dateStr.rfind('.');
		if (dotPos == std::string::npos) continue;
		dateStr = dateStr.substr(0, dotPos);

		std::tm fileTm = {};
		if (sscanf_s(dateStr.c_str(), "%d-%d-%d",
			&fileTm.tm_year, &fileTm.tm_mon, &fileTm.tm_mday) != 3) continue;

		fileTm.tm_year -= 1900;
		fileTm.tm_mon -= 1;
		std::time_t fileTime = std::mktime(&fileTm);
		if (fileTime == -1) continue;

		if (std::difftime(fileTime, std::mktime(&cutoffTm)) < 0) {
			std::string fullPath = logDir + "/" + fileName;
			DeleteFileA(fullPath.c_str());
		}

	} while (FindNextFileA(hFind, &findData));

	FindClose(hFind);
}
