#pragma once
#include <Windows.h>
#include <iostream>
#include <vector>

namespace RestAPI {
	class Utils {
	public:
		static std::string reverse_str(std::string str);
		static std::vector<byte> reverse_bytes(std::vector<byte> bytes);

		static std::vector<byte> hex2bytes(const std::string& hex);
		static std::string n2hexstr(uint8_t w, size_t hex_len = sizeof(uint8_t) << 1);
	};

	class CURLUtils {
	public:
		static size_t response2string(void* data, size_t size, size_t nmemb, void* userp);
	};
}