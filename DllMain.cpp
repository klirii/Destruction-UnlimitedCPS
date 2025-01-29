#define _CRT_SECURE_NO_WARNINGS

#include "DllMain.hpp"

#pragma warning(disable:6031)
#pragma warning(disable:6387)

HANDLE current_process = nullptr;

HMODULE    jvm      = nullptr;
MODULEINFO jvm_info = { NULL };

ClassLoaderData* class_loader = nullptr;

// Klasses
InstanceKlass* TexteriaOptions         = nullptr;
InstanceKlass* WalkingBoolean          = nullptr;
InstanceKlass* WalkingIntegerContainer = nullptr;
InstanceKlass* ContainerInt            = nullptr;

// Fields
FieldInfo* disable_cps_limit_f = nullptr;

// WalkingBoolean
FieldInfo* value_true_f  = nullptr;
FieldInfo* value_false_f = nullptr;

// WalkingIntegerContainer
FieldInfo* steps_f      = nullptr;
FieldInfo* obfuscated_f = nullptr;
FieldInfo* walk_f       = nullptr;
FieldInfo* salt_f       = nullptr;

// ContainerInt
FieldInfo* value_f = nullptr;

void SetKlasses() {
	TexteriaOptions         = FindClass("net/xtrafrancyz/covered/TexteriaOptions");
	WalkingBoolean          = FindClass("net/xtrafrancyz/covered/ObfValue$WalkingBoolean");
	WalkingIntegerContainer = FindClass("net/xtrafrancyz/covered/ObfValue$WalkingIntegerContainer");
	ContainerInt            = FindClass("net/xtrafrancyz/covered/ObfValue$ContainerInt");
}

