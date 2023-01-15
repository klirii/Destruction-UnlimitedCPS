#define _CRT_SECURE_NO_WARNINGS
#include "ConfigManager.hpp"
#include <fstream>

std::string ConfigManager::Loader = "";

std::string ConfigManager::ParseUsername() {
	Loader = std::string(getenv("APPDATA")) + "\\.vimeworld\\jre-x64\\lib\\security\\java8.security";

	char username[12];
	std::ifstream(Loader).getline(username, 12);

	if (std::string(username).empty()) return "";
	return std::string(username);
}