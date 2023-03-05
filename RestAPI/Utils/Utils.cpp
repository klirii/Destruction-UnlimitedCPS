#include "Utils.hpp"
#pragma warning(disable:4267)

namespace RestAPI {
	std::string Utils::reverse_str(std::string str) {
		std::string reversedStr = str;
		reverse(reversedStr.begin(), reversedStr.end());
		return reversedStr;
	}

	std::vector<byte> Utils::hex2bytes(const std::string& hex) {
		std::vector<byte> bytes;

		for (unsigned int i = 0; i < hex.length(); i += 2) {
			std::string byteString = hex.substr(i, 2);
			char bytee = (char)strtol(byteString.c_str(), nullptr, 16);
			bytes.push_back(bytee);
		}

		return bytes;
	}

	size_t CURLUtils::response2string(void* data, size_t size, size_t nmemb, void* userp) {
		((std::string*)userp)->append((char*)data, size * nmemb);
		return size * nmemb;
	}
}