#include "DllMain.hpp"

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

	jclass TexteriaOptions = JNIHandler::FindLoadedClass("net/xtrafrancyz/mods/texteria/TexteriaOptions");
	if (!TexteriaOptions) Utils::ErrorHandler::send(CLASS_NOT_FOUND);

	// ≈сли инстанс от которого вызываетс€ метод совпадает с инстансом disableCpsLimit - мен€ем value на true
	byte movRdxJClass[10] = { 0x48, 0xBA };
	WriteProcessMemory(hProc, movRdxJClass + 0x02, &TexteriaOptions, 8, nullptr);

	byte movRdxDereference[3] = { 0x48, 0x8B, 0x12 };
	byte movRdxDisableCpsLimit[4] = { 0x48, 0x8B, 0x52, 0x68 };

	byte cmpInstance[4] = { 0x39, 0x54, 0x24, 0x18 };
	byte jne[2] = { 0x75, 0x08 };
	byte movValue[8] = { 0xC7, 0x44, 0x24, 0x10, 0x01, 0x00, 0x00, 0x00 };

	WriteProcessMemory(hProc, reinterpret_cast<LPVOID>(insertionAddress), movRdxJClass, 10, nullptr);
	WriteProcessMemory(hProc, reinterpret_cast<LPVOID>(insertionAddress + 10), movRdxDereference, 3, nullptr);
	WriteProcessMemory(hProc, reinterpret_cast<LPVOID>(insertionAddress + 13), movRdxDisableCpsLimit, 4, nullptr);
	WriteProcessMemory(hProc, reinterpret_cast<LPVOID>(insertionAddress + 17), cmpInstance, 4, nullptr);
	WriteProcessMemory(hProc, reinterpret_cast<LPVOID>(insertionAddress + 21), jne, 2, nullptr);
	WriteProcessMemory(hProc, reinterpret_cast<LPVOID>(insertionAddress + 23), movValue, 8, nullptr);
	return 31;
}

void licenseCheck() {
	while (true) {
		client.getkey(client.user.name, "CAFEBABE");
		
		if (std::string(client.user.data["session"]) != client.user.session) exit(0);
		if (std::string(client.user.data["un_hash"]) != Utils::Hashes::GetUnHash()) ExitProcess(0);
		if (std::string(client.user.data["re_hash"]) != Utils::Hashes::GetReHash()) exit(0);

		if (client.user.data["features"].empty()) exit(0);
		nlohmann::json features = nlohmann::json::parse(client.user.data["features"].dump());
		if (!features.contains("unlimitedcps")) ExitProcess(0);
		if (features["unlimitedcps"].get<int>() <= 0) exit(0);

		Sleep(30 * 1000);
	}
}

void init() {
	JNIHandler::setVM();
	JNIHandler::setEnv();
	JNIHandler::setClassLoader();
	CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(licenseCheck), nullptr, NULL, nullptr);

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
		initStaticFields();

		client.host = "https://destructiqn.com:9990";
		client.user.name = ConfigManager::ParseUsername();
		client.user.session = reinterpret_cast<const char*>(reserved);

		client.getkey(client.user.name, Utils::Hashes::GetReHash());
		if (!client.user.data["features"].empty()) {
			nlohmann::json features = nlohmann::json::parse(client.user.data["features"].dump());
			if (features.contains("unlimitedcps")) {
				if (features["unlimitedcps"].get<int>() > 0) {
					CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(init), nullptr, NULL, nullptr);
				}
			}
		}

		break;
	}

	return TRUE;
}