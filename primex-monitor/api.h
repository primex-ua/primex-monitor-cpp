#ifndef API_H
#define API_H

#define CURL_STATICLIB

#include <string>

bool sendData(const std::string& url, const std::string& apiKey, const std::string& systemUUID, const std::string& jsonString);

bool sendHeartbeat(const std::string& url, const std::string& apiKey, const std::string& systemUUID);

#endif // !API_H
