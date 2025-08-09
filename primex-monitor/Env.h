#ifndef ENV_H
#define ENV_H

#include <iostream>
#include "INIReader.h"
#include "path.h"
#include <memory>

class Env {
private:
	static Env* instance;
	INIReader reader;

	Env();

public:
	static Env& getEnv();

	Env(const Env&) = delete;
	Env& operator=(const Env&) = delete;

	std::string getDbPath();
	std::string getApiKey();
	std::string getApiUrl();
	int getSyncPeriodDays();
};

#endif // !ENV_H