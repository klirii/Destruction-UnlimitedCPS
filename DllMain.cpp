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

void SetKeyboardHook() {
	HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, nullptr, NULL);
	if (!hook) Utils::ErrorHandler::send(KEYBOARD_HOOK_ERROR);

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

void SetWalkingBoolean(oop instance, jboolean value) {
	bool setted = false;

	// WalkingIntegerContainer

	jclass WalkingIntegerContainer = FindClass("net.xtrafrancyz.covered.ObfValue$WalkingIntegerContainer");
	if (!WalkingIntegerContainer) Utils::ErrorHandler::send(CLASS_NOT_FOUND);

	FieldInfo* obfuscated_f = FindField(WalkingIntegerContainer, "obfuscated", "Lnet/xtrafrancyz/covered/ObfValue$ContainerInt;");
	if (!obfuscated_f) Utils::ErrorHandler::send(FIELD_NOT_FOUND);

	FieldInfo* walk_f = FindField(WalkingIntegerContainer, "walk", "I");
	if (!walk_f) Utils::ErrorHandler::send(FIELD_NOT_FOUND);

	FieldInfo* steps_f = FindField(WalkingIntegerContainer, "steps", "I");
	if (!steps_f) Utils::ErrorHandler::send(FIELD_NOT_FOUND);

	FieldInfo* salt_f = FindField(WalkingIntegerContainer, "salt", "Lnet/xtrafrancyz/covered/ObfValue$ContainerInt;");
	if (!salt_f) Utils::ErrorHandler::send(FIELD_NOT_FOUND);

	// ContainerInt
	
	jclass ContainerInt = FindClass("net.xtrafrancyz.covered.ObfValue$ContainerInt");
	if (!ContainerInt) Utils::ErrorHandler::send(CLASS_NOT_FOUND);

	FieldInfo* value_f = FindField(ContainerInt, "value", "I");
	if (!value_f) Utils::ErrorHandler::send(FIELD_NOT_FOUND);

	// WalkingBoolean

	jclass WalkingBoolean = FindClass("net.xtrafrancyz.covered.ObfValue$WalkingBoolean");
	if (!WalkingBoolean) Utils::ErrorHandler::send(CLASS_NOT_FOUND);

	FieldInfo* value_true_f = FindField(WalkingBoolean, "vTrue", "I");
	if (!value_true_f) Utils::ErrorHandler::send(FIELD_NOT_FOUND);

	FieldInfo* value_false_f = FindField(WalkingBoolean, "vFalse", "I");
	if (!value_false_f) Utils::ErrorHandler::send(FIELD_NOT_FOUND);

	while (!setted) {
		jint steps = GetField<jint>(instance, steps_f);
		jint walk = GetField<jint>(instance, walk_f);

		if (steps >= walk) {
			SetField<jint>(instance, steps_f, 0);
			continue;
		}

		oop obfuscated = GetObjectField(instance, obfuscated_f);
		oop salt = GetObjectField(instance, salt_f);
		jint salt_value = GetField<jint>(salt, value_f) ^ 0xDF099FDE;

		jint value_true = GetField<jint>(instance, value_true_f);
		jint value_false = GetField<jint>(instance, value_false_f);
		
		value_true = (value_true ^ salt_value) ^ 0xDF099FDE;
		value_false = (value_false ^ salt_value) ^ 0xDF099FDE;

		SetField<jint>(obfuscated, value_f, value ? value_true : value_false);
		setted = true;

		Sleep(10);
	}

	env->DeleteLocalRef(WalkingBoolean);
	env->DeleteLocalRef(ContainerInt);
	env->DeleteLocalRef(WalkingIntegerContainer);
}

