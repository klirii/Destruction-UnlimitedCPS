#include "Utils.hpp"
#pragma warning(disable:4267)

namespace RestAPI {
	std::string Utils::reverse_str(std::string str) {
		std::string reversedStr = str;
		reverse(reversedStr.begin(), reversedStr.end());
		return reversedStr;
	}

	std::vector<byte> Utils::reverse_bytes(std::vector<byte> bytes) {
		std::vector<byte> reversed;
		for (int i = (bytes.size() - 1); i >= 0; i--) reversed.push_back(bytes[i]);
		return reversed;
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

	std::string Utils::n2hexstr(uint8_t w, size_t hex_len) {
		static const char* digits = "0123456789ABCDEF";
		std::string rc(hex_len, '0');
		for (size_t i = 0, j = (hex_len - 1) * 4; i < hex_len; ++i, j -= 4)
			rc[i] = digits[(w >> j) & 0x0f];
		return rc;
	}

	size_t CURLUtils::response2string(void* data, size_t size, size_t nmemb, void* userp) {
		((std::string*)userp)->append((char*)data, size * nmemb);
		return size * nmemb;
	}
}