#pragma once
#include <Windows.h>

namespace AssemblerAPI {
	class Utils {
	public:
		static byte GetAddressInBytes(PVOID address, BYTE* buffer) {
			// »щем размер адреса в байтах
			byte subtracted = 0;
			for (; *reinterpret_cast<byte*>((reinterpret_cast<uintptr_t>(&address) + 7) - subtracted) == 0x00; subtracted++) {}
			byte addressSize = 8 - subtracted;

			ReadProcessMemory(GetCurrentProcess(), &address, buffer, addressSize, nullptr);
			return addressSize;
		}
	};

	class Assembler {
	public:
		static void jmp(PVOID from, PVOID to) {
			PVOID offset = reinterpret_cast<PVOID>(reinterpret_cast<uintptr_t>(to) - reinterpret_cast<uintptr_t>(from) - 5);
			byte jmp[5] = { 0xE9 };
			ReadProcessMemory(GetCurrentProcess(), &offset, reinterpret_cast<LPVOID>(reinterpret_cast<uintptr_t>(jmp) + 1), 4, nullptr);
			WriteProcessMemory(GetCurrentProcess(), from, jmp, 5, nullptr);
		}

		static void jmpRCX(PVOID from, PVOID to) {
			*reinterpret_cast<byte*>(from) = 0x51; // PUSH RCX дл€ сохранени€ регистра

			byte address[8];
			byte addressSize = Utils::GetAddressInBytes(to, address);

			byte movRcx[10] = { 0x48, 0xB9 };
			for (byte i = 2; i < (2 + addressSize); i++) movRcx[i] = address[i - 2];

			byte jmpRcx[2]{ 0xFF, 0xE1 };

			WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(reinterpret_cast<uintptr_t>(from) + 1), movRcx, 10, nullptr);
			WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(reinterpret_cast<uintptr_t>(from) + 11), jmpRcx, 2, nullptr);
		}
	};
}