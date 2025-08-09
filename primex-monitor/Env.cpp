#include "Env.h"
#include "INIReader.h"
#include "path.h"
#include "Logger.h"
#include <iostream>
#include <memory>

std::string exeDir = getExecutableDir();
std::string configPath = exeDir + "\\primex-monitor-config.ini";

Env* Env::instance = nullptr;

Env::Env() : reader(configPath) {
	if (reader.ParseError() < 0) {
		Logger::log("Can't load config file: " + configPath);
		throw std::runtime_error("Failed to load config file.");
	}

	if (!reader.HasValue("database", "path")) {
		Logger::log("Missing value: [database] path in " + configPath);
		throw std::runtime_error("Database path not specified in primex-monitor-config.ini");
	}

	if (!reader.HasValue("api", "url")) {
		Logger::log("Missing value: [api] url in " + configPath);
		throw std::runtime_error("API URL not specified in primex-monitor-config.ini");
	}

	if (!reader.HasValue("api", "key")) {
		Logger::log("Missing value: [api] key in " + configPath);
		throw std::runtime_error("API key not specified in primex-monitor-config.ini");
	}

	// Log loaded config values
	Logger::log("Config loaded from 'primex-monitor-config.ini'");
	Logger::log("dbPath = " + reader.Get("database", "path", "UNDEFINED"));
	Logger::log("apiUrl = " + reader.Get("api", "url", "UNDEFINED"));
	Logger::log("apiKey = " + reader.Get("api", "key", "UNDEFINED"));
	Logger::log("syncPeriodDays = " + reader.Get("settings", "sync_period_days", "UNDEFINED"));
	Logger::log("");
}

Env& Env::getEnv() {
	if (instance == nullptr) {
		instance = new Env();
	}
	return *instance;
}

std::string Env::getDbPath() {
	return reader.Get("database", "path", "UNDEFINED");
}

std::string Env::getApiUrl() {
	return reader.Get("api", "url", "UNDEFINED");
}

std::string Env::getApiKey() {
	return reader.Get("api", "key", "UNDEFINED");
}

int Env::getSyncPeriodDays() {
	return reader.GetInteger("settings", "sync_period_days", 0);
}
