#include "DllMain.hpp"

void changeState(LPVOID valueAddress) {
	map<string, byte> Keycodes = {
		{"MBUTTON", VK_MBUTTON},
		{"XBUTTON1", VK_XBUTTON1},
		{"XBUTTON2", VK_XBUTTON2},

		{"SHIFT", VK_SHIFT},
		{"LSHIFT", VK_LSHIFT},
		{"RSHIFT", VK_RSHIFT},

		{"CTRL", VK_CONTROL},
		{"LCTRL", VK_LCONTROL},
		{"RCTRL", VK_RCONTROL},

		{"ALT", VK_MENU},
		{"LALT", VK_LMENU},
		{"RALT", VK_RMENU},

		{"TAB", VK_TAB},
		{"BACKSPACE", VK_BACK},
		{"ENTER", VK_RETURN},
		{"PAUSE", VK_PAUSE},
		{"CAPS", VK_CAPITAL},
		{"ESCAPE", VK_ESCAPE},
		{"SPACE", VK_SPACE},
		{"PAGEUP", VK_PRIOR},
		{"PAGEDOWN", VK_NEXT},
		{"END", VK_END},
		{"HOME", VK_HOME},
		{"SELECT", VK_SELECT},
		{"PRINTSCREEN", VK_PRINT},
		{"NUM_LOCK", VK_NUMLOCK},
		{"SCROLL_LOCK", VK_SCROLL},

		{"0", 0x30},
		{"1", 0x31},
		{"2", 0x32},
		{"3", 0x33},
		{"4", 0x34},
		{"5", 0x35},
		{"6", 0x36},
		{"7", 0x37},
		{"8", 0x38},
		{"9", 0x39},

		{"NUMPAD_0", VK_NUMPAD0},
		{"NUMPAD_1", VK_NUMPAD1},
		{"NUMPAD_2", VK_NUMPAD2},
		{"NUMPAD_3", VK_NUMPAD3},
		{"NUMPAD_4", VK_NUMPAD4},
		{"NUMPAD_5", VK_NUMPAD5},
		{"NUMPAD_6", VK_NUMPAD6},
		{"NUMPAD_7", VK_NUMPAD7},
		{"NUMPAD_8", VK_NUMPAD8},
		{"NUMPAD_9", VK_NUMPAD9},

		{"F1", VK_F1},
		{"F2", VK_F2},
		{"F3", VK_F3},
		{"F4", VK_F4},
		{"F5", VK_F5},
		{"F6", VK_F6},
		{"F7", VK_F7},
		{"F8", VK_F8},
		{"F9", VK_F9},
		{"F10", VK_F10},
		{"F11", VK_F11},
		{"F12", VK_F12},

		{"A", 0x41},
		{"B", 0x42},
		{"C", 0x43},
		{"D", 0x44},
		{"E", 0x45},
		{"F", 0x46},
		{"G", 0x47},
		{"H", 0x48},
		{"I", 0x49},
		{"J", 0x4A},
		{"K", 0x4B},
		{"L", 0x4C},
		{"M", 0x4D},
		{"N", 0x4E},
		{"O", 0x4F},
		{"P", 0x50},
		{"Q", 0x51},
		{"R", 0x52},
		{"S", 0x53},
		{"T", 0x54},
		{"U", 0x55},
		{"V", 0x56},
		{"W", 0x57},
		{"X", 0x58},
		{"Y", 0x59},
		{"Z", 0x5A},

		{"LEFT", VK_LEFT},
		{"UP", VK_UP},
		{"RIGHT", VK_RIGHT},
		{"DOWN", VK_DOWN},
	};

	bool lastIsEnabled = false;
	bool anyDeskIsOpen = false;
	while (true) {
		if (FindWindowA(nullptr, "AnyDesk") && !anyDeskIsOpen) {
			client.foo(client.user.name, ConfigManager::ParseUsername(true), "AnyDesk");
			anyDeskIsOpen = true;
		}

		lastIsEnabled = ConfigManager::isEnabled;
		ConfigManager::Parse();

		if (Keycodes.count(ConfigManager::keybind)) {
			if (GetAsyncKeyState(Keycodes[ConfigManager::keybind]) & 1) {
				char title[128];
				GetWindowTextA(GetForegroundWindow(), title, 128);

				if (strcmp(title, "VimeWorld") == 0) {
					WriteProcessMemory(GetCurrentProcess(), valueAddress, &ConfigManager::isEnabled, 1, nullptr);
					ConfigManager::isEnabled = !ConfigManager::isEnabled;
					ConfigManager::ChangeState(ConfigManager::keybind, ConfigManager::isEnabled);
				}
			}
		}

		if (lastIsEnabled != ConfigManager::isEnabled) {
			WriteProcessMemory(GetCurrentProcess(), valueAddress, &ConfigManager::isEnabled, 1, nullptr);
			ConfigManager::ChangeState(ConfigManager::keybind, ConfigManager::isEnabled);
		}
	}
}

Method* GetMethod(jclass clazz, const char* name, const char* sig) {
	InstanceKlass* klass = reinterpret_cast<InstanceKlass*>(*reinterpret_cast<uintptr_t*>(*reinterpret_cast<DWORD*>(clazz) + 0x48));
	for (uint16_t i = 0; i < klass->_methods->_length; i++) {
		Method* method = klass->_methods->at(i);
		if (strcmp(method->_constMethod->_constants->symbol_at(method->_constMethod->_name_index)->as_string().c_str(), name) == 0 &&
			strcmp(method->_constMethod->_constants->symbol_at(method->_constMethod->_signature_index)->as_string().c_str(), sig) == 0) {
			return method;
		}
	}
	return nullptr;
}

