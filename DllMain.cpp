#define _CRT_SECURE_NO_WARNINGS

#include "DllMain.hpp"

#pragma warning(disable:6031)
#define DEBUG(s, ...) printf((string("[DEBUG] ") + s + '\n').c_str(), __VA_ARGS__)

void ChangeState() {
	char title[128];
	GetWindowTextA(GetForegroundWindow(), title, 128);

	if (strcmp(title, "VimeWorld") == 0) {
		ConfigManager::isEnabled = !ConfigManager::isEnabled;
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
			case WM_MBUTTONDOWN: {
				if (ConfigManager::keybind == "MBUTTON")
					ChangeState();

				break;
			}
			case WM_XBUTTONDOWN: {
				PMSLLHOOKSTRUCT p = reinterpret_cast<PMSLLHOOKSTRUCT>(lParam);
				if ((p->mouseData >> 16) == 1 && ConfigManager::keybind == "XBUTTON1")
					ChangeState();
				else if ((p->mouseData >> 16) == 2 && ConfigManager::keybind == "XBUTTON2")
					ChangeState();

				break;
			}
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

		if (lastIsEnabled != ConfigManager::isEnabled)
			ConfigManager::ChangeState(ConfigManager::keybind, ConfigManager::isEnabled);

		Sleep(333);
	}
}

Method* GetMethod(jclass clazz, const char* name, const char* sig) {
	InstanceKlass* klass = *(InstanceKlass**)(*(DWORD*)clazz + 0x48);
	Array<Method*>* _methods = nullptr;

	for (uintptr_t _init_thread_p = (uintptr_t)klass + FIELD_OFFSET(InstanceKlass, _init_thread); !_methods; _init_thread_p += 8) {
		LPVOID _init_thread = *(LPVOID*)_init_thread_p;
		BYTE aob_sign[8];

		if (_init_thread != nullptr && ReadProcessMemory(GetCurrentProcess(), _init_thread, aob_sign, 8, nullptr))
			if (Utils::PatternScanner::compareBytes(aob_sign, (PBYTE)"\x03\x00\x00\x00\x00\x00\x00\x00", (PCHAR)"xxxxxxxx"))
				_methods = (Array<Method*>*)_init_thread;
	}

	for (uint16_t i = 0; i < _methods->_length; i++) {
		Method* method = _methods->at(i);
		if (strcmp(method->_constMethod->_constants->symbol_at(method->_constMethod->_name_index)->as_string().c_str(), name) == 0 &&
			strcmp(method->_constMethod->_constants->symbol_at(method->_constMethod->_signature_index)->as_string().c_str(), sig) == 0) {
			return method;
		}
	}

	return nullptr;
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

void __declspec(naked) __cdecl SetWalkingBooleanInterceptor() {
	if (ConfigManager::isEnabled) {
		JNIEnv* env = nullptr;
		JNIHandler::vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_8);

		jstring class_name = env->NewStringUTF("net/xtrafrancyz/covered/TexteriaOptions");
		jclass TexteriaOptions = JNIHandler::JVM_FindLoadedClass(env, JNIHandler::ClassLoader, class_name);
		if (!TexteriaOptions) Utils::ErrorHandler::send(CLASS_NOT_FOUND);

		uintptr_t java_mirror = *reinterpret_cast<uintptr_t*>(TexteriaOptions);
		DWORD disable_cps_limit_instance = *reinterpret_cast<DWORD*>(java_mirror + 0x68);

		if (*reinterpret_cast<DWORD*>(hook->registers.R13 + 8) == disable_cps_limit_instance)
			*reinterpret_cast<jboolean*>(hook->registers.R13) = true;

		// Freeing memory
		if (TexteriaOptions) {
			env->DeleteLocalRef(class_name);
			env->DeleteLocalRef(TexteriaOptions);
		}
	}

	hook->original();
}

void init() {
	JNIHandler::setVM();
	JNIHandler::setEnv();
	JNIHandler::setClassLoader();

	ConfigManager::ConfigManager();
	ConfigManager::Parse();

	HANDLE hCheckLicense = CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(CheckLicense), nullptr, NULL, nullptr);
	if (!hCheckLicense) exit(0);

	jclass WalkingBoolean = JNIHandler::FindClass("net.xtrafrancyz.covered.ObfValue$WalkingBoolean");
	if (!WalkingBoolean) Utils::ErrorHandler::send(CLASS_NOT_FOUND);

	Method* set = GetMethod(WalkingBoolean, "set", "(Z)V");
	if (!set) Utils::ErrorHandler::send(METHOD_NOT_FOUND);

	if (set) {
		LPVOID _from_interpreted_entry_p = nullptr;
		for (uintptr_t _constMethod_p = (uintptr_t)set + FIELD_OFFSET(Method, _constMethod); !_from_interpreted_entry_p; _constMethod_p += 8) {
			LPVOID _constMethod = *(LPVOID*)_constMethod_p;
			BYTE aob_sign[3];

			if (_constMethod != nullptr && ReadProcessMemory(GetCurrentProcess(), _constMethod, aob_sign, 3, nullptr))
				if (Utils::PatternScanner::compareBytes(aob_sign, (PBYTE)"\x48\x8B\x04", (PCHAR)"xxx"))
					_from_interpreted_entry_p = (LPVOID)_constMethod_p;
		}
		
		hook = new JavaHook(reinterpret_cast<PVOID*>(_from_interpreted_entry_p), SetWalkingBooleanInterceptor, 2);
		CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(CheckState), nullptr, NULL, nullptr);
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
	}

	return TRUE;
}