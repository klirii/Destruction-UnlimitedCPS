#include "DllMain.hpp"
using namespace std;

Method* FindMethodByBytecode(PBYTE bytes, USHORT len) {
	// Создаём маску для паттерна
	string mask;
	for (USHORT i = 0; i < len; i++) mask += "x";

	// Ищем байткод в памяти
	PBYTE bytecode = Utils::PatternScanner::scan(bytes, const_cast<char*>(mask.c_str()));
	if (!bytecode) Utils::ErrorHandler::send(METHOD_NOT_FOUND);

	// Получаем адрес Method vTable`а
	PVOID vTable = reinterpret_cast<PVOID>(reinterpret_cast<uintptr_t>(JNIHandler::jvm) + 0x56E6B8);
	byte vTableAddress[8];
	ReadProcessMemory(GetCurrentProcess(), &vTable, vTableAddress, 8, nullptr);

	// Инкрементируем адрес байткода, пока не достигнем адрес Method vTable`а
	while (!Utils::PatternScanner::compareBytes(bytecode, vTableAddress, const_cast<char*>("xxxxxxxx"))) bytecode++;
	return reinterpret_cast<Method*>(bytecode);
}

USHORT changeValue(uintptr_t insertionAddress) {
	jclass TexteriaOptions = JNIHandler::FindAnyClass("net/xtrafrancyz/mods/texteria/TexteriaOptions");
	if (!TexteriaOptions) Utils::ErrorHandler::send(CLASS_NOT_FOUND);

	byte movRdxJClass[10] = { 0x48, 0xBA };
	WriteProcessMemory(GetCurrentProcess(), movRdxJClass + 0x02, &TexteriaOptions, 8, nullptr);

	byte movRdxDereference[3] = { 0x48, 0x8B, 0x12 };
	byte movRdxDisableCpsLimit[4] = { 0x48, 0x8B, 0x52, 0x68 };

	byte cmpInstance[4] = { 0x39, 0x54, 0x24, 0x18 };
	byte jne[2] = { 0x75, 0x08 };
	byte movValue[8] = { 0xC7, 0x44, 0x24, 0x10, 0x01, 0x00, 0x00, 0x00 };

	WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(insertionAddress), movRdxJClass, 10, nullptr);
	WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(insertionAddress + 10), movRdxDereference, 3, nullptr);
	WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(insertionAddress + 13), movRdxDisableCpsLimit, 4, nullptr);
	WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(insertionAddress + 17), cmpInstance, 4, nullptr);
	WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(insertionAddress + 21), jne, 2, nullptr);
	WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(insertionAddress + 23), movValue, 8, nullptr);
	return 31;
}

void init() {
	JNIHandler::setVM();
	JNIHandler::setEnv();
	JNIHandler::setClassLoader();

	Method* method = FindMethodByBytecode(reinterpret_cast<PBYTE>(const_cast<char*>(
		"\xDC\x1B\x99\x00\x09\xB2\x00\x00\xA7\x00\x06\xB2\x01\x00\xB6\x03\x00\xB1")),
		18);

	if (method) {
		static RegistersInterceptor::JMethodInterceptor interceptor;
		interceptor.intercept(method, changeValue);
		MessageBoxA(Utils::ErrorHandler::window, "UnlimitedCPS успешно внедрён!\nПриятной игры!", "Destruction", MB_OK);
	}
}

BOOL APIENTRY DllMain(HINSTANCE handle, DWORD reason, LPVOID reserved) {
	switch (reason) {
	case DLL_PROCESS_ATTACH:
		CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(init), nullptr, NULL, nullptr);
		break;
	}

	return TRUE;
}