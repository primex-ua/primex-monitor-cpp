#ifndef HASHER_H
#define HASHER_H

#include <fstream>
#include <string>
#include <windows.h>

class Hasher {
public:
	static std::string simpleHash(std::string input);
};

#endif // HASHER_H
