#include "Client.hpp"

#include "../Utils/Utils.hpp"
#include "../../Utils/Time.hpp"

// Network
#pragma comment(lib, "wldap32.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Normaliz.lib")

#define CURL_STATICLIB
#include <curl/curl.h>

#include "vault.hpp"
#include "vault.cpp"

#pragma warning(disable:26812)
#pragma warning(disable:4244)

namespace RestAPI {
	std::vector<byte> rolling_xor(std::vector<byte> bytes, bool decode = false) {
		if (decode) {
			reverse(bytes.begin(), bytes.end());
			std::vector<byte> output;

			for (int i = 0; i < bytes.size(); i++) {
				if (i == (bytes.size() - 1)) output.push_back(bytes[i]);
				else output.push_back(bytes[i] ^ bytes[i + 1]);
			}

			reverse(output.begin(), output.end());
			return output;
		}
		else {
			std::vector<byte> output(1);
			output[0] = bytes[0];

			for (int i = 0; i < (bytes.size() - 1); i++) output.push_back(bytes[i + 1] ^ output[i]);
			return output;
		}
	}

	std::string __forceinline generate_hash(std::string servertime) {
		for (uint8_t i = 0; i < 22; i++) {
			srand(::Utils::Time::now.UnixNano());
			servertime += Utils::n2hexstr(rand() % 16, 1);
			Sleep(1);
		}
		return servertime;
	}

	std::string __forceinline decrypt(json data) {
		CURL* curl = curl_easy_init();
		CURLcode reqCode;

		std::string url;
		std::string response;
		json encryptData;

		std::string hwid = Utils::reverse_str(data["HWID"]);
		url = "https://destructiqn.com:9990/encrypt=" + hwid + "/hash=" + generate_hash(to_string(data["servertime"]));

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CURLUtils::response2string);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		reqCode = curl_easy_perform(curl);
		if (reqCode != CURLE_OK) exit(0);
		encryptData = json::parse(response);

		std::string hash = Utils::reverse_str(encryptData["hash"]);
		std::vector<byte> hashBytes = Utils::hex2bytes(hash);
		for (uint8_t i = 0; i < 3; i++) hashBytes = rolling_xor(hashBytes, true);
		hashBytes = Utils::reverse_bytes(hashBytes);

		std::stringstream key;
		for (byte b : hashBytes) key << (char)b;

		std::vector<byte> contentBytes = Utils::hex2bytes(data["content"]);
		std::stringstream content;
		for (byte b : contentBytes) content << (char)b;

		curl_easy_cleanup(curl);
		return vault::ARC4(content.str(), key.str());
	}

	void Client::getkey(std::string username, std::string hash) {
		CURL* curl = curl_easy_init();
		CURLcode reqCode;

		std::string url = this->host + "/getkey=" + username + "/hash=" + hash;
		std::string response;
		json data;

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, RestAPI::CURLUtils::response2string);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		reqCode = curl_easy_perform(curl);
		if (reqCode != CURLE_OK) {
			std::cout << url << std::endl;
			Sleep(30 * 1000);
		}
		data = json::parse(response);

		response = decrypt(data);
		replace(response.begin(), response.end(), '\'', '\"');
		data = json::parse(response);

		this->user.data = data;
		curl_easy_cleanup(curl);
	}
}