#include "path.h"
#include <windows.h>

std::string getExecutablePath() {
	char path[1024];
#ifdef _WIN32
	GetModuleFileNameA(NULL, path, sizeof(path)); // Windows API to get the executable path
#elif __linux__
	ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1); // Linux: Read symbolic link to the executable
	if (len != -1) path[len] = '\0';
#elif __APPLE__
	uint32_t size = sizeof(path);
	if (_NSGetExecutablePath(path, &size) == 0) path[size] = '\0'; // macOS API to get executable path
#endif
	return std::string(path);
}

std::string getExecutableDir() {
	std::string path = getExecutablePath();

	size_t pos = path.rfind('\\');

	if (pos != std::string::npos) {
		path.erase(pos);
	}

	return path;
}