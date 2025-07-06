#define _CRT_SECURE_NO_WARNINGS
#define DLL_VIMEWORLD_ATTACH 0xFF

#pragma warning(disable:4312)
#pragma warning(disable:26451)

#include <Windows.h>
#include <thread>

#include "JLI.hpp"

#include "Utils/Hashes.hpp"
#include "Utils/Keybind.hpp"
#include "Utils/ErrorHandler.hpp"

#include "RestAPI/Core/Client.hpp"
#include "RestAPI/Utils/Utils.hpp"
#include "Config/ConfigManager.hpp"

using namespace std;
using json = nlohmann::json;

static RestAPI::Client client;