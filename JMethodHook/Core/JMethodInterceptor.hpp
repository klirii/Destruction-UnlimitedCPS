#pragma once
#include "RegistersInterceptor.hpp"
#include "../JVM/hotspot/src/share/vm/oops/method.hpp"

#include <functional>
#include <jni.h>

namespace RegistersInterceptor {
	class JMethodInterceptor : public Interceptor {
	public:
		uint64_t callCounter = 0;

		void intercept(PVOID address, std::function<USHORT(uintptr_t)> asmInsert) {
			Method* method = reinterpret_cast<Method*>(address);
			PVOID alloc = VirtualAlloc(NULL, 4096, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

			// «аписываем регистры в this->registers
			*reinterpret_cast<byte*>(alloc) = 0x52; // push rdx дл€ сохранени€ значени€ регистра
			this->WriteRegistersInStruct(reinterpret_cast<PVOID>(reinterpret_cast<uintptr_t>(alloc) + 1));

			// ”величиваем this->callCounter на 1
			byte inc[6] = { 0xFF, 0x82, 0xD0, 0x04, 0x00, 0x00 };
			WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(reinterpret_cast<uintptr_t>(alloc) + 120), inc, 6, nullptr);

			USHORT offset2jmp = 126;
			if (asmInsert) offset2jmp += asmInsert(reinterpret_cast<uintptr_t>(alloc) + offset2jmp);

			*reinterpret_cast<byte*>(reinterpret_cast<uintptr_t>(alloc) + offset2jmp) = 0x5A; // pop rdx дл€ сохранени€ значени€ регистра

			// ƒелаем jmp на _i2i_entry
			AssemblerAPI::Assembler::jmp(reinterpret_cast<PVOID>(reinterpret_cast<uintptr_t>(alloc) + (offset2jmp + 1)),
				method->_code ? method->_from_interpreted_entry : method->_i2i_entry);

			// «аписываем в структуру метода новый _from_interpreted_entry, указывающий на наш alloc
			byte newEntry[8];
			ReadProcessMemory(GetCurrentProcess(), &alloc, newEntry, 8, nullptr);
			WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(reinterpret_cast<uintptr_t>(method) + 0x50), newEntry, 8, nullptr);
		}

		void intercept(jmethodID mid, std::function<USHORT(uintptr_t)> asmInsert) {
			PVOID address = reinterpret_cast<PVOID>(*(reinterpret_cast<uintptr_t*>(mid)));
			this->intercept(address, asmInsert);
		}

		void intercept(jmethodID mid) {
			PVOID address = reinterpret_cast<PVOID>(*(reinterpret_cast<uintptr_t*>(mid)));
			this->intercept(address, nullptr);
		}

		void intercept(PVOID address) {
			this->intercept(address, nullptr);
		}
	};
}