#ifndef SEND_DATA_H
#define SEND_DATA_H

#define CURL_STATICLIB

#include <string>

bool sendData(const std::string& url, const std::string& apiKey, const std::string& jsonString);

#endif // !SEND_DATA_H