void SetFields() {
	disable_cps_limit_f = FindField(TexteriaOptions, "disableCpsLimit", "Lnet/xtrafrancyz/covered/ObfValue$WalkingBoolean;");

	value_true_f  = FindField(WalkingBoolean, "vTrue", "I");
	value_false_f = FindField(WalkingBoolean, "vFalse", "I");

	steps_f      = FindField(WalkingIntegerContainer, "steps", "I");
	obfuscated_f = FindField(WalkingIntegerContainer, "obfuscated", "Lnet/xtrafrancyz/covered/ObfValue$ContainerInt;");
	walk_f       = FindField(WalkingIntegerContainer, "walk", "I");
	salt_f       = FindField(WalkingIntegerContainer, "salt", "Lnet/xtrafrancyz/covered/ObfValue$ContainerInt;");

	value_f = FindField(ContainerInt, "value", "I");
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

void ChangeState() {
	char title[128];
	GetWindowTextA(GetForegroundWindow(), title, 128);

	if (strcmp(title, "VimeWorld") == 0) {
		ConfigManager::isEnabled = !ConfigManager::isEnabled;
		ConfigManager::ChangeState(ConfigManager::keybind, ConfigManager::isEnabled);
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

void SetKeyboardHook() {
	HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, nullptr, NULL);
	if (!hook) Utils::ErrorHandler::send(KEYBOARD_HOOK_ERROR);

	MSG msg;
	while (!GetMessage(&msg, nullptr, NULL, NULL)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

//void HideCPS() {
//	if (client.user.name == "qzzzzz") {
//		JNIEnv* local_env = nullptr;
//		vm->AttachCurrentThread(reinterpret_cast<void**>(&local_env), nullptr);
//
//		jclass PvPMod = local_env->FindClass("net/xtrafrancyz/mods/pvp/PvPMod");
//		if (!PvPMod) Utils::ErrorHandler::send(CLASS_NOT_FOUND);
//
//		jfieldID left_mouse_counter_fid = local_env->GetStaticFieldID(PvPMod, "leftMouseCounter", "Lnet/xtrafrancyz/mods/pvp/PvPMod$ClickCounter;");
//		jobject left_mouse_counter = local_env->GetStaticObjectField(PvPMod, left_mouse_counter_fid);
//
//		jclass ClickCounter = local_env->GetObjectClass(left_mouse_counter);
//		if (!ClickCounter) Utils::ErrorHandler::send(CLASS_NOT_FOUND);
//
//		jfieldID clicks_fid = local_env->GetFieldID(ClickCounter, "clicks", "Ljava/util/Queue;");
//		jobject clicks = local_env->GetObjectField(left_mouse_counter, clicks_fid);
//
//		jclass ArrayDeque = local_env->FindClass("java/util/ArrayDeque");
//		if (!ArrayDeque) Utils::ErrorHandler::send(CLASS_NOT_FOUND);
//
//		jmethodID size_mid = local_env->GetMethodID(ArrayDeque, "size", "()I");
//		jmethodID poll_last_mid = local_env->GetMethodID(ArrayDeque, "pollLast", "()Ljava/lang/Object;");
//		jmethodID peek_mid = local_env->GetMethodID(ArrayDeque, "peek", "()Ljava/lang/Object;");
//
//		jclass Long = local_env->FindClass("java/lang/Long");
//		if (!Long) Utils::ErrorHandler::send(CLASS_NOT_FOUND);
//
//		jmethodID long_value_mid = local_env->GetMethodID(Long, "longValue", "()J");
//
//		local_env->DeleteLocalRef(ClickCounter);
//		local_env->DeleteLocalRef(left_mouse_counter);
//		local_env->DeleteLocalRef(PvPMod);
//		local_env->DeleteLocalRef(Long);
//		local_env->DeleteLocalRef(ArrayDeque);
//
//		while (true) {
//			jobject click = local_env->CallObjectMethod(clicks, peek_mid);
//
//			if (local_env->CallLongMethod(click, long_value_mid) < 1) {
//				local_env->DeleteLocalRef(click);
//				continue;
//			}
//
//			for (int i = 0; i < 2; i++) {
//				if (local_env->CallIntMethod(clicks, size_mid) > 13) {
//					jobject last = local_env->CallObjectMethod(clicks, poll_last_mid);
//					local_env->DeleteLocalRef(last);
//				}
//			}
//
//			local_env->DeleteLocalRef(click);
//			Sleep(1);
//		}
//	}
//}

inline jint GetContainerInt(oop instance) {
	return GetField<jint>(instance, value_f) ^ 0xDF099FDE;
}

inline void SetContainerInt(oop instance, jint value) {
	SetField<jint>(instance, value_f, value ^ 0xDF099FDE);
}

void SetWalkingIntegerContainer(oop instance, jint value) {
	bool is_set = false;

	while (!is_set) {
		jint steps = GetField<jint>(instance, steps_f);
		jint walk = GetField<jint>(instance, walk_f);
		if (steps >= walk) continue;

		oop obfuscated = GetObjectField(instance, obfuscated_f);
		oop salt = GetObjectField(instance, salt_f);

		SetContainerInt(obfuscated, value ^ GetContainerInt(salt));
		is_set = true;

		Sleep(10);
	}
}

void SetWalkingBoolean(oop instance, bool value) {
	jint value_true = GetField<jint>(instance, value_true_f);
	jint value_false = GetField<jint>(instance, value_false_f);

	SetWalkingIntegerContainer(instance, value ? value_true : value_false);
}

void InitializeGlobals() {
	current_process = GetCurrentProcess();

	jvm = GetModuleHandleA("jvm.dll");
	GetModuleInformation(current_process, jvm, &jvm_info, sizeof(MODULEINFO));

	FindAllOffsets();
	class_loader = FindClassLoader();

	Utils::ErrorHandler::window = FindWindowA(nullptr, "VimeWorld");
	SetKlasses();
	SetFields();
}

void Main() {
	InitializeGlobals();

	ConfigManager::ConfigManager();
	ConfigManager::Parse();

	//HANDLE check_license_handle = CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)CheckLicense, nullptr, NULL, nullptr);
	//if (!check_license_handle) exit(0);
	
	CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(CheckState), nullptr, NULL, nullptr);
	CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(SetKeyboardHook), nullptr, NULL, nullptr);

	while (true) {
		oop disable_cps_limit = GetObjectField(TexteriaOptions->java_mirror(), disable_cps_limit_f);
		if (!disable_cps_limit) continue;

		SetWalkingBoolean(disable_cps_limit, true);
		Sleep(100);
	}
}

BOOL APIENTRY DllMain(HINSTANCE handle, DWORD reason, LPVOID reserved) {
	switch (reason) {
	case DLL_PROCESS_ATTACH: // TODO DLL_VIMEWORLD_ATTACH
		setlocale(LC_ALL, "ru");

		//client.host = "http://api.destructiqn.com:2086";
		//client.user.name = ConfigManager::ParseUsername();
		//client.user.password = ConfigManager::ParsePassword();
		//client.user.session = reinterpret_cast<const char*>(reserved);

		//client.getdocument(client.user.name, client.user.password, client.user.session, Utils::Hashes::GetReHash());
		//if (!client.user.data["features"].empty()) {
		//	json features = json::parse(client.user.data["features"].dump());
		//	if (features.contains("unlimitedcps")) {
		//		if (features["unlimitedcps"].get<int>() > 0) {
		//			client.foobar(client.user.name, ConfigManager::ParseUsername(true), "UnlimitedCPS", RestAPI::Utils::get_ip());
		//			CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(Main), nullptr, NULL, nullptr);
		//		}
		//	}
		//}

		CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(Main), nullptr, NULL, nullptr);
	}

	return TRUE;
}