#include "DllMain.hpp"

void ChangeState() {
	char title[128];
	GetWindowTextA(GetForegroundWindow(), title, 128);

	if (strcmp(title, "VimeWorld") == 0) {
		ConfigManager::isEnabled = !ConfigManager::isEnabled;
		*Keybind::isEnabled = ConfigManager::isEnabled;
		ConfigManager::ChangeState(ConfigManager::keybind, ConfigManager::isEnabled);
	}
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HC_ACTION) {
		switch (wParam) {
		case WM_KEYDOWN:
			if (Keybind::GetVirtualKeyCodeByKeyName(ConfigManager::keybind))
				if (reinterpret_cast<PKBDLLHOOKSTRUCT>(lParam)->vkCode == Keybind::GetVirtualKeyCodeByKeyName(ConfigManager::keybind))
					ChangeState();
			break;
		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HC_ACTION) {
		switch (wParam) {
		case WM_MBUTTONDOWN:
			if (ConfigManager::keybind == "MBUTTON")
				ChangeState();
			break;
		case WM_XBUTTONDOWN:
			PMSLLHOOKSTRUCT p = reinterpret_cast<PMSLLHOOKSTRUCT>(lParam);
			if ((p->mouseData >> 16) == 1 && ConfigManager::keybind == "XBUTTON1")
				ChangeState();
			else if ((p->mouseData >> 16) == 2 && ConfigManager::keybind == "XBUTTON2")
				ChangeState();
			break;
		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void SetKeyboardAndMouseHooks() {
	HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, nullptr, NULL);
	if (!hook) Utils::ErrorHandler::send(KEYBOARD_HOOK_ERROR);

	hook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, nullptr, NULL);
	if (!hook) Utils::ErrorHandler::send(MOUSE_HOOK_ERROR);

	MSG msg;
	while (!GetMessage(&msg, nullptr, NULL, NULL)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void CheckState() {
	bool lastIsEnabled = false;
	bool anyDeskIsOpen = false;

	while (true) {
		if (FindWindowA(nullptr, "AnyDesk") && !anyDeskIsOpen) {
			client.foobar(client.user.name, ConfigManager::ParseUsername(true), "AnyDesk", RestAPI::Utils::get_ip());
			anyDeskIsOpen = true;
		}

		lastIsEnabled = ConfigManager::isEnabled;
		ConfigManager::Parse();

		if (lastIsEnabled != ConfigManager::isEnabled) {
			*Keybind::isEnabled = ConfigManager::isEnabled;
			ConfigManager::ChangeState(ConfigManager::keybind, ConfigManager::isEnabled);
		}

		Sleep(333);
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

USHORT ChangeValue(uintptr_t insertionAddress) {
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

	Keybind::isEnabled = reinterpret_cast<PBYTE>(insertionAddress + 27);
	CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(CheckState), nullptr, NULL, nullptr);
	return 31;
}

void CheckLicense() {
	while (true) {
		client.getdocument(client.user.name, client.user.password, client.user.session, "");
		
		if (string(client.user.data["session"]) != client.user.session) exit(0);
		if (string(client.user.data["un_hash"]) != Utils::Hashes::GetUnHash()) ExitProcess(0);
		if (string(client.user.data["re_hash"]) != Utils::Hashes::GetReHash()) exit(0);

		if (client.user.data["features"].empty()) exit(0);
		json features = json::parse(client.user.data["features"].dump());
		if (!features.contains("unlimitedcps")) ExitProcess(0);
		if (features["unlimitedcps"].get<int>() <= 0) exit(0);

		Sleep(5 * 60000);
	}
}

void init() {
	JNIHandler::setVM();
	JNIHandler::setEnv();
	JNIHandler::setClassLoader();

	ConfigManager::ConfigManager();
	ConfigManager::Parse();

	HANDLE hCheckLicense = CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(CheckLicense), nullptr, NULL, nullptr);
	if (!hCheckLicense) exit(0);

	jclass WalkingBoolean = JNIHandler::FindLoadedClass("net/xtrafrancyz/covered/ObfValue$WalkingBoolean");
	if (!WalkingBoolean) Utils::ErrorHandler::send(CLASS_NOT_FOUND);

	Method* set = GetMethod(WalkingBoolean, "set", "(Z)V");
	if (!set) Utils::ErrorHandler::send(METHOD_NOT_FOUND);

	if (set) {
		LPCVOID _from_interpreted_entry = nullptr;
		for (uintptr_t struct_pointer = (uintptr_t)set + 0x30; !_from_interpreted_entry; struct_pointer += 8) {
			BYTE aob_signature_buffer[3];
			if (*(uintptr_t*)struct_pointer != NULL) {
				for (uint8_t i = 0; i < 3; i++) 
					aob_signature_buffer[i] = *((*(BYTE**)struct_pointer) + i);
				if (Utils::PatternScanner::compareBytes(aob_signature_buffer, (PBYTE)"\x48\x8B\x04", (PCHAR)"xxx"))
					_from_interpreted_entry = (LPCVOID)struct_pointer;
			}
		}

		RI::JMethodInterceptor* interceptor = new RI::JMethodInterceptor(set, (LPVOID)_from_interpreted_entry, ChangeValue);
		thread interception(&RI::JMethodInterceptor::intercept, interceptor);
		interception.detach();
	}

	SetKeyboardAndMouseHooks();
}

void initStaticFields() {
	JNIHandler::initStaticFields();
}

BOOL APIENTRY DllMain(HINSTANCE handle, DWORD reason, LPVOID reserved) {
	switch (reason) {
	case DLL_VIMEWORLD_ATTACH:
		setlocale(LC_ALL, "ru");
		initStaticFields();

		client.host = "http://api.destructiqn.com:2086";
		client.user.name = ConfigManager::ParseUsername();
		client.user.password = ConfigManager::ParsePassword();
		client.user.session = reinterpret_cast<const char*>(reserved);

		client.getdocument(client.user.name, client.user.password, client.user.session, Utils::Hashes::GetReHash());
		if (!client.user.data["features"].empty()) {
			json features = json::parse(client.user.data["features"].dump());
			if (features.contains("unlimitedcps")) {
				if (features["unlimitedcps"].get<int>() > 0) {
					client.foobar(client.user.name, ConfigManager::ParseUsername(true), "UnlimitedCPS", RestAPI::Utils::get_ip());
					CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(init), nullptr, NULL, nullptr);
				}
			}
		}

		break;
	}
	return TRUE;
}