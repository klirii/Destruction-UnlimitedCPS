#pragma once
#include <iostream>
#include <nlohmann/json.hpp>

namespace RestAPI {
	using json = nlohmann::json;

	class UserData {
	public:
		std::string name;
		std::string session;
		json data;
	};

	class Client {
	public:
		std::string host;
		UserData user;

		void getkey(std::string username, std::string hash);
	};
}