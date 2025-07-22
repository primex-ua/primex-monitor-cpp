#include "Env.h"
#include <iostream>
#include "INIReader.h"
#include "path.h"
#include <memory>

std::string exeDir = getExecutableDir();
std::string configPath = exeDir + "\\primex-monitor-config.ini";

Env* Env::instance = nullptr;

Env::Env() : reader(configPath) {
	if (reader.ParseError() < 0) {
		std::cout << "Can't load " << configPath << std::endl;
		throw std::runtime_error("Failed to load config file.");
	}

	if (!reader.HasValue("database", "path")) {
		throw std::runtime_error("Database path not specified in primex-monitor-config.ini");
	}

	if (!reader.HasValue("api", "url")) {
		throw std::runtime_error("API URL not specified in primex-monitor-config.ini");
	}
	
	if (!reader.HasValue("api", "key")) {
		throw std::runtime_error("API key not specified in primex-monitor-config.ini");
	}

	std::cout << "Config loaded from 'primex-monitor-config.ini'" << std::endl;
	std::cout << "dbPath = " << reader.Get("database", "path", "UNDEFINED") << std::endl;
	std::cout << "apiUrl = " << reader.Get("api", "url", "UNDEFINED") << std::endl;
	std::cout << "apiKey = " << reader.Get("api", "key", "UNDEFINED") << std::endl;
	std::cout << "syncPeriodDays = " << reader.Get("settings", "sync_period_days", "UNDEFINED") << std::endl;
	std::cout << std::endl;
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