#pragma once
#include <iostream>

class ConfigManager {
public:
	static std::string ParseUsername(bool game = false);
private:
	static std::string Loader;
	static std::string Game;
};