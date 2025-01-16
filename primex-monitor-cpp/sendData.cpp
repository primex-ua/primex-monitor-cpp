#include "sendData.h"
#include <iostream>
#include <curl/curl.h>

bool sendData(const std::string& url, const std::string& apiKey, const std::string& jsonString) {
	CURL *curl;
	CURLcode res;
	long httpResponseCode = 0;

	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	if (curl) {
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Content-Type: application/json");
		std::string cookieHeader = "X-API-KEY=" + apiKey;

		// curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonString.c_str());
		curl_easy_setopt(curl, CURLOPT_COOKIE, cookieHeader.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		res = curl_easy_perform(curl);

		std::cout << std::endl;

		if (res != CURLE_OK) {
			std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
			curl_easy_cleanup(curl);
			curl_slist_free_all(headers);
			curl_global_cleanup();
			return false;
		}

		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpResponseCode);

		if (httpResponseCode != 201) {
			std::cerr << "Server responded with error code: " << httpResponseCode << std::endl;
			curl_easy_cleanup(curl);
			curl_slist_free_all(headers);
			curl_global_cleanup();
			return false;
		}

		std::cout << "Data sent successfully" << std::endl;

		curl_easy_cleanup(curl);
		curl_slist_free_all(headers);
	}

	curl_global_cleanup();
	return true;
}