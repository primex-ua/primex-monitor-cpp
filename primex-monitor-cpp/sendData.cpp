#include "sendData.h"
#include "Logger.h"
#include <curl/curl.h>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
	size_t totalSize = size * nmemb;
	std::string* response = static_cast<std::string*>(userp);
	response->append(static_cast<char*>(contents), totalSize);
	return totalSize;
}

bool sendData(const std::string& url, const std::string& apiKey, const std::string& systemUUID, const std::string& jsonString) {
	CURL* curl = curl_easy_init();
	if (!curl) {
		Logger::log("Failed to initialize cURL");
		return false;
	}

	CURLcode res;
	long httpResponseCode = 0;
	std::string responseBody;

	struct curl_slist* headers = nullptr;
	headers = curl_slist_append(headers, "Content-Type: application/json");

	std::string cookieHeader = "X-API-KEY=" + apiKey + "; X-SYSTEM-UUID=" + systemUUID;

	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonString.c_str());
	curl_easy_setopt(curl, CURLOPT_COOKIE, cookieHeader.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);  // Avoid blocking forever

	Logger::log("Sending data to: " + url);
	res = curl_easy_perform(curl);

	if (res != CURLE_OK) {
		Logger::log("cURL perform failed: " + std::string(curl_easy_strerror(res)));
		curl_easy_cleanup(curl);
		curl_slist_free_all(headers);
		return false;
	}

	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpResponseCode);

	Logger::log("HTTP Status: " + std::to_string(httpResponseCode));
	Logger::log("Server Response: " + responseBody);

	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);

	if (httpResponseCode != 201) {
		Logger::log("Server responded with error code: " + std::to_string(httpResponseCode));
		return false;
	}

	Logger::log("Data sent successfully");
	return true;
}
