#include <windows.h>
#include <iostream>

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

int main() {
	STARTUPINFOA si = { sizeof(si) };
	PROCESS_INFORMATION pi;

	std::string exeDir = getExecutableDir();
	std::string applicationPath = exeDir + "\\primex-monitor.exe";

	BOOL success = CreateProcessA(
		applicationPath.c_str(),
		NULL, NULL, NULL,
		FALSE,
		0, NULL, NULL,
		&si, &pi
	);

	if (!success) {
		std::cerr << "Failed to start app. Error: " << GetLastError() << std::endl;
		return 1;
	}

	std::cout << "App started successfully." << std::endl;
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	std::cout << "Press Enter to exit...";
	std::cin.get();

	return 0;
}
