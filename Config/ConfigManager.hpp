#pragma once
#include <iostream>

class ConfigManager {
public:
	static std::string ParseUsername();
private:
	static std::string Loader;
};