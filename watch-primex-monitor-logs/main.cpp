#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <sstream>
#include <iomanip>

std::string getTodayLogPath() {
	SYSTEMTIME st;
	GetLocalTime(&st);

	std::ostringstream oss;
	oss << ".\\primex-monitor-logs\\"
		<< st.wYear << "-"
		<< std::setw(2) << std::setfill('0') << st.wMonth << "-"
		<< std::setw(2) << std::setfill('0') << st.wDay
		<< ".log";
	return oss.str();
}

void tailFile(const std::string &filename, size_t tailLines = 20, int pollMs = 500) {
	std::ifstream file(filename, std::ios::binary);
	if (!file) {
		std::cerr << "Cannot open file: " << filename << "\n";
		return;
	}

	// Seek to last tailLines from end
	file.seekg(0, std::ios::end);
	std::streamoff fileSize = file.tellg();
	std::streamoff pos = fileSize;
	size_t linesFound = 0;

	while (pos > 0 && linesFound <= tailLines) {
		pos--;
		file.seekg(pos);
		char ch;
		file.get(ch);
		if (ch == '\n') {
			linesFound++;
		}
	}
	if (pos < 0) pos = 0;
	file.clear();
	file.seekg(pos);

	std::string line;
	while (true) {
		if (std::getline(file, line)) {
			std::cout << line << std::endl;
		}
		else {
			if (file.eof()) {
				file.clear();
				std::this_thread::sleep_for(std::chrono::milliseconds(pollMs));
			}
			else {
				std::cerr << "Error reading file\n";
				break;
			}
		}
	}
}

int main(int argc, char *argv[]) {
	size_t tailLines = (argc >= 2) ? std::stoul(argv[1]) : 100;

	std::string logPath = getTodayLogPath();
	std::cout << "Watching log file: " << logPath << "\n";

	tailFile(logPath, tailLines);
	return 0;
}
