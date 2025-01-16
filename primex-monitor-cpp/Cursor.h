#ifndef CURSOR_H
#define CURSOR_H

#include <string>

class Cursor {
public:
	static int setCursor(std::string cursor);
	static std::string getCursor();
	static std::string getStartingTimestamp();
};

#endif // !CURSOR_H