void Main() {
	vm->AttachCurrentThread(reinterpret_cast<void**>(&env), nullptr);
	SetClassLoader();

	ConfigManager::ConfigManager();
	ConfigManager::Parse();

	HANDLE hCheckLicense = CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(CheckLicense), nullptr, NULL, nullptr);
	if (!hCheckLicense) exit(0);

	jclass TexteriaOptions = FindClass("net.xtrafrancyz.covered.TexteriaOptions");
	if (!TexteriaOptions) Utils::ErrorHandler::send(CLASS_NOT_FOUND);

	FieldInfo* disable_cps_limit_f = FindField(TexteriaOptions, "disableCpsLimit", "Lnet/xtrafrancyz/covered/ObfValue$WalkingBoolean;");
	if (!disable_cps_limit_f) Utils::ErrorHandler::send(FIELD_NOT_FOUND);
	
	CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(CheckState), nullptr, NULL, nullptr);
	CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(SetKeyboardHook), nullptr, NULL, nullptr);

	while (true) {
		oop disable_cps_limit = GetObjectField(oopDesc::resolve_jclass(TexteriaOptions), disable_cps_limit_f);
		if (!disable_cps_limit) continue;

		SetWalkingBoolean(disable_cps_limit, ConfigManager::isEnabled);
		Sleep(100);
	}

	//if (client.user.name == "qzzzzz") {
	//	jclass PvPMod = JNIHandler::FindClass("net.xtrafrancyz.mods.pvp.PvPMod");
	//	if (!PvPMod) Utils::ErrorHandler::send(CLASS_NOT_FOUND);

	//	jfieldID left_mouse_counter_fid = JNIHandler::env->GetStaticFieldID(PvPMod, "leftMouseCounter", "Lnet/xtrafrancyz/mods/pvp/PvPMod$ClickCounter;");
	//	jobject left_mouse_counter = JNIHandler::env->GetStaticObjectField(PvPMod, left_mouse_counter_fid);

	//	jclass ClickCounter = JNIHandler::env->GetObjectClass(left_mouse_counter);
	//	if (!ClickCounter) Utils::ErrorHandler::send(CLASS_NOT_FOUND);

	//	jfieldID clicks_fid = JNIHandler::env->GetFieldID(ClickCounter, "clicks", "Ljava/util/Queue;");
	//	jobject clicks = JNIHandler::env->GetObjectField(left_mouse_counter, clicks_fid);

	//	jclass ArrayDeque = JNIHandler::env->FindClass("java/util/ArrayDeque");
	//	if (!ArrayDeque) Utils::ErrorHandler::send(CLASS_NOT_FOUND);

	//	jmethodID size_mid = JNIHandler::env->GetMethodID(ArrayDeque, "size", "()I");
	//	jmethodID poll_last_mid = JNIHandler::env->GetMethodID(ArrayDeque, "pollLast", "()Ljava/lang/Object;");
	//	jmethodID peek_mid = JNIHandler::env->GetMethodID(ArrayDeque, "peek", "()Ljava/lang/Object;");

	//	jclass Long = JNIHandler::env->FindClass("java/lang/Long");
	//	if (!Long) Utils::ErrorHandler::send(CLASS_NOT_FOUND);

	//	jmethodID long_value_mid = JNIHandler::env->GetMethodID(Long, "longValue", "()J");

	//	JNIHandler::env->DeleteLocalRef(ClickCounter);
	//	JNIHandler::env->DeleteLocalRef(left_mouse_counter);
	//	JNIHandler::env->DeleteLocalRef(PvPMod);
	//	JNIHandler::env->DeleteLocalRef(Long);
	//	JNIHandler::env->DeleteLocalRef(ArrayDeque);

	//	while (true) {
	//		jobject click = JNIHandler::env->CallObjectMethod(clicks, peek_mid);

	//		if (JNIHandler::env->CallLongMethod(click, long_value_mid) < 1) {
	//			JNIHandler::env->DeleteLocalRef(click);
	//			continue;
	//		}

	//		for (int i = 0; i < 2; i++) {
	//			if (JNIHandler::env->CallIntMethod(clicks, size_mid) > 13) {
	//				jobject last = JNIHandler::env->CallObjectMethod(clicks, poll_last_mid);
	//				JNIHandler::env->DeleteLocalRef(last);
	//			}
	//		}

	//		JNIHandler::env->DeleteLocalRef(click);
	//		Sleep(1);
	//	}

	//	JNIHandler::env->DeleteLocalRef(clicks);
	//}
}

void InitializeGlobals() {
	jvm = GetModuleHandleA("jvm.dll");
	brainstorm = GetModuleHandleA("Brainstorm64.dll");

	if (jvm) JNI_GetCreatedJavaVMs_p = reinterpret_cast<JNI_GetCreatedJavaVMs_t>(GetProcAddress(jvm, "IIIIlllllIIl"));
	JNI_GetCreatedJavaVMs_p(&vm, 1, nullptr);

	Offsets::Initialize();
	JavaHook::active_hooks = std::vector<JavaHook*>();
	JavaHook::DisableIntegrityChecks();

	Utils::ErrorHandler::window = FindWindowA(nullptr, "VimeWorld");
}

BOOL APIENTRY DllMain(HINSTANCE handle, DWORD reason, LPVOID reserved) {
	switch (reason) {
	case DLL_VIMEWORLD_ATTACH:
		//AllocConsole();
		//freopen("CONOUT$", "w", stdout);
		setlocale(LC_ALL, "ru");
		InitializeGlobals();

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
					CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(Main), nullptr, NULL, nullptr);
				}
			}
		}
	}

	return TRUE;
}