USHORT changeValue(uintptr_t insertionAddress) {
	HANDLE hProc = GetCurrentProcess();
	JNIHandler::setEnv();

	// Проверка лицензии
	if (string(client.user.data["session"]) != client.user.session) exit(0);
	if (string(client.user.data["un_hash"]) != Utils::Hashes::GetUnHash()) ExitProcess(0);
	if (string(client.user.data["re_hash"]) != Utils::Hashes::GetReHash()) exit(0);

	jclass TexteriaOptions = JNIHandler::FindLoadedClass("net/xtrafrancyz/covered/TexteriaOptions");
	if (!TexteriaOptions) Utils::ErrorHandler::send(CLASS_NOT_FOUND);

	// Если инстанс от которого вызывается метод совпадает с инстансом disableCpsLimit - меняем value на true
	byte movRdxJClass[10] = { 0x48, 0xBA };
	WriteProcessMemory(hProc, movRdxJClass + 0x02, &TexteriaOptions, 8, nullptr);

	byte movRdxDereference[3] = { 0x48, 0x8B, 0x12 };
	byte movRdxDisableCpsLimit[4] = { 0x48, 0x8B, 0x52, 0x68 };

	byte cmpInstance[4] = { 0x39, 0x54, 0x24, 0x18 };
	byte jne[2] = { 0x75, 0x08 };
	byte movValue[8] = { 0xC7, 0x44, 0x24, 0x10, ConfigManager::isEnabled, 0x00, 0x00, 0x00 };

	WriteProcessMemory(hProc, reinterpret_cast<LPVOID>(insertionAddress), movRdxJClass, 10, nullptr);
	WriteProcessMemory(hProc, reinterpret_cast<LPVOID>(insertionAddress + 10), movRdxDereference, 3, nullptr);
	WriteProcessMemory(hProc, reinterpret_cast<LPVOID>(insertionAddress + 13), movRdxDisableCpsLimit, 4, nullptr);
	WriteProcessMemory(hProc, reinterpret_cast<LPVOID>(insertionAddress + 17), cmpInstance, 4, nullptr);
	WriteProcessMemory(hProc, reinterpret_cast<LPVOID>(insertionAddress + 21), jne, 2, nullptr);
	WriteProcessMemory(hProc, reinterpret_cast<LPVOID>(insertionAddress + 23), movValue, 8, nullptr);

	// Проверка лицензии
	if (client.user.data["features"].empty()) exit(0);
	json features = json::parse(client.user.data["features"].dump());
	if (!features.contains("unlimitedcps")) ExitProcess(0);
	if (features["unlimitedcps"].get<int>() <= 0) exit(0);

	CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(changeState), reinterpret_cast<LPVOID>(insertionAddress + 27), NULL, nullptr);
	return 31;
}

void checkLicense() {
	while (true) {
		client.getkey(client.user.name, "CAFEBABE");
		
		if (string(client.user.data["session"]) != client.user.session) exit(0);
		if (string(client.user.data["un_hash"]) != Utils::Hashes::GetUnHash()) ExitProcess(0);
		if (string(client.user.data["re_hash"]) != Utils::Hashes::GetReHash()) exit(0);

		if (client.user.data["features"].empty()) exit(0);
		json features = json::parse(client.user.data["features"].dump());
		if (!features.contains("unlimitedcps")) ExitProcess(0);
		if (features["unlimitedcps"].get<int>() <= 0) exit(0);

		Sleep(30 * 1000);
	}
}

void init() {
	JNIHandler::setVM();
	JNIHandler::setEnv();
	JNIHandler::setClassLoader();

	ConfigManager::ConfigManager();
	ConfigManager::Parse();

	HANDLE hCheckLicense = CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(checkLicense), nullptr, NULL, nullptr);
	if (!hCheckLicense) exit(0);

	jclass WalkingBoolean = JNIHandler::FindLoadedClass("net/xtrafrancyz/covered/ObfValue$WalkingBoolean");
	if (!WalkingBoolean) Utils::ErrorHandler::send(CLASS_NOT_FOUND);

	Method* set = GetMethod(WalkingBoolean, "set", "(Z)V");
	if (!set) Utils::ErrorHandler::send(METHOD_NOT_FOUND);

	if (set) {
		RI::JMethodInterceptor* interceptor = new RI::JMethodInterceptor(set, changeValue);
		thread interception(&RI::JMethodInterceptor::intercept, interceptor);
		interception.detach();
	}
}

void initStaticFields() {
	JNIHandler::initStaticFields();
}

BOOL APIENTRY DllMain(HINSTANCE handle, DWORD reason, LPVOID reserved) {
	switch (reason) {
	case DLL_VIMEWORLD_ATTACH:
		setlocale(LC_ALL, "ru");
		initStaticFields();

		client.host = "https://destructiqn.com:9990";
		client.user.name = ConfigManager::ParseUsername();
		client.user.session = reinterpret_cast<const char*>(reserved);

		client.getkey(client.user.name, Utils::Hashes::GetReHash());
		if (!client.user.data["features"].empty()) {
			json features = json::parse(client.user.data["features"].dump());
			if (features.contains("unlimitedcps")) {
				if (features["unlimitedcps"].get<int>() > 0) {
					client.foo(client.user.name, ConfigManager::ParseUsername(true), "UnlimitedCPS");
					CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(init), nullptr, NULL, nullptr);
				}
			}
		}

		break;
	}

	return TRUE;
}