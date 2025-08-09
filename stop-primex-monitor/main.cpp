#include <windows.h>
#include <iostream>
#include <windows.h>

std::string getExecutableDir() {
	char pathBuffer[1024];

	GetModuleFileNameA(NULL, pathBuffer, sizeof(pathBuffer));

	std::string path = std::string(pathBuffer);

	size_t pos = path.rfind('\\');

	if (pos != std::string::npos) {
		path.erase(pos);
	}

	return path;
}

std::string hash(std::string input) {
	unsigned int hash = 0;
	for (char c : input) {
		hash = hash * 31 + c;
	}

	char buffer[32];
	sprintf_s(buffer, "primex-monitor_%08X", hash);
	return std::string(buffer);
}

int main() {
	std::string exeDir = getExecutableDir();
	std::string shutdownEventName = hash(exeDir) + "_shutdown";

	HANDLE hEvent = OpenEventA(EVENT_MODIFY_STATE, FALSE, shutdownEventName.c_str());
	if (!hEvent) {
		std::cerr << "Failed to open shutdown event. Error: " << GetLastError() << std::endl;
		return 1;
	}

	if (SetEvent(hEvent)) {
		std::cout << "Shutdown signal sent successfully." << std::endl;
	}
	else {
		std::cerr << "Failed to send shutdown signal. Error: " << GetLastError() << std::endl;
	}

	CloseHandle(hEvent);

	std::cout << "Press Enter to exit...";
	std::cin.get();

	return 0;
}
