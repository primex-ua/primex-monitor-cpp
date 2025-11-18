#ifndef CURSOR_H
#define CURSOR_H

#include <string>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

class Cursor {
public:
	static const json DEFAULT_CURSOR;

	static int setCursor(json cursor);
	static json getCursor();
};

#endif // !CURSOR_H
