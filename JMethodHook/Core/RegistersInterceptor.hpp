#pragma once
#include "ASM/AssemblerAPI.hpp"
#pragma warning(disable:6011)

namespace RegistersInterceptor {
	class Interceptor {
	protected:
		void WriteRegistersInStruct(PVOID address) {
			byte structAddress[8];
			byte addressSize = AssemblerAPI::Utils::GetAddressInBytes(&this->registers, structAddress);

			byte movStruct[10] = { 0x48, 0xBA };
			for (byte i = 2; i < 10; i++) movStruct[i] = structAddress[i - 2];

			WriteProcessMemory(GetCurrentProcess(), address, movStruct, 10, nullptr);

			byte movRegisters[109] = {
				0x48, 0x89, 0x42, 0x78, 0x48, 0x89, 0x8A, 0x80, 0x00, 0x00, 0x00, 0x48, 0x89, 0x92, 0x88, 0x00, 0x00, 0x00, 0x48, 0x89, 0x9A,
				0x90, 0x00, 0x00, 0x00, 0x48, 0x89, 0xA2, 0x98, 0x00, 0x00, 0x00, 0x48, 0x89, 0xAA, 0xA0, 0x00, 0x00, 0x00, 0x48, 0x89, 0xB2,
				0xA8, 0x00, 0x00, 0x00, 0x48, 0x89, 0xBA, 0xB0, 0x00, 0x00, 0x00, 0x4C, 0x89, 0x82, 0xB8, 0x00, 0x00, 0x00, 0x4C, 0x89, 0x8A,
				0xC0, 0x00, 0x00, 0x00, 0x4C, 0x89, 0x92, 0xC8, 0x00, 0x00, 0x00, 0x4C, 0x89, 0x9A, 0xD0, 0x00, 0x00, 0x00, 0x4C, 0x89, 0xA2,
				0xD8, 0x00, 0x00, 0x00, 0x4C, 0x89, 0xAA, 0xE0, 0x00, 0x00, 0x00, 0x4C, 0x89, 0xB2, 0xE8, 0x00, 0x00, 0x00, 0x4C, 0x89, 0xBA,
				0xF0, 0x00, 0x00, 0x00
			};

			WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(reinterpret_cast<uintptr_t>(address) + 10), movRegisters, 109, nullptr);
		}
	public:
		CONTEXT registers = {NULL};

		virtual void intercept(PVOID address, byte opcodesSize) {
			// Сохраняем инструкции которые стоят до создания прыжка, чтобы не поломать оригинальный код
			byte* opcodes = new byte[opcodesSize];
			ReadProcessMemory(GetCurrentProcess(), address, opcodes, opcodesSize, nullptr);

			// Код до прыжка
			PVOID alloc = VirtualAlloc(NULL, 4096, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
			AssemblerAPI::Assembler::jmpRCX(address, alloc); // Прыжок вперёд, в пустую память
			*reinterpret_cast<byte*>(reinterpret_cast<uintptr_t>(address) + 13) = 0x59; // POP RCX после инструкции jmp для сохранения регистра

			// Код внутри прыжка
			*reinterpret_cast<byte*>(alloc) = 0x59; // POP RCX внутри прыжка для сохранения регистра
			this->WriteRegistersInStruct(reinterpret_cast<PVOID>(reinterpret_cast<uintptr_t>(alloc) + 1));

			// Записываем оригинальные инструкции чтобы не поломать оригинальный код
			WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<PVOID>(reinterpret_cast<uintptr_t>(alloc) + 120),
				opcodes, opcodesSize, nullptr);

			// Прыжок назад
			AssemblerAPI::Assembler::jmpRCX(reinterpret_cast<PVOID>(reinterpret_cast<uintptr_t>(alloc) + 120 + opcodesSize),
				reinterpret_cast<PVOID>(reinterpret_cast<uintptr_t>(address) + 13));

			// Удаляем динамический массив с байтами опкодов
			delete[] opcodes;
		}
	};
}