#include "network.hpp"

#include <cstdlib>
#include <cstring>
#include <list>
#include <string>
#include <curl/curl.h>

#include "json.hpp"

const char* userAgent = "3DShock/0.0.0";
char token[] = "SBB7r75ogF1Iul8CKPVgaBAFy62MIFdhCSIOuF4Moyom421YQICZLUS0vo8vvrp7";
char apiUrl[] = "http://10.215.28.228/";
User user = {token};
#define HTTP_CONTENT_LENGTH_HEADER "Content-Length"
#define HTTP_TIMEOUT_SEC 5

typedef struct {
	u32 bufferSize;
	u64* contentLength;
	void* userData;
	Result (*callback)(void* userData, void* buffer, size_t size);

	void* buf;
	u32 pos;

	Result res;
} http_curl_data;

static size_t http_curl_header_callback(const char* buffer, size_t size, size_t nitems, void* userdata) {
	auto* curlData = static_cast<http_curl_data *>(userdata);

	size_t bytes = size * nitems;
	size_t headerNameLen = strlen(HTTP_CONTENT_LENGTH_HEADER);

	if(bytes >= headerNameLen && strncmp(buffer, HTTP_CONTENT_LENGTH_HEADER, headerNameLen) == 0) {
		char* separator = strstr(buffer, ": ");
		if(separator != nullptr) {
			char* valueStart = separator + 2;

			char value[32];
			memset(value, '\0', sizeof(value));
			strncpy(value, valueStart, bytes - (valueStart - buffer));

			if(curlData->contentLength != nullptr) {
				*(curlData->contentLength) = static_cast<u64>(atoll(value));
			}
		}
	}

	return size * nitems;
}

static size_t http_curl_write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
	auto curlData = static_cast<http_curl_data *>(userdata);

	size_t available = size * nmemb;
	while(available > 0) {
		size_t remaining = curlData->bufferSize - curlData->pos;
		size_t copySize = available < remaining ? available : remaining;

		memcpy((u8*) curlData->buf + curlData->pos, ptr, copySize);
		curlData->pos += copySize;
		available -= copySize;

		if(curlData->pos == curlData->bufferSize) {
			curlData->res = curlData->callback(curlData->userData, curlData->buf, curlData->bufferSize);
			curlData->pos = 0;
		}
	}

	return R_SUCCEEDED(curlData->res) ? size * nmemb : 0;
}

char* concatenateCharPtrAndStr(const char* charPtr, const std::string& str) {
	// Calculate the length of the resulting string
	size_t strLength = str.length();
	size_t charPtrLength = strlen(charPtr);
	size_t totalLength = strLength + charPtrLength + 1; // +1 for the null terminator

	// Allocate memory for the resulting string
	char* result = new char[totalLength];
	// Copy the contents of the char* to the result
	strcpy(result, charPtr);
	// Append the contents of the std::string to the result
	strcpy(result + charPtrLength, str.c_str());

	return result;
}

char* concatenateStringAndCharPtr(const std::string& str, const char* charPtr) {
	// Calculate the length of the resulting string
	size_t strLength = str.length();
	size_t charPtrLength = strlen(charPtr);
	size_t totalLength = strLength + charPtrLength + 1; // +1 for the null terminator

	// Allocate memory for the resulting string
	char *result = new char[totalLength];

	// Copy the contents of the std::string to the result
	strcpy(result, str.c_str());

	// Append the contents of the char* to the result
	strcpy(result + strLength, charPtr);

	return result;
}

Result http_get(const char *url, u8 **buf, char* token)
{
	CURL *curl = curl_easy_init();
	if (curl) {
		constexpr int bufferSize = 4096;
		u64* contentLength = nullptr;
		void* userData = nullptr;
		Result (*callback)(void* userData, void* buffer, size_t size) = nullptr;
		*buf = static_cast<u8 *>(malloc(bufferSize));
		for (int i = 0; i < bufferSize; i++) {
			(*buf)[i] = 0;
		}
		http_curl_data curlData = {bufferSize, contentLength, userData, callback, *buf, 0, 0};

		curl_slist *headers = nullptr;
		headers = curl_slist_append(headers, "Host: api.openshock.zap");
		headers = curl_slist_append(headers, concatenateStringAndCharPtr("OpenShockToken: ", token));

		// Set up curl options
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent);
		curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, bufferSize);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, HTTP_TIMEOUT_SEC);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_curl_write_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*) &curlData);
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, http_curl_header_callback);
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void*) &curlData);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		// Perform the request
		CURLcode res = curl_easy_perform(curl);
		free(contentLength);
		curl_slist_free_all(headers);
		if (res != CURLE_OK) {
			printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			return res;
		}

		// Cleanup
		curl_easy_cleanup(curl);
		return 0;
	}
	printf("Failed to initialize libcurl.\n");
	return 1;
}

