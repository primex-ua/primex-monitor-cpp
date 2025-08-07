#include <string>
#include "Hasher.h"

std::string Hasher::simpleHash(std::string input) {
	unsigned int hash = 0;
	for (char c : input) {
		hash = hash * 31 + c;
	}

	char buffer[32];
	sprintf_s(buffer, "PrimexMonitorMutex_%08X", hash);
	return std::string(buffer);
}