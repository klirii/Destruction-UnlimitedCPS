#define _CRT_SECURE_NO_WARNINGS
#define DLL_VIMEWORLD_ATTACH 0x888

#pragma warning(disable:4312)
#pragma warning(disable:26451)

#include <Windows.h>
#include <thread>

#include "JNI/Handler.h"
#include "Utils/PatternScanner.hpp"
#include "Utils/ErrorHandler.hpp"
#include "Utils/Hashes.hpp"

#include "JMethodHook/JVM/hotspot/src/share/vm/oops/instanceKlass.hpp"
#include "JMethodHook/Core/JMethodInterceptor.hpp"

#include "RestAPI/Core/Client.hpp"
#include "Config/ConfigManager.hpp"

using namespace std;
using json = nlohmann::json;
namespace RI = RegistersInterceptor;

static RestAPI::Client client;