using json = nlohmann::json;

void printShockerDetails(Shocker s) {
	printf("Shocker ID: %s\n", s.id);
	printf("Shocker Name: %s\n", s.name);
	printf("Shocker Model: %s\n", s.model);
	printf("Shocker isPaused: %s\n", s.isPaused ? "true" : "false");
	printf("Shocker Limits Intensity: %d\n", s.limits.intensity);
	printf("Shocker Limits Duration: %d\n", s.limits.duration);
	printf("Shocker Permissions Shock: %s\n", s.permissions.shock ? "true" : "false");
	printf("Shocker Permissions Vibrate: %s\n", s.permissions.vibrate ? "true" : "false");
	printf("Shocker Permissions Sound: %s\n", s.permissions.sound ? "true" : "false");
	printf("Shocker Permissions Live: %s\n", s.permissions.live ? "true" : "false");
}

Shocker parseShocker(nlohmann::basic_json<>& shocker) {
	Shocker parsedShocker = {};

	parsedShocker.name = strdup(shocker["name"].get<std::string>().c_str());
	parsedShocker.id = strdup(shocker["id"].get<std::string>().c_str());
	if (shocker["model"] != nullptr) parsedShocker.model = strdup(shocker["model"].get<std::string>().c_str());
	else parsedShocker.model = strdup("Unknown");
	parsedShocker.isPaused = shocker["isPaused"].get<bool>();
	if (shocker["limits"] != nullptr) {
		parsedShocker.limits.intensity = shocker["limits"]["intensity"].get<int>();
		parsedShocker.limits.duration = shocker["limits"]["duration"].get<int>();
	} else {
		parsedShocker.limits.intensity = 100;
		parsedShocker.limits.duration = 30000;
	}
	if (shocker["permissions"] != nullptr) {
		parsedShocker.permissions.shock = shocker["permissions"]["shock"].get<bool>();
		parsedShocker.permissions.vibrate = shocker["permissions"]["vibrate"].get<bool>();
		parsedShocker.permissions.sound = shocker["permissions"]["sound"].get<bool>();
		parsedShocker.permissions.live = shocker["permissions"]["live"].get<bool>();
	} else {
		parsedShocker.permissions.shock = true;
		parsedShocker.permissions.vibrate = true;
		parsedShocker.permissions.sound = true;
		parsedShocker.permissions.live = true;
	}

	return parsedShocker;
}

json tmpShockers;
void getShockers(std::list<Shocker> shockers) {
	printf("Getting shockers...\n");
	u8* buf;
	shockers.clear();

	Result r = http_get(concatenateCharPtrAndStr(apiUrl, "1/shockers/own"), &buf, user.token);
	printf("HTTP_GET returned 0x%08X\n", r);
	if (buf) {
		printf("Response body: %s\n", reinterpret_cast<char *>(buf));
		printf("\nParsing\n\n");
		tmpShockers = json::parse(buf);
		printf("\nParsed\n\n");
		if (tmpShockers["data"] != nullptr) {
			for (auto& hub : tmpShockers["data"]) {
				if (hub["name"] != nullptr) printf("\nName:%s\n", hub["name"].get<std::string>().c_str());
				else printf("\nName:Unknown\n");
				if (hub["shockers"] != nullptr) {
					for (auto& shocker : hub["shockers"]) {
						Shocker parsedShocker = parseShocker(shocker);
						shockers.push_back(parsedShocker);
					}
				}
			}
		}
		free(buf);
	} else {
		printf("Buffer was freed\n");
	}
	printf("Next request duh\n");
	r = http_get(concatenateCharPtrAndStr(apiUrl, "1/shockers/shared"), &buf, user.token);
	printf("HTTP_GET returned 0x%08X\n", r);
	if (buf) {
		printf("Response body: %s\n", reinterpret_cast<char *>(buf));
		printf("\nParsing\n\n");
		sleep(1);
		json newTmpShockers = json::parse(buf);
		printf("\nParsed\n\n");
		if (newTmpShockers["data"] != nullptr) {
			for (auto& deviceCollection : newTmpShockers["data"]) {
				if (deviceCollection["devices"] != nullptr) {
					for (auto& hub : deviceCollection["devices"]) {
						printf("\nName:%s\n", hub["name"].get<std::string>().c_str());
						if (hub["shockers"] != nullptr) {
							for (auto& shocker : hub["shockers"]) {
								Shocker parsedShocker = parseShocker(shocker);
								shockers.push_back(parsedShocker);
							}
						}
					}
				}
			}
		}
		free(buf);
	} else {
		printf("Buffer was freed\n");
	}
	printf("Got%s %d shockers\n", !shockers.empty() ? "" : " no", static_cast<int>(shockers.size()));